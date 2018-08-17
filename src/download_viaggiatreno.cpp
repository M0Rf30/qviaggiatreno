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

#include "qviaggiatreno.h"
#include "download_viaggiatreno.h"
#include "schedaviaggiatreno.h"


DownloadViaggiaTreno::DownloadViaggiaTreno(QViaggiaTreno* qvt, QNetworkAccessManager *nam)
{
    m_nam = nam;
    m_qvt = qvt;

    //crea e connette i timer
    m_timerDownload = new QTimer(this);
    m_timerControlloVT = new QTimer(this);
    connect(m_timerControlloVT, SIGNAL(timeout()), this, SLOT(controllaViaggiaTreno()));
    connect(m_timerDownload, SIGNAL(timeout()), this, SLOT(download()));

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
    if (!m_codaDownload.count())
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
        case DettagliTrenoConOrigine: richiestaHTTPDettagliTrenoConOrigine(item);
        }
}

// slot richiamati dalle schede per mettere in coda un download
void DownloadViaggiaTreno::downloadStazione(quint32 idScheda, const QString &nomeStazione)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, StazioneConNome);
    item->impostaDato("NomeStazione", nomeStazione);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadStazioneCodice(quint32 idScheda, const QString &codiceStazione)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, StazioneConCodice);
    item->impostaDato("CodiceStazione", codiceStazione);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadRiepilogoTreno(quint32 idScheda, const QString &numero)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, RiepilogoTreno);
    item->impostaDato("Numero", numero);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadRiepilogoTreno(quint32 idScheda, const QString &numero, const QString &codiceStazOrigine)
{
    DownloadViaggiaTrenoItem *item = new DownloadViaggiaTrenoItem(idScheda, RiepilogoTrenoConOrigine);
    item->impostaDato("Numero", numero);
    item->impostaDato("CodiceStazione", codiceStazOrigine );

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadDettagliTreno(quint32 idScheda, const QString &numero)
{
    DownloadViaggiaTrenoItem * item = new DownloadViaggiaTrenoItem(idScheda, DettagliTreno);
    item->impostaDato("Numero", numero);

    m_codaDownload.enqueue(item);
}

void DownloadViaggiaTreno::downloadDettagliTreno(quint32 idScheda, const QString &numero, const QString &codiceStazOrigine)
{
    DownloadViaggiaTrenoItem * item = new DownloadViaggiaTrenoItem(idScheda, DettagliTrenoConOrigine);
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


//invia richiesta HTTP al server di ViaggiaTreno per ottenere la scheda della stazione, fornendo come parametro il nome della stazione
void DownloadViaggiaTreno::richiestaHTTPStazioneConNome(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;
    QString dati;


    dati = QString("stazione=%1").arg(item->dato("NomeStazione"));
    dati.replace(" ", "+");
   // request.setUrl(QUrl("http://mobile.viaggiatreno.it/viaggiatreno/mobile/stazione?lang=IT"));
     request.setUrl(QUrl("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/stazione?lang=IT"));
    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->post(request, dati.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));
}

//invia richiesta HTTP al server di ViaggiaTreno per ottenere la scheda della stazione, fornendo come parametro il codice della stazione
void DownloadViaggiaTreno::richiestaHTTPStazioneConCodice(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;
    QString dati;


    dati = QString("codiceStazione=%1").arg(item->dato("CodiceStazione"));
    dati.replace(" ", "+");
    //request.setUrl(QUrl("http://mobile.viaggiatreno.it/viaggiatreno/mobile/stazione?lang=IT"));
    request.setUrl(QUrl("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/stazione?lang=IT"));
    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->post(request, dati.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));

}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda di un treno dato il suo numero
void DownloadViaggiaTreno::richiestaHTTPRiepilogoTreno(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;
    QString str;

    str = QString("numeroTreno=%1&tipoRicerca=numero&lang=IT").arg(item->dato("Numero"));
 //   request.setUrl(QUrl("http://mobile.viaggiatreno.it/viaggiatreno/mobile/numero"));
     request.setUrl(QUrl("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/numero"));
    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->post(request, str.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));
}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda di un treno dato il suo numero
//ed il codice della stazione di origine
void DownloadViaggiaTreno::richiestaHTTPRiepilogoTrenoConOrigine(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;
    QString str;

    str = QString("cbxTreno=%1;").arg(item->dato("Numero"));
    str += QString("%1&tipoRicerca=numero&lang=IT").arg(item->dato("CodiceStazione"));
    //request.setUrl(QUrl("http://mobile.viaggiatreno.it/viaggiatreno/mobile/numero"));
    request.setUrl(QUrl("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/numero"));
    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->post(request, str.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));

}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda con i dettagli di un treno dato il suo numero
void DownloadViaggiaTreno::richiestaHTTPDettagliTreno(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;

   // request.setUrl(QUrl(QString("http://mobile.viaggiatreno.it/viaggiatreno/mobile/scheda?dettaglio=visualizza&numeroTreno=%1&tipoRicerca=numero&lang=IT").arg(item->dato("Numero"))));
       request.setUrl(QUrl(QString("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/scheda?dettaglio=visualizza&numeroTreno=%1&tipoRicerca=numero&lang=IT").arg(item->dato("Numero"))));
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));

}

