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

#include "download_trenord.h"
#include "schedaavvisitrenord.h"
#include "qviaggiatreno.h"
#include "parser_trenord.h"

#include <QLocale>
#include <QSettings>

SchedaAvvisiTrenord::SchedaAvvisiTrenord(QViaggiaTreno *parent, const unsigned int intervalloStandard) :
    SchedaQViaggiaTreno(parent, tsAvvisiTrenord, intervalloStandard)
{
    m_stato = statoNuovaScheda;

    //crea il widget con la tabella contenente gli avvisi
    m_parser = new ParserTrenord(this);
    m_avvisi = new ModelloAvvisiTrenord(this);

    m_widgetAvvisi = new WidgetAvvisiTrenord(this, m_avvisi);
    addWidget(m_widgetAvvisi);

    //connessioni
    connect(this, &SchedaAvvisiTrenord::statoCambiato, parent, &QViaggiaTreno::aggiornaStatoScheda);
    connect(this, &SchedaAvvisiTrenord::messaggioStatus, parent, &QViaggiaTreno::mostraMessaggioStatusBar);
    connect(this, &SchedaAvvisiTrenord::aggiornaListaDirettrici, qViaggiaTreno()->downloadTrenord(), &DownloadTrenord::aggiornaListaDirettrici);
}

//parser e modello hanno questa scheda come parent: vengono distrutti da Qt
SchedaAvvisiTrenord::~SchedaAvvisiTrenord() = default;

void SchedaAvvisiTrenord::avvia()
{
    SchedaQViaggiaTreno::avvia();

    //se un aggiornamento è già in corso lo stato corretto resta "in aggiornamento"
    if (m_inAggiornamento)
        cambiaStato(statoInAggiornamento);
}

void SchedaAvvisiTrenord::aggiorna()
{
    //evita di sovrapporre due aggiornamenti
    if (m_inAggiornamento)
        return;

    m_inAggiornamento = true;
    cambiaStato(statoInAggiornamento);

    emit aggiornaListaDirettrici();
}

void SchedaAvvisiTrenord::salvaScheda(QSettings &settings)
{
    settings.setValue("tipo scheda", "avvisi trenord");
    settings.setValue("intervallo", intervallo());
}

//è arrivata la lista delle linee: si scaricano le schede delle sole linee con criticità
void SchedaAvvisiTrenord::downloadFinito(const QString &rispostaTN)
{
    if (!m_parser->analizzaListaDirettrici(rispostaTN))
    {
        downloadFallito(QString::fromUtf8("risposta del sito Trenord non riconosciuta"));
        return;
    }

    const QList<TrenordVT::Linea> linee = m_parser->linee();
    m_nomiLinee.clear();
    QStringList conCriticita;
    for (const TrenordVT::Linea &linea : linee)
    {
        m_nomiLinee.insert(linea.codice, linea.nome);
        if (linea.stato == TrenordVT::LineaConCriticita || linea.stato == TrenordVT::LineaConGraviCriticita)
            conCriticita.append(QString::fromUtf8("%1 (%2)").arg(linea.nome, linea.codice));
    }

    if (conCriticita.isEmpty())
        m_widgetAvvisi->impostaRiepilogo(QString::fromUtf8("Circolazione regolare su tutte le %1 linee").arg(linee.count()));
    else
        m_widgetAvvisi->impostaRiepilogo(QString::fromUtf8("Linee con criticità: %1 su %2 — %3")
                                         .arg(conCriticita.count()).arg(linee.count())
                                         .arg(conCriticita.join(QStringLiteral(", "))));

    const QQueue<QString> coda = m_parser->listaDirettrici();
    const QStringList codici(coda.begin(), coda.end());
    m_lineeInAttesa = QSet<QString>(codici.begin(), codici.end());
    m_avvisiInArrivo.clear();
    m_lineeNonScaricate.clear();

    if (m_lineeInAttesa.isEmpty())
        completaAggiornamento();
    else
        qViaggiaTreno()->downloadTrenord()->scaricaAvvisi(this, codici);
}

void SchedaAvvisiTrenord::downloadFallito(const QString &errore)
{
    m_inAggiornamento = false;
    m_lineeInAttesa.clear();
    m_avvisiInArrivo.clear();

    SchedaQViaggiaTreno::downloadFallito(errore);
    m_widgetAvvisi->impostaAggiornamento(QString::fromUtf8("Aggiornamento non riuscito: %1").arg(errore), true);
}

void SchedaAvvisiTrenord::dettagliLineaScaricati(const QString &codice, const QString &risposta)
{
    if (!m_lineeInAttesa.contains(codice))
        return;

    const QString nome = m_nomiLinee.value(codice, codice);
    if (!ParserTrenord::analizzaDettagliLinea(risposta, QString::fromUtf8("%1 (%2)").arg(nome, codice), m_avvisiInArrivo))
        m_lineeNonScaricate.append(codice);

    lineaCompletata(codice);
}

void SchedaAvvisiTrenord::dettagliLineaNonScaricati(const QString &codice, const QString &errore)
{
    if (!m_lineeInAttesa.contains(codice))
        return;

    m_lineeNonScaricate.append(codice);
    emit messaggioStatus(QString::fromUtf8("Avvisi della linea %1 non scaricati: %2").arg(codice, errore));
    lineaCompletata(codice);
}

void SchedaAvvisiTrenord::lineaCompletata(const QString &codice)
{
    m_lineeInAttesa.remove(codice);
    if (m_lineeInAttesa.isEmpty())
        completaAggiornamento();
}

//tutte le schede delle linee sono arrivate: aggiorna in un colpo solo il modello
void SchedaAvvisiTrenord::completaAggiornamento()
{
    m_inAggiornamento = false;
    m_avvisi->impostaAvvisi(ParserTrenord::unisciAvvisi(m_avvisiInArrivo));
    m_avvisiInArrivo.clear();

    m_ultimoAgg = QDateTime::currentDateTime();
    QString messaggio = QString::fromUtf8("Ultimo aggiornamento: %1")
                            .arg(QLocale().toString(m_ultimoAgg, QLocale::ShortFormat));
    if (!m_lineeNonScaricate.isEmpty())
        messaggio += QString::fromUtf8(" (avvisi non disponibili per: %1)").arg(m_lineeNonScaricate.join(QStringLiteral(", ")));
    m_widgetAvvisi->impostaAggiornamento(messaggio, !m_lineeNonScaricate.isEmpty());

    cambiaStato(fermata() ? statoMonitoraggioFermato : statoMonitoraggioAttivo);
    emit messaggioStatus(QString::fromUtf8("Aggiornati avvisi Trenord"));
}
