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

#include <QCoreApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace {
//indirizzo base del sito Trenord
const QString s_urlBaseTrenord = QStringLiteral("https://www.trenord.it");
//lista delle linee con il relativo stato della circolazione (risposta JSON con frammento HTML)
const QString s_percorsoLinee = QStringLiteral("/rest/render/shoulder-lines");
//scheda di una linea con gli avvisi in corso
const QString s_percorsoDettagliLinea = QStringLiteral("/rest/render/line-details");
//timeout (in ms) delle richieste
const int s_timeoutRichiesta = 30000;
//intervallo (in ms) tra il download di due schede di linea, per non sovraccaricare il sito
const int s_intervalloDettagli = 1000;
//proprietà della reply con il codice della linea scaricata
const char* const s_proprietaCodice = "codiceLinea";
}

DownloadTrenord::DownloadTrenord(QViaggiaTreno *qvt, QNetworkAccessManager *nam)
{
    m_qvt = qvt;
    m_nam = nam;
    m_timerAvvisi = new QTimer(this);
    m_timerAvvisi->setInterval(s_intervalloDettagli);
    connect(m_timerAvvisi, &QTimer::timeout, this, &DownloadTrenord::scaricaNuovaDirettrice);
}

//il servizio REST di Trenord risponde 403 se la richiesta non dichiara di accettare JSON
QNetworkRequest DownloadTrenord::creaRichiesta(const QString &percorso) const
{
    QNetworkRequest request(QUrl(s_urlBaseTrenord + percorso));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("QViaggiaTreno/") + QCoreApplication::applicationVersion());
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setTransferTimeout(s_timeoutRichiesta);
    return request;
}

//richiede al sito Trenord la lista delle linee con il loro stato
void DownloadTrenord::aggiornaListaDirettrici()
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("no_cache"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("mxp"), QStringLiteral("false"));
    query.addQueryItem(QStringLiteral("L"), QStringLiteral("0"));

    QNetworkRequest request = creaRichiesta(s_percorsoLinee + QLatin1Char('?') + query.toString(QUrl::FullyEncoded));
    request.setOriginatingObject(sender());
    QNetworkReply* reply = m_nam->get(request);

    connect(reply, &QNetworkReply::finished, this, &DownloadTrenord::downloadFinito);
}

void DownloadTrenord::downloadFinito()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    //recupera il puntatore della scheda che ha inviato la richiesta di download
    SchedaQViaggiaTreno *scheda = qobject_cast<SchedaQViaggiaTreno*>(reply->request().originatingObject());

    //controlla se la scheda è aperta, se lo è richiama il metodo downloadFinito della scheda
    if (m_qvt->schedaAperta(scheda))
    {
        if (reply->error() != QNetworkReply::NoError)
            scheda->downloadFallito(reply->errorString());
        else
            scheda->downloadFinito(QString::fromUtf8(reply->readAll()));
    }
}

void DownloadTrenord::scaricaAvvisi(SchedaAvvisiTrenord* scheda, const QStringList& codiciLinee)
{
    //elimina eventuali richieste ancora in coda per la stessa scheda (aggiornamento precedente)
    QQueue<RichiestaLinea> coda;
    for (const RichiestaLinea &r : std::as_const(m_coda))
        if (r.scheda && r.scheda != scheda)
            coda.enqueue(r);
    m_coda = coda;

    for (const QString &codice : codiciLinee)
        m_coda.enqueue({scheda, codice});

    if (m_coda.isEmpty())
        //non ci sono nuovi avvisi da scaricare, inutile continuare
        return;

    if (!m_timerAvvisi->isActive())
    {
        //scarica subito la prima linea, le successive ad intervalli regolari
        scaricaNuovaDirettrice();
        m_timerAvvisi->start();
    }
}

//questo slot viene richiamato dal timer: ad ogni esecuzione preleva dalla coda una linea e ne
//scarica la scheda. Se non ci sono più linee interrompe il timer
void DownloadTrenord::scaricaNuovaDirettrice()
{
    while (!m_coda.isEmpty())
    {
        const RichiestaLinea richiesta = m_coda.dequeue();
        //la scheda potrebbe essere stata chiusa nel frattempo
        if (!richiesta.scheda)
            continue;

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("code"), richiesta.codice);
        query.addQueryItem(QStringLiteral("L"), QStringLiteral("0"));

        QNetworkRequest request = creaRichiesta(s_percorsoDettagliLinea + QLatin1Char('?')
                                                + query.toString(QUrl::FullyEncoded));
        request.setOriginatingObject(richiesta.scheda);
        QNetworkReply *reply = m_nam->get(request);
        reply->setProperty(s_proprietaCodice, richiesta.codice);
        connect(reply, &QNetworkReply::finished, this, &DownloadTrenord::dettagliLineaScaricati);
        return;
    }

    m_timerAvvisi->stop();
}

void DownloadTrenord::dettagliLineaScaricati()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    SchedaAvvisiTrenord *scheda = qobject_cast<SchedaAvvisiTrenord*>(reply->request().originatingObject());
    if (!m_qvt->schedaAperta(scheda))
        return;

    const QString codice = reply->property(s_proprietaCodice).toString();
    if (reply->error() != QNetworkReply::NoError)
        scheda->dettagliLineaNonScaricati(codice, reply->errorString());
    else
        scheda->dettagliLineaScaricati(codice, QString::fromUtf8(reply->readAll()));
}
