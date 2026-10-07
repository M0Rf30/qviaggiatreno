/***************************************************************************
 *   Copyright (C) 2010-2012 by fra74                                           *
 *   francesco.b74@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "parser_trenord.h"
#include <QColor>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QRegularExpression>
#include <QStringList>
#include <QTextDocument>

#include <algorithm>
#include <utility>

namespace {

// decodifica le entità HTML più comuni presenti nei valori degli attributi
QString decodificaEntita(QString valore)
{
    if (!valore.contains(QLatin1Char('&')))
        return valore;

    static const QRegularExpression reNumerica(QStringLiteral("&#(?:[xX]([0-9a-fA-F]+)|([0-9]+));"));
    QString risultato;
    qsizetype pos = 0;
    QRegularExpressionMatchIterator it = reNumerica.globalMatch(valore);
    while (it.hasNext())
    {
        const QRegularExpressionMatch m = it.next();
        risultato += valore.mid(pos, m.capturedStart(0) - pos);
        const bool esadecimale = !m.captured(1).isEmpty();
        const uint codice = esadecimale ? m.captured(1).toUInt(nullptr, 16) : m.captured(2).toUInt();
        if (codice > 0 && codice <= 0x10FFFF)
        {
            const char32_t carattere = static_cast<char32_t>(codice);
            risultato += QString::fromUcs4(&carattere, 1);
        }
        pos = m.capturedEnd(0);
    }
    risultato += valore.mid(pos);

    risultato.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    risultato.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    risultato.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    risultato.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
    risultato.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    return risultato;
}


// attributi di un tag: nome (minuscolo) -> valore
using Attributi = QHash<QString, QString>;

// estrae gli attributi dal testo di un tag di apertura (es. "<a href='x' class=y>")
// tollera apici singoli/doppi/assenti, maiuscole e spazi arbitrari
Attributi estraiAttributi(const QString &tag)
{
    static const QRegularExpression reAttr(
        QStringLiteral("([^\\s=/<>\"']+)(?:\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>\"']+)))?"));

    Attributi attributi;
    // salta il nome del tag
    qsizetype inizio = tag.indexOf(QRegularExpression(QStringLiteral("[\\s/]")));
    if (inizio < 0)
        return attributi;

    QRegularExpressionMatchIterator it = reAttr.globalMatch(tag, inizio);
    while (it.hasNext())
    {
        const QRegularExpressionMatch m = it.next();
        const QString nome = m.captured(1).toLower();
        QString valore = m.captured(2);
        if (valore.isEmpty())
            valore = m.captured(3);
        if (valore.isEmpty())
            valore = m.captured(4);
        if (!attributi.contains(nome))
            attributi.insert(nome, decodificaEntita(valore));
    }
    return attributi;
}

bool haClasse(const Attributi &attributi, const QString &classe)
{
    const QStringList classi = attributi.value(QStringLiteral("class"))
                                   .split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    return classi.contains(classe, Qt::CaseSensitive);
}

// le risposte di /rest/render/* sono JSON della forma {"message": "<html>"}: estrae il frammento
// HTML. Se la risposta non è JSON la si considera già HTML
QString estraiFrammentoHtml(const QString &risposta)
{
    QJsonParseError errore;
    const QJsonDocument doc = QJsonDocument::fromJson(risposta.toUtf8(), &errore);
    if (errore.error == QJsonParseError::NoError && doc.isObject())
        return doc.object().value(QStringLiteral("message")).toString();
    return risposta;
}

// converte un frammento HTML in testo semplice, eliminando le righe vuote ripetute
QString htmlInTesto(const QString &html)
{
    QTextDocument documento;
    documento.setHtml(html);
    QString testo = documento.toPlainText();
    testo.replace(QChar::Nbsp, QLatin1Char(' '));

    //il sito pubblica a volte caratteri Windows-1252 (virgolette, trattini...) codificati come
    //caratteri di controllo C1 (U+0080-U+009F): si convertono nel carattere Unicode corretto
    static const QHash<char16_t, char16_t> cp1252 = {
        {0x80, 0x20AC}, {0x85, 0x2026}, {0x91, 0x2018}, {0x92, 0x2019}, {0x93, 0x201C},
        {0x94, 0x201D}, {0x96, 0x2013}, {0x97, 0x2014}, {0x99, 0x2122},
    };
    for (qsizetype i = testo.size() - 1; i >= 0; i--)
    {
        const char16_t c = testo.at(i).unicode();
        if (c < 0x80 || c > 0x9F)
            continue;
        if (cp1252.contains(c))
            testo[i] = QChar(cp1252.value(c));
        else
            testo.remove(i, 1);
    }

    QStringList righe;
    const QStringList tutte = testo.split(QLatin1Char('\n'));
    for (const QString &riga : tutte)
    {
        const QString pulita = riga.simplified();
        if (pulita.isEmpty() && (righe.isEmpty() || righe.last().isEmpty()))
            continue;
        righe.append(pulita);
    }
    while (!righe.isEmpty() && righe.last().isEmpty())
        righe.removeLast();
    return righe.join(QLatin1Char('\n'));
}

// tag di apertura generico: gestisce '>' dentro valori fra apici
const QString &tagAperto()
{
    static const QString s = QStringLiteral("(?:\"[^\"]*\"|'[^']*'|[^>\"'])*");
    return s;
}

} // namespace

QString AvvisoTrenord::sommario() const
{
    //salta le intestazioni ("LINEA S11", "LINEE S1 - S2", "AGGIORNAMENTO"...), cioè le righe brevi
    //scritte tutte in maiuscolo, e restituisce la prima riga utile
    const QStringList righe = m_testo.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &riga : righe)
    {
        const bool intestazione = riga.size() < 40 && riga == riga.toUpper() && riga != riga.toLower();
        if (!intestazione)
            return riga;
    }
    return righe.value(0);
}

ParserTrenord::ParserTrenord(SchedaQViaggiaTreno *scheda)
{
    m_scheda = scheda;
}

//analizza la lista delle linee restituita da https://www.trenord.it/rest/render/shoulder-lines
//ogni linea è un elemento <a data-code=".." data-direttrice=".." data-name=".."> che contiene un
//<div class="status-line ... green-line|critical ..."> con un tooltip che descrive lo stato
bool ParserTrenord::analizzaListaDirettrici(const QString &rispostaTN)
{
    static const QRegularExpression reA(QStringLiteral("<a(?![\\w-])") + tagAperto() + QStringLiteral(">"),
                                        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reFineA(QStringLiteral("</a\\s*>"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reDiv(QStringLiteral("<div(?![\\w-])") + tagAperto() + QStringLiteral(">"),
                                          QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reTooltip(QStringLiteral("class=\"tooltip\"\\s*>([^<]*)<"),
                                              QRegularExpression::CaseInsensitiveOption);

    const QString html = estraiFrammentoHtml(rispostaTN);

    QList<TrenordVT::Linea> linee;
    QQueue<QString> direttrici;

    QRegularExpressionMatchIterator it = reA.globalMatch(html);
    while (it.hasNext())
    {
        const QRegularExpressionMatch a = it.next();
        const Attributi attributi = estraiAttributi(a.captured(0));
        const QString codice = attributi.value(QStringLiteral("data-code")).trimmed();
        if (codice.isEmpty())
            continue;

        TrenordVT::Linea linea;
        linea.codice = codice;
        linea.nome = attributi.value(QStringLiteral("data-name")).trimmed();
        linea.direttrice = attributi.value(QStringLiteral("data-direttrice")).trimmed();

        const qsizetype inizio = a.capturedEnd(0);
        const QRegularExpressionMatch fine = reFineA.match(html, inizio);
        const QString contenuto = fine.hasMatch() ? html.mid(inizio, fine.capturedStart(0) - inizio)
                                                  : html.mid(inizio);

        QRegularExpressionMatchIterator itDiv = reDiv.globalMatch(contenuto);
        while (itDiv.hasNext())
        {
            const Attributi div = estraiAttributi(itDiv.next().captured(0));
            if (!haClasse(div, QStringLiteral("status-line")))
                continue;

            const QRegularExpressionMatch tooltip = reTooltip.match(contenuto);
            //il tooltip può contenere entità con nome (es. "criticit&agrave;"): si decodifica come HTML
            linea.descrizioneStato = tooltip.hasMatch() ? htmlInTesto(tooltip.captured(1)).simplified()
                                                        : QString();
            static const QRegularExpression reGravi(QStringLiteral("\\bgravi\\b"),
                                                    QRegularExpression::CaseInsensitiveOption);
            if (haClasse(div, QStringLiteral("green-line")))
                linea.stato = TrenordVT::LineaRegolare;
            else if (reGravi.match(linea.descrizioneStato).hasMatch())
                linea.stato = TrenordVT::LineaConGraviCriticita;
            else
                linea.stato = TrenordVT::LineaConCriticita;
            break;
        }

        //la stessa linea può comparire in più gruppi: si considera solo la prima occorrenza
        bool giaPresente = false;
        for (const TrenordVT::Linea &l : std::as_const(linee))
            if (l.codice == linea.codice)
                giaPresente = true;
        if (giaPresente)
            continue;

        linee.append(linea);
        if (linea.stato == TrenordVT::LineaConCriticita || linea.stato == TrenordVT::LineaConGraviCriticita)
            direttrici.enqueue(linea.codice);
    }

    if (linee.isEmpty())
        //qualcosa è andato storto, restituisci false
        return false;

    m_linee = linee;
    m_direttrici = direttrici;
    return true;
}

//analizza la scheda di una linea restituita da https://www.trenord.it/rest/render/line-details?code=..
//ogni avviso è un <div class="item N tipo"> (tipo = info, warning, critical) che contiene la data
//in formato ISO in <span class="news-date">, l'etichetta dello stato in <p class="p2"> e il testo
//dentro <div class="... body-texts ...">
bool ParserTrenord::analizzaDettagliLinea(const QString &rispostaTN, const QString &nomeLinea,
                                          QList<AvvisoTrenord> &avvisi)
{
    static const QRegularExpression reItem(QStringLiteral("<div\\s+class=\"item\\s+\\d+\\s+([\\w-]+)\\s*\"\\s*>"),
                                           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reData(QStringLiteral("class=\"news-date\"\\s*>\\s*([^<\\s]+)\\s*<"),
                                           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reStato(QStringLiteral("status-line[^\"]*\"\\s*>.*?<p class=\"p2\"\\s*>\\s*([^<]*?)\\s*</p>"),
                                            QRegularExpression::CaseInsensitiveOption
                                                | QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression reTesto(QStringLiteral("<div[^>]*class=\"[^\"]*body-texts[^\"]*\"[^>]*>"),
                                            QRegularExpression::CaseInsensitiveOption);

    const QString html = estraiFrammentoHtml(rispostaTN);
    if (!html.contains(QStringLiteral("specific-line")))
        return false;

    QList<QRegularExpressionMatch> item;
    QRegularExpressionMatchIterator it = reItem.globalMatch(html);
    while (it.hasNext())
        item.append(it.next());

    for (int i = 0; i < item.size(); i++)
    {
        const qsizetype inizio = item.at(i).capturedEnd(0);
        const qsizetype fine = (i + 1 < item.size()) ? item.at(i + 1).capturedStart(0) : html.size();
        const QString blocco = html.mid(inizio, fine - inizio);

        AvvisoTrenord avviso;
        avviso.impostaDirettrice(nomeLinea);

        const QString tipo = item.at(i).captured(1).toLower();
        if (tipo == QLatin1String("critical"))
            avviso.impostaGravita(TrenordVT::AvvisoCritico);
        else if (tipo == QLatin1String("warning"))
            avviso.impostaGravita(TrenordVT::AvvisoAttenzione);
        else
            avviso.impostaGravita(TrenordVT::AvvisoInformativo);

        const QRegularExpressionMatch data = reData.match(blocco);
        if (data.hasMatch())
            avviso.impostaOrario(QDateTime::fromString(data.captured(1), Qt::ISODateWithMs).toLocalTime());

        const QRegularExpressionMatch stato = reStato.match(blocco);
        if (stato.hasMatch())
            avviso.impostaStato(htmlInTesto(stato.captured(1)).simplified());

        const QRegularExpressionMatch testo = reTesto.match(blocco);
        if (testo.hasMatch())
            avviso.impostaTesto(htmlInTesto(blocco.mid(testo.capturedEnd(0))));

        if (!avviso.testo().isEmpty())
            avvisi.append(avviso);
    }

    return true;
}

QList<AvvisoTrenord> ParserTrenord::unisciAvvisi(const QList<AvvisoTrenord> &avvisi)
{
    QList<AvvisoTrenord> risultato;
    for (const AvvisoTrenord &avviso : avvisi)
    {
        bool unito = false;
        for (AvvisoTrenord &esistente : risultato)
        {
            //lo stesso avviso pubblicato su più linee può differire nei dettagli (es. collegamenti):
            //si considerano uguali avvisi con stessa data, gravità e riga iniziale
            if (esistente.orario() == avviso.orario() && esistente.gravita() == avviso.gravita()
                && esistente.sommario() == avviso.sommario())
            {
                if (!esistente.direttrice().split(QStringLiteral(", ")).contains(avviso.direttrice()))
                    esistente.impostaDirettrice(esistente.direttrice() + QStringLiteral(", ") + avviso.direttrice());
                unito = true;
                break;
            }
        }
        if (!unito)
            risultato.append(avviso);
    }

    //prima i più gravi, poi i più recenti
    std::stable_sort(risultato.begin(), risultato.end(), [](const AvvisoTrenord &a, const AvvisoTrenord &b) {
        if (a.gravita() != b.gravita())
            return a.gravita() > b.gravita();
        return a.orario() > b.orario();
    });
    return risultato;
}

ModelloAvvisiTrenord::ModelloAvvisiTrenord(QObject *parent) : QAbstractTableModel(parent)
{
}

//restituisce il numero di righe nel modello, cioè il numero complessivo di avvisi
int ModelloAvvisiTrenord::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_avvisi.count());
}

//restituisce il numero di colonne nel modello: data/ora - linea - stato - avviso
int ModelloAvvisiTrenord::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : numeroColonne;
}

QVariant ModelloAvvisiTrenord::headerData(int section, Qt::Orientation orientation, int role) const
{
    // usa l'implementazione di default per restituire i dati dell'header verticale
    if (orientation == Qt::Vertical)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (role == Qt::DisplayRole)
    {
        switch(section)
        {
        case colOrario: return QString::fromUtf8("Data/ora");
        case colLinea: return QString::fromUtf8("Linea");
        case colStato: return QString::fromUtf8("Stato");
        case colAvviso: return QString::fromUtf8("Avviso");
        default: return QVariant();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant ModelloAvvisiTrenord::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_avvisi.count())
        return QVariant();

    const AvvisoTrenord &avviso = m_avvisi.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case colOrario: return QLocale().toString(avviso.orario(), QLocale::ShortFormat);
        case colLinea: return avviso.direttrice();
        case colStato: return avviso.stato();
        case colAvviso: return avviso.sommario();
        default: return QVariant();
        }

    case Qt::ToolTipRole:
        return index.column() == colLinea ? avviso.direttrice() : avviso.testo();

    case Qt::BackgroundRole:
        switch (avviso.gravita())
        {
        case TrenordVT::AvvisoCritico: return QColor(255, 210, 210);
        case TrenordVT::AvvisoAttenzione: return QColor(255, 240, 190);
        default: return QVariant();
        }

    case Qt::ForegroundRole:
        //sfondo chiaro forzato: il testo deve restare scuro anche con temi scuri
        if (avviso.gravita() != TrenordVT::AvvisoInformativo)
            return QColor(Qt::black);
        return QVariant();

    default:
        return QVariant();
    }
}

void ModelloAvvisiTrenord::impostaAvvisi(const QList<AvvisoTrenord> &avvisi)
{
    beginResetModel();
    m_avvisi = avvisi;
    endResetModel();
}
