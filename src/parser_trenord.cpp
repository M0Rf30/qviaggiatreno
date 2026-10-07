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
#include "utils.h"
#include <QHash>
#include <QRegularExpression>
#include <QStringList>

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

} // namespace

ParserTrenord::ParserTrenord(SchedaQViaggiaTreno *scheda)
{
    m_scheda = scheda;
}

//analizza il testo della pagina all'URL http://www.trenord.it/mobile/it/breaking-news.aspx
// che contiene la lista delle direttrici trenord con alla destra del nome della direttrice
// una icona che può essere rossa o verde nel caso siano o no presenti avvisi aggiornati
//restituisce true se l'analisi è andata a buon fine e nel caso crea anche una coda di QString
//ciascuna contente l'indirizzo con la pagina dei dettagli di una direttrice per cui
//sono presenti avvisi
bool ParserTrenord::analizzaListaDirettrici(const QString &rispostaTN)
{
    // tag di apertura generico: gestisce '>' dentro valori fra apici
    static const QString tagAperto = QStringLiteral("(?:\"[^\"]*\"|'[^']*'|[^>\"'])*");
    static const QRegularExpression reLi(QStringLiteral("<li(?![\\w-])") + tagAperto + QStringLiteral(">"),
                                         QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reFineLi(QStringLiteral("</li\\s*>"),
                                             QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reImg(QStringLiteral("<img(?![\\w-])") + tagAperto + QStringLiteral(">"),
                                          QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reA(QStringLiteral("<a(?![\\w-])") + tagAperto + QStringLiteral(">"),
                                        QRegularExpression::CaseInsensitiveOption);

    bool trovato = false;
    QQueue<QString> direttrici;

    //per ogni <li> con classe "direttrici-item" cerca il primo <img> con classe "indicator":
    //se l'immagine contiene nel nome "indicator-on" la direttrice ha avvisi aggiornati e
    //si accoda l'indirizzo del primo <a> contenuto nell'elemento <li>
    QRegularExpressionMatchIterator it = reLi.globalMatch(rispostaTN);
    while (it.hasNext())
    {
        const QRegularExpressionMatch li = it.next();
        if (!haClasse(estraiAttributi(li.captured(0)), QStringLiteral("direttrici-item")))
            continue;

        trovato = true;

        const qsizetype inizio = li.capturedEnd(0);
        const QRegularExpressionMatch fine = reFineLi.match(rispostaTN, inizio);
        const QString contenuto = fine.hasMatch()
            ? rispostaTN.mid(inizio, fine.capturedStart(0) - inizio)
            : rispostaTN.mid(inizio);

        QRegularExpressionMatchIterator itImg = reImg.globalMatch(contenuto);
        while (itImg.hasNext())
        {
            const Attributi img = estraiAttributi(itImg.next().captured(0));
            if (!haClasse(img, QStringLiteral("indicator")))
                continue;

            if (img.value(QStringLiteral("src")).contains(QStringLiteral("indicator-on")))
            {
                const QRegularExpressionMatch a = reA.match(contenuto);
                direttrici.enqueue(a.hasMatch()
                                       ? estraiAttributi(a.captured(0)).value(QStringLiteral("href"))
                                       : QString());
            }
            break; // conta solo il primo <img class="indicator">
        }
    }

    if (!trovato)
        //qualcosa è andato storto, restituisci false
        return false;

    m_direttrici = direttrici;
    return true;
}

ModelloAvvisiTrenord::ModelloAvvisiTrenord(QWidget *parent) : QAbstractTableModel(parent)
{

}

//restituisce il numero di righe nel modello, cioè il numero complessivo di avvisi
int ModelloAvvisiTrenord::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_avvisi.count();
}

//restituisce il numero di colonne nel modello, attualmente 3: data/ora avviso - direttrice - avviso
int ModelloAvvisiTrenord::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 3;
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
        case 0: return QString::fromUtf8("Data/ora"); break;
        case 1: return QString::fromUtf8("Direttrice"); break;
        case 2: return QString::fromUtf8("Testo avviso"); break;
        default: return QVariant();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant ModelloAvvisiTrenord::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    //TODO: da implementare
    return QVariant();
}
