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

DownloadTrenord::DownloadTrenord(QViaggiaTreno *qvt, QNetworkAccessManager *nam)
{
    m_qvt = qvt;
    m_nam = nam;
    m_parser = 0L;
    m_timerAvvisi = new QTimer(this);
}

//richiede al sito webTrenord la pagina con la lista delle direttrici
void DownloadTrenord::aggiornaListaDirettrici()
{
    QNetworkRequest request;

    //invia una richiesta HTTP GET per scaricare la pagina con la lista delle direttrici
    request.setUrl(QUrl("http://www.trenord.it/mobile/it/breaking-news.aspx"));
    request.setOriginatingObject(sender());
    QNetworkReply* reply = m_nam->get(request);

    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinito()));
}

void DownloadTrenord::downloadFinito()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString risposta = QString::fromUtf8(reply->readAll());

    //recupera il puntatore della scheda che ha inviato la richiesta di download
    SchedaQViaggiaTreno *scheda = qobject_cast<SchedaQViaggiaTreno*>(reply->request().originatingObject());

    //controlla se la scheda è aperta, se lo è richiama il metodo downloadFinito della scheda
    if (m_qvt->schedaAperta(scheda))
        scheda->downloadFinito(risposta);

    reply->deleteLater();
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
    connect(m_timerAvvisi, SIGNAL(timeout()), this, SLOT(scaricaNuovaDirettrice()));

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
        disconnect(m_timerAvvisi);
        m_timerAvvisi->stop();
        return;
    }

    QString url = QString("http://www.trenord.it%1").arg(m_coda.dequeue());

}