//invia richiesta HTTP a ViaggiaTreno per ottenere la scheda con i dettagli di un treno
//dati il suo numero ed il codice della stazione di origine
void DownloadViaggiaTreno::richiestaHTTPDettagliTrenoConOrigine(DownloadViaggiaTrenoItem *item)
{
    QNetworkRequest request;

//    request.setUrl(QUrl(QString("http://mobile.viaggiatreno.it/viaggiatreno/mobile/scheda?dettaglio=visualizza&numeroTreno=%1&&codLocOrig=%2&tipoRicerca=numero&lang=IT").
    request.setUrl(QUrl(QString("http://mobile.viaggiatreno.it/vt_pax_internet/mobile/scheda?dettaglio=visualizza&numeroTreno=%1&&codLocOrig=%2&tipoRicerca=numero&lang=IT").
                        arg(item->dato("Numero")).arg(item->dato("CodiceStazione"))));
    request.setOriginatingObject(item);
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadEffettuato()));
}


//slot
//questo slot viene chiamato quando un download da ViaggiaTreno è terminato
void DownloadViaggiaTreno::downloadEffettuato()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    QString risposta = QString::fromUtf8(reply->readAll());
    //il file XHTML generato da viaggiatreno non è sintatticamente corretto, vanno corretti alcuni errori
    risposta = correggiOutputVT(risposta);

   // debugStringa(risposta);

    //ricava l'item corrispondente a questo downloadQNetworkReply

    DownloadViaggiaTrenoItem * item = qobject_cast<DownloadViaggiaTrenoItem*>(reply->request().originatingObject());

    //recupera puntatore alla scheda
    SchedaQViaggiaTreno* scheda = m_qvt->scheda(item->idScheda());

    //se la scheda è ancora aperta richiama la funzione downloadFinito della scheda, altrimenti non fare nulla
    if (scheda)
        scheda->downloadFinito(risposta);

    //in questa sezione andrà aggiunta il testo della scheda alla cache se necessario

    //l'item non serve più, liberare memoria
    delete item;

    //la risposta non serve più, liberare memoria
    reply->deleteLater();
}


//slot
//effettua un controllo periodico sul corretto funzionamento del servizio web di viaggiatreno
bool DownloadViaggiaTreno::controllaViaggiaTreno()
{
    //prova a scaricare la pagina di query di un treno di ViaggiaTreno
    QNetworkRequest request;
    //request.setUrl(QUrl("http://mobile.viaggiatreno.it/viaggiatreno/mobile/"));
    request.setUrl(QUrl("http://mobile.viaggiatreno.it/vt_pax_internet/mobile"));

    QNetworkReply *reply = m_nam->get(request);
    // usa un event loop per effettuare una richiesta sincrona
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //ed usa un timer singleshot per il timeout
    QTimer::singleShot(m_qvt->configurazione().intervalloControlloVT()*1000, &loop, SLOT(quit()));
    //esegui l'event loop
    loop.exec();

    //si è usciti dal loop o perché c'è stato un timeout o perché il download si è concluso positivamente
    //verifica se non si è concluso positivamente
    if (reply->error()!= QNetworkReply::NoError)
    {
        emit statoViaggiaTreno(false);
        //ferma i donwload da ViaggiaTreno
        m_timerDownload->stop();
        return false;
    }
    else
    {
        //il download sembra essersi concluso positivamente, estraiamo il codice HTML
        QString paginaVT = QString::fromUtf8(reply->readAll());
        //e cerchiamo se la pagina contiene il testo "Numero treno"
        if (paginaVT.contains("Numero treno"))
        {
            emit statoViaggiaTreno(true);
            return true;
        }
        else
        {
            emit statoViaggiaTreno(false);
            //ferma i donwload da ViaggiaTreno
            m_timerDownload->stop();
            return false;
        }
    }
}

//avvia il downloader
void DownloadViaggiaTreno::avvia()
{
    //inizializza i timer
    m_timerDownload->setInterval(m_qvt->configurazione().intervalloQueryVT()*1000);
    m_timerControlloVT->setInterval(m_qvt->configurazione().intervalloControlloVT()*60*1000);
    //avvia il timer per il controllo periodico del funzionamento di ViaggiaTreno
    m_timerControlloVT->start();

    //esegue un controllo preliminare sul funzionamento di Viaggiatreno
    if (!controllaViaggiaTreno())
    // viaggiatreno non funziona
        return;
    else
        m_timerDownload->start();

}
