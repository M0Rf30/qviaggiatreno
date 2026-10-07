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
#include "schedaviaggiatreno.h"
#include "qviaggiatreno.h"

//TODO Rimuvovere l'include seguente quando non è più necessario fare debug
#include "utils.h"
#include "parser_trenord.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
//indirizzo base del sito Trenord
const QString s_urlBaseTrenord = QStringLiteral("http://www.trenord.it");
//timeout (in ms) delle richieste
const int s_timeoutRichiesta = 30000;
}

DownloadTrenord::DownloadTrenord(QViaggiaTreno *qvt, QNetworkAccessManager *nam)
{
    m_qvt = qvt;
    m_nam = nam;
    m_timerAvvisi = new QTimer(this);
    connect(m_timerAvvisi, &QTimer::timeout, this, &DownloadTrenord::scaricaNuovaDirettrice);
}

//richiede al sito webTrenord la pagina con la lista delle direttrici
void DownloadTrenord::aggiornaListaDirettrici()
{
    QNetworkRequest request;

    //invia una richiesta HTTP GET per scaricare la pagina con la lista delle direttrici
    request.setUrl(QUrl(s_urlBaseTrenord + "/mobile/it/breaking-news.aspx"));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("QViaggiaTreno/") + QCoreApplication::applicationVersion());
    request.setTransferTimeout(s_timeoutRichiesta);
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

void DownloadTrenord::scaricaAvvisi(ParserTrenord *parser)
{
    m_parser = parser;
    m_coda = parser->listaDirettrici();

    if (!m_coda.count())
        //non ci sono nuovi avvisi da scaricare, inutile continuare
        return;

    //imposta il timer
    //TODO: per il momento scarica gli avvisi con un intervallo fisso di 1 s, successivamente da configurare
    m_timerAvvisi->setInterval(1000);
    m_timerAvvisi->start();
}

//questo slot viene richiamato dal timer. Ad ogni esecuzione preleva l'indirizzo della pagina di una direttrice
//corregge l'url aggiungendo http://www.trenord.it
//e avvia il download della pagina
//se non ci sono più direttrici allora semplicemente interrompe il timeout ed esce
void DownloadTrenord::scaricaNuovaDirettrice()
{
    //controlla che ci siano ancora direttrici da scaricare ed in caso negativo
    //arresta il timer ed esce
    if (!m_coda.count())
    {
        //TODO: questo e' il punto in cui si può aggiornare il modello!
        m_timerAvvisi->stop();
        return;
    }

    //TODO: il download della pagina della direttrice non è ancora implementato
    const QString url = s_urlBaseTrenord + m_coda.dequeue();
    Q_UNUSED(url)

}
