/***************************************************************************
 *   Copyright (C) 2010-2011 by fra74                                           *
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

#include "download_viaggiatreno.h"
#include "qviaggiatreno.h"
#include "schedaviaggiatreno.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace {
//indirizzo base del servizio mobile di ViaggiaTreno
const QString s_urlBaseVT = QStringLiteral("http://mobile.viaggiatreno.it/vt_pax_internet/mobile");
//timeout (in ms) delle richieste di download dati
const int s_timeoutRichiesta = 30000;

//costruisce un corpo application/x-www-form-urlencoded (valori in UTF-8, percent-encoded, spazio come '+')
QByteArray corpoForm(const QList<QPair<QString, QString>>& parametri)
{
    QByteArray corpo;
    for (const auto& p : parametri)
    {
        if (!corpo.isEmpty())
            corpo += '&';
        corpo += QUrl::toPercentEncoding(p.first).replace("%20", "+");
        corpo += '=';
        corpo += QUrl::toPercentEncoding(p.second).replace("%20", "+");
    }
    return corpo;
}

//imposta intestazioni e timeout comuni a tutte le richieste dati
void impostaRichiesta(QNetworkRequest& request, const QUrl& url, QObject* item)
{
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("QViaggiaTreno/") + QCoreApplication::applicationVersion());
    request.setTransferTimeout(s_timeoutRichiesta);
    request.setOriginatingObject(item);
}
}

DownloadViaggiaTreno::DownloadViaggiaTreno(QViaggiaTreno* qvt, QNetworkAccessManager *nam)
    : m_qvt(qvt), m_nam(nam)
{
    //crea e connette i timer
    m_timerDownload = new QTimer(this);
    m_timerControlloVT = new QTimer(this);
    connect(m_timerControlloVT, &QTimer::timeout, this, &DownloadViaggiaTreno::controllaViaggiaTreno);
    connect(m_timerDownload, &QTimer::timeout, this, &DownloadViaggiaTreno::download);
}

QString DownloadViaggiaTreno::correggiOutputVT(QString testoVT)
{
    //effettua alcune sostituzione nel codice XHTML generato da ViaggiaTreno, che NON è valido
    //sostituisci <br> con <br/>
    QString temp = testoVT.simplified();
    //sostutuisce l'entità per gli accenti....
    temp.replace("&#039;", "'");
    //sostituisce gli ampersend negli URL con &amp;
    temp.replace("&", "&amp;");
    // sostituisce <br> con <br/>
    temp.replace("<br>", "<br/>");
    temp.replace("</strong> <br/> <br/>", "</strong> </p> <br/>");

    return temp;
}


//slot
// questo slot viene richiamato ad intervalli prefissati
//lo slot controlla se ci sono altre richieste di schede da caricare in coda, in caso positivo scarica la prima e richiama la funzione
//privata più opportuna per impostare la richiesta HTTP
//in futuro in questo slot si verificherà anche se la scheda è già nella cache
void DownloadViaggiaTreno::download()
{
    // se non ci sono richieste in coda esce immediatamente
    if (m_codaDownload.isEmpty())
        return;

    //altrimenti prende il primo elemento in coda
    DownloadViaggiaTrenoItem * item = m_codaDownload.dequeue();

    //individua il tipo di scheda richiesta a viaggiaTreno e richiama la funzione privata giusta
    //per generare la richiesta HTTP
    switch(item->tipoScheda())
        {
        case StazioneConNome: richiestaHTTPStazioneConNome(item); break;
        case StazioneConCodice: richiestaHTTPStazioneConCodice(item); break;
        case RiepilogoTreno: richiestaHTTPRiepilogoTreno(item); break;
        case RiepilogoTrenoConOrigine: richiestaHTTPRiepilogoTrenoConOrigine(item); break;
        case DettagliTreno: richiestaHTTPDettagliTreno(item); break;
        case DettagliTrenoConOrigine: richiestaHTTPDettagliTrenoConOrigine(item); break;
        }
}

// slot richiamati dalle schede per mettere in coda un download
//gli item in coda hanno come parent il downloader, così vengono liberati alla sua distruzione
void DownloadViaggiaTreno::downloadStazione(quint32 idScheda, const QString &nomeStazione)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, StazioneConNome);
    item->setParent(this);
    item->impostaDato("NomeStazione", nomeStazione);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadStazioneCodice(quint32 idScheda, const QString &codiceStazione)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, StazioneConCodice);
    item->setParent(this);
    item->impostaDato("CodiceStazione", codiceStazione);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadRiepilogoTreno(quint32 idScheda, const QString &numero)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, RiepilogoTreno);
    item->setParent(this);
    item->impostaDato("Numero", numero);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadRiepilogoTreno(quint32 idScheda, const QString &numero, const QString &codiceStazOrigine)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, RiepilogoTrenoConOrigine);
    item->setParent(this);
    item->impostaDato("Numero", numero);
    item->impostaDato("CodiceStazione", codiceStazOrigine );

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadDettagliTreno(quint32 idScheda, const QString &numero)
{
    DownloadViaggiaTrenoItem * item = new DownloadViaggiaTrenoItem(idScheda, DettagliTreno);
    item->setParent(this);
    item->impostaDato("Numero", numero);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadDettagliTreno(quint32 idScheda, const QString &numero, const QString &codiceStazOrigine)
{
    DownloadViaggiaTrenoItem * item = new DownloadViaggiaTrenoItem(idScheda, DettagliTrenoConOrigine);
    item->setParent(this);
    item->impostaDato("Numero", numero);
    item->impostaDato("CodiceStazione", codiceStazOrigine );

    m_codaDownload.enqueue(item);
}

//costruisce un'istanza della classe DownloadViaggiaTrenoItem
DownloadViaggiaTrenoItem::DownloadViaggiaTrenoItem(quint32 idScheda, TipoSchedaViaggiaTreno tipoScheda)
{
    m_scheda = idScheda;
    m_tipoSchedaVT = tipoScheda;

    //imposta data/ora richiesta
    m_dataEOra = QDateTime::currentDateTime();
}

//invia una richiesta POST con corpo form-urlencoded e collega la risposta a downloadEffettuato
void DownloadViaggiaTreno::inviaPost(DownloadViaggiaTrenoItem *item, const QString& percorso,
                                     const QList<QPair<QString, QString>>& parametri)
{
    QNetworkRequest request;
    impostaRichiesta(request, QUrl(s_urlBaseVT + percorso), item);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = m_nam->post(request, corpoForm(parametri));
    connect(reply, &QNetworkReply::finished, this, &DownloadViaggiaTreno::downloadEffettuato);
}

//invia una richiesta GET e collega la risposta a downloadEffettuato
void DownloadViaggiaTreno::inviaGet(DownloadViaggiaTrenoItem *item, const QString& indirizzo)
{
    QNetworkRequest request;
    impostaRichiesta(request, QUrl(indirizzo), item);
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &DownloadViaggiaTreno::downloadEffettuato);
}

//invia richiesta HTTP al server di ViaggiaTreno per ottenere la scheda della stazione, fornendo come parametro il nome della stazione
void DownloadViaggiaTreno::richiestaHTTPStazioneConNome(DownloadViaggiaTrenoItem *item)
{
    inviaPost(item, QStringLiteral("/stazione?lang=IT"), {{QStringLiteral("stazione"), item->dato("NomeStazione")}});
}

//invia richiesta HTTP al server di ViaggiaTreno per ottenere la scheda della stazione, fornendo come parametro il codice della stazione
void DownloadViaggiaTreno::richiestaHTTPStazioneConCodice(DownloadViaggiaTrenoItem *item)
{
    inviaPost(item, QStringLiteral("/stazione?lang=IT"), {{QStringLiteral("codiceStazione"), item->dato("CodiceStazione")}});
}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda di un treno dato il suo numero
void DownloadViaggiaTreno::richiestaHTTPRiepilogoTreno(DownloadViaggiaTrenoItem *item)
{
    inviaPost(item, QStringLiteral("/numero"), {
        {QStringLiteral("numeroTreno"), item->dato("Numero")},
        {QStringLiteral("tipoRicerca"), QStringLiteral("numero")},
        {QStringLiteral("lang"), QStringLiteral("IT")}});
}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda di un treno dato il suo numero
//ed il codice della stazione di origine
void DownloadViaggiaTreno::richiestaHTTPRiepilogoTrenoConOrigine(DownloadViaggiaTrenoItem *item)
{
    inviaPost(item, QStringLiteral("/numero"), {
        {QStringLiteral("cbxTreno"), item->dato("Numero") + QLatin1Char(';') + item->dato("CodiceStazione")},
        {QStringLiteral("tipoRicerca"), QStringLiteral("numero")},
        {QStringLiteral("lang"), QStringLiteral("IT")}});
}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda con i dettagli di un treno dato il suo numero
void DownloadViaggiaTreno::richiestaHTTPDettagliTreno(DownloadViaggiaTrenoItem *item)
{
    inviaGet(item, QString(s_urlBaseVT + "/scheda?dettaglio=visualizza&numeroTreno=%1&tipoRicerca=numero&lang=IT")
             .arg(item->dato("Numero")));
}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda con i dettagli di un treno
//dati il suo numero ed il codice della stazione di origine
void DownloadViaggiaTreno::richiestaHTTPDettagliTrenoConOrigine(DownloadViaggiaTrenoItem *item)
{
    inviaGet(item, QString(s_urlBaseVT + "/scheda?dettaglio=visualizza&numeroTreno=%1&&codLocOrig=%2&tipoRicerca=numero&lang=IT")
             .arg(item->dato("Numero"), item->dato("CodiceStazione")));
}


//slot
//questo slot viene chiamato quando un download da ViaggiaTreno è terminato
void DownloadViaggiaTreno::downloadEffettuato()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    //ricava l'item corrispondente a questo download
    DownloadViaggiaTrenoItem * item = qobject_cast<DownloadViaggiaTrenoItem*>(reply->request().originatingObject());
    if (!item)
        return;

    //recupera puntatore alla scheda
    SchedaQViaggiaTreno* scheda = m_qvt->scheda(item->idScheda());

    //se la scheda è ancora aperta notifica l'esito, altrimenti non fare nulla
    if (scheda)
    {
        if (reply->error() != QNetworkReply::NoError)
            scheda->downloadFallito(reply->errorString());
        else
        {
            //il file XHTML generato da viaggiatreno non è sintatticamente corretto, vanno corretti alcuni errori
            const QString risposta = correggiOutputVT(QString::fromUtf8(reply->readAll()));
            scheda->downloadFinito(risposta);
        }
    }

    //in questa sezione andrà aggiunta il testo della scheda alla cache se necessario

    //l'item non serve più, liberare memoria
    delete item;
}


//slot
//effettua un controllo periodico (asincrono) sul corretto funzionamento del servizio web di viaggiatreno
void DownloadViaggiaTreno::controllaViaggiaTreno()
{
    //se c'è già un controllo in corso non ne avvia un altro
    if (m_controlloInCorso)
        return;

    //prova a scaricare la pagina di query di un treno di ViaggiaTreno
    QNetworkRequest request;
    request.setUrl(QUrl(s_urlBaseVT));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("QViaggiaTreno/") + QCoreApplication::applicationVersion());
    request.setTransferTimeout(m_qvt->configurazione().intervalloControlloVT()*1000);

    m_controlloInCorso = m_nam->get(request);
    connect(m_controlloInCorso, &QNetworkReply::finished, this, &DownloadViaggiaTreno::controlloTerminato);
}

//conclusione del controllo: il servizio funziona se non ci sono errori e la pagina contiene "Numero treno"
void DownloadViaggiaTreno::controlloTerminato()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    m_controlloInCorso = nullptr;

    bool funziona = false;
    if (reply->error() == QNetworkReply::NoError)
        funziona = QString::fromUtf8(reply->readAll()).contains("Numero treno");

    emit statoViaggiaTreno(funziona);
    if (funziona)
    {
        if (!m_timerDownload->isActive())
            m_timerDownload->start();
    }
    else
        //ferma i donwload da ViaggiaTreno
        m_timerDownload->stop();
}

//avvia il downloader
void DownloadViaggiaTreno::avvia()
{
    //inizializza i timer
    m_timerDownload->setInterval(m_qvt->configurazione().intervalloQueryVT()*1000);
    m_timerControlloVT->setInterval(m_qvt->configurazione().intervalloControlloVT()*60*1000);
    //avvia il timer per il controllo periodico del funzionamento di ViaggiaTreno
    m_timerControlloVT->start();

    //esegue un controllo preliminare sul funzionamento di Viaggiatreno;
    //il timer di download parte solo se il controllo ha esito positivo
    controllaViaggiaTreno();
}
