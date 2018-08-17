/***************************************************************************
 *   Copyright (C) 2008-2011 by fra74                                           *
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


#include <QtGui>

#include "schedastazione.h"
#include "schedalistatreni.h"
#include "qviaggiatreno.h"
#include "download_viaggiatreno.h"
#include "download_trenord.h"
#include "qledindicator.h"

//numero versione
QString QViaggiaTreno::s_versione = QString("2013.7");

QViaggiaTreno::QViaggiaTreno()
{
    m_licenseDialog = 0L;

    m_nam = new QNetworkAccessManager(this);

    //crea una istanza delle varie classi per il download
    m_downloadViaggiaTreno = new DownloadViaggiaTreno(this, m_nam);
    m_downloadTrenord = new DownloadTrenord(this, m_nam);

    creaAzioni();
    creaMenu();
    creaToolBar();
    creaStatusBar();
    creaWidgetCentrale();
    setWindowTitle(QString::fromUtf8("QViaggiaTreno"));
    leggiImpostazioniFinestra();
    impostaProxy();
    //avvia le varie classi per il download
    //per il momento esiste solo la classe per il download da viaggiatreno in futuro è conveniente
    //incapsulare l'avvio in un metodo
    connect(m_downloadViaggiaTreno, SIGNAL(statoViaggiaTreno(bool)), this, SLOT(statoViaggiaTrenoCambiato(bool)));
    m_downloadViaggiaTreno->avvia();
    ripristinaSchede();
}



void QViaggiaTreno::creaAzioni()
{
    m_exitAct = new QAction(QString::fromUtf8("E&sci"), this);
    m_exitAct->setShortcut(QString("Ctrl+Q"));
    m_exitAct->setStatusTip(QString::fromUtf8("Chiude l'applicazione"));
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_licenseAct = new QAction(QString::fromUtf8("Licen&za"), this);
    m_licenseAct->setStatusTip(QString::fromUtf8("Mostra la licenza di QViaggiaTreno"));
    connect(m_licenseAct, SIGNAL(triggered()), this, SLOT(visualizzaLicenza()));

    m_aboutAct = new QAction(QString::fromUtf8("Informazioni &su QViaggiaTreno"), this);
    m_aboutAct->setStatusTip(QString::fromUtf8("Mostra la finestra di informazione sull'applicazione"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_aboutQtAct = new QAction(QString::fromUtf8("Informazioni su &Qt"), this);
    m_aboutQtAct->setStatusTip(QString::fromUtf8("Mostra la finestra di informazioni sulla libreria Qt"));
    connect(m_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    m_nuovaStazioneAct = new QAction(QString::fromUtf8("Stazione..."), this);
    m_nuovaStazioneAct->setStatusTip(QString::fromUtf8("Controlla una stazione con ViaggiaTreno"));
    m_nuovaStazioneAct->setIcon(QIcon(":/img/stazione.png"));
    connect(m_nuovaStazioneAct, SIGNAL(triggered()), this ,SLOT(nuovaStazione()));

    m_nuovoTrenoAct = new QAction(QString::fromUtf8("Treno..."), this);
    m_nuovoTrenoAct->setStatusTip(QString::fromUtf8("Controlla un treno con ViaggiaTreno"));
    m_nuovoTrenoAct->setIcon(QIcon(":/img/treno.png"));
    connect(m_nuovoTrenoAct, SIGNAL(triggered()), this, SLOT(nuovoTreno()));

    m_nuovaListaTreniAct = new QAction(QString::fromUtf8("Lista di treni"), this);
    m_nuovaListaTreniAct->setStatusTip(QString::fromUtf8("Controlla una lista di treni con ViaggiaTreno"));
    m_nuovaListaTreniAct->setIcon(QIcon(":/img/listatreni.png"));
    connect(m_nuovaListaTreniAct, SIGNAL(triggered()), this, SLOT(nuovaListaTreni()));

    m_avvisiTrenordAct = new QAction(QString::fromUtf8("Avvisi Trenord"), this);
    m_avvisiTrenordAct->setStatusTip(QString::fromUtf8("Controlla gli avvisi di Trenord"));
    m_avvisiTrenordAct->setIcon(QIcon(":/img/trenord.png"));
    connect(m_avvisiTrenordAct, SIGNAL(triggered()), this, SLOT(nuovaSchedaAvvisiTrenord()));

    m_intervalloAct = new QAction(QString::fromUtf8("Cambia intervallo aggiornamento..."), this);
    m_intervalloAct->setStatusTip(QString::fromUtf8("Cambia l'intervallo di aggiornamento della scheda corrente"));
    m_intervalloAct->setIcon(QIcon(":/img/cronometro.png"));
    connect(m_intervalloAct, SIGNAL(triggered()), this, SLOT(modificaIntervallo()));
    m_intervalloAct->setDisabled(true);

    m_avviaAct = new QAction(QString::fromUtf8("Avvia"), this);
    m_avviaAct->setStatusTip(QString::fromUtf8("Avvia/riprendi il monitoraggio con Viaggiatreno"));
    m_avviaAct->setIcon(QIcon(":/img/avvia.png"));
    connect(m_avviaAct, SIGNAL(triggered()), this, SLOT(avvia()));
    m_avviaAct->setDisabled(true);

    m_fermaAct = new QAction(QString::fromUtf8("Ferma"), this);
    m_fermaAct->setStatusTip(QString::fromUtf8("Interrompi il monitoraggio con ViaggiaTreno"));
    m_fermaAct->setIcon(QIcon(":/img/interrompi.png"));
    connect(m_fermaAct, SIGNAL(triggered()), this, SLOT(ferma()));
    m_fermaAct->setDisabled(true);

    m_fermaTutteAct = new QAction(QString::fromUtf8("Ferma tutte le schede"), this);
    m_fermaTutteAct->setStatusTip(QString::fromUtf8("Interrompi il monitoraggio con ViaggiaTreno per tutte le schede"));
    connect(m_fermaTutteAct, SIGNAL(triggered()), this, SLOT(fermaTutte()));

    m_avviaTutteAct = new QAction(QString::fromUtf8("Avvia tutte le schede"), this);
    m_avviaTutteAct->setStatusTip(QString::fromUtf8("Avvia/riprendi il monitoraggio con ViaggiaTreno per tutte le schede"));
    connect(m_avviaTutteAct, SIGNAL(triggered()), this, SLOT(avviaTutte()));

    m_riprovaComunicazioneConVTAct = new QAction(QString::fromUtf8("Ritenta comunicazione con &ViaggiaTreno"), this);
    m_riprovaComunicazioneConVTAct->setStatusTip(QString::fromUtf8("prova a verificare se la comunicazione con"
                                                                  "ViaggiaTreno ha ripreso a funzionare normalmente"));
    connect(m_riprovaComunicazioneConVTAct, SIGNAL(triggered()), m_downloadViaggiaTreno, SLOT(avvia()));
    m_riprovaComunicazioneConVTAct->setDisabled(true);

    m_aggiornaAct = new QAction(QString::fromUtf8("Aggiorna"), this);
    m_aggiornaAct-> setStatusTip(QString::fromUtf8("Aggiorna la scheda corrente"));
    m_aggiornaAct->setIcon(QIcon(":/img/aggiorna.png"));
    connect(m_aggiornaAct, SIGNAL(triggered()), this, SLOT(aggiorna()));
    m_aggiornaAct->setDisabled(true);

    m_configuraAct = new QAction(QString::fromUtf8("Configura QViaggiaTreno..."), this);
    m_configuraAct->setStatusTip(QString::fromUtf8("Mostra la finestra di configurazione di QViaggiaTreno"));
    m_configuraAct->setIcon(QIcon(":/img/configura.png"));
    connect(m_configuraAct, SIGNAL(triggered()), this, SLOT(configura()));

    m_stampaSchedaAct = new QAction(QString::fromUtf8("Stam&pa scheda corrente..."), this);
    m_stampaSchedaAct->setStatusTip(QString::fromUtf8("Stampa la scheda corrente"));
    m_stampaSchedaAct->setIcon(QIcon(":img/stampa.png"));
    connect(m_stampaSchedaAct, SIGNAL(triggered()), this, SLOT(stampaSchedaCorrente()));
    m_stampaSchedaAct->setDisabled(true);

    m_esportaSchedaAct = new QAction(QString::fromUtf8("Esporta sche&da corrente..."), this);
    m_esportaSchedaAct->setStatusTip(QString::fromUtf8("Esporta la scheda corrente in formato HTML o PDF"));
    m_esportaSchedaAct->setIcon(QIcon(":img/esporta.png"));
    connect(m_esportaSchedaAct, SIGNAL(triggered()), this, SLOT(esportaSchedaCorrente()));
    m_esportaSchedaAct->setDisabled(true);

    //crea azioni per i vari menu specifici per tipo di scheda
    m_cambiaNomeStazioneAct = new QAction(QString::fromUtf8("Cambia nome sta&zione..."), this);
    connect(m_cambiaNomeStazioneAct, SIGNAL(triggered()), this, SLOT(modificaNomeStazione()));

    m_apriAct = new QAction(QString::fromUtf8("Apri..."), this);
    m_apriAct->setStatusTip(QString::fromUtf8("Apre una lista di treni"));
    m_apriAct->setIcon(QIcon(":/img/apri.png"));
    connect(m_apriAct, SIGNAL(triggered()), this, SLOT(apri()));

    m_salvaAct = new QAction(QString::fromUtf8("Salva"), this);
    m_salvaAct->setStatusTip(QString::fromUtf8("Salva una lista di treni"));
    m_salvaAct->setIcon(QIcon(":/img/salva.png"));
    connect(m_salvaAct, SIGNAL(triggered()), this, SLOT(salva()));

    m_salvaConNomeAct = new QAction(QString::fromUtf8("Salva con nome..."), this);
    m_salvaConNomeAct->setStatusTip(QString::fromUtf8("Salva una lista di treni con un nuovo nome"));
    m_salvaConNomeAct->setIcon(QIcon(":/img/salva-come.png"));
    connect(m_salvaConNomeAct, SIGNAL(triggered()), this, SLOT(salvaConNome()));

    m_aggiungiTrenoAct = new QAction(QString::fromUtf8("Aggiungi treni alla lista..."), this);
    m_aggiungiTrenoAct->setStatusTip(QString::fromUtf8("Aggiungi uno o più treni alla lista dei treni da controllare"));
    m_aggiungiTrenoAct->setIcon(QIcon(":/img/aggiungi.png"));
    connect(m_aggiungiTrenoAct, SIGNAL(triggered()), this, SLOT(aggiungiTreni()));

    m_rimuoviTrenoAct = new QAction(QString::fromUtf8("Rimuovi treni dalla lista..."), this);
    m_rimuoviTrenoAct->setStatusTip(QString::fromUtf8("Rimuovi uno o più treni dalla lista dei treni da controllare"));
    m_rimuoviTrenoAct->setIcon(QIcon(":/img/rimuovi.png"));
    connect(m_rimuoviTrenoAct, SIGNAL(triggered()), this, SLOT(rimuoviTreni()));

    m_rimuoviTuttiAct = new QAction(QString::fromUtf8("Rimuovi tutti i treni dalla lista"), this);
    m_rimuoviTuttiAct->setStatusTip(QString::fromUtf8("Rimuovi tutti i treni dalla lista dei treni da controllare"));
    m_rimuoviTuttiAct->setIcon(QIcon(":/img/cancella-tutti.png"));
    connect(m_rimuoviTuttiAct, SIGNAL(triggered()), this, SLOT(rimuoviTuttiITreni()));

    m_impostaTitoloAct = new QAction(QString::fromUtf8("Imposta il titolo..."), this);
    m_impostaTitoloAct->setStatusTip(QString::fromUtf8("Imposta il titolo della lista di treni"));
    connect(m_impostaTitoloAct, SIGNAL(triggered()), this, SLOT(impostaTitolo()));

}

void QViaggiaTreno::creaMenu()
{
    m_viaggiatrenoMenu = menuBar()->addMenu("&QViaggiaTreno");
    m_viaggiatrenoMenu->addAction(m_nuovaStazioneAct);
    m_viaggiatrenoMenu->addAction(m_nuovoTrenoAct);
    m_viaggiatrenoMenu->addAction(m_nuovaListaTreniAct);
    m_viaggiatrenoMenu->addSeparator();
 //TODO rimossa temporaneamente voce scheda trenord ripristinare
 //   m_viaggiatrenoMenu->addAction(m_avvisiTrenordAct);
    m_viaggiatrenoMenu->addSeparator();
 //TODO: rimuovere commenti quando sarà pronto il codice per l'esportazione/stampa
 //   m_viaggiatrenoMenu->addAction(m_esportaSchedaAct);
 //   m_viaggiatrenoMenu->addAction(m_stampaSchedaAct);
 //   m_viaggiatrenoMenu->addSeparator();
    m_viaggiatrenoMenu->addAction(m_avviaAct);
    m_viaggiatrenoMenu->addAction(m_fermaAct);
    m_viaggiatrenoMenu->addAction(m_aggiornaAct);
    m_viaggiatrenoMenu->addAction(m_intervalloAct);
    m_viaggiatrenoMenu->addSeparator();
    m_viaggiatrenoMenu->addAction(m_avviaTutteAct);
    m_viaggiatrenoMenu->addAction(m_fermaTutteAct);
    m_viaggiatrenoMenu->addAction(m_riprovaComunicazioneConVTAct);
    m_viaggiatrenoMenu->addSeparator();
    m_viaggiatrenoMenu->addAction(m_exitAct);

    //crea menu specifici per ogni tipo di scheda
    // scheda stazione
    m_stazioneMenu = menuBar()->addMenu("Sta&zione");
    m_stazioneMenu->addAction(m_cambiaNomeStazioneAct);
    //scheda treno
    m_trenoMenu = menuBar()->addMenu("Tre&no");
    //scheda lista treni
    m_listaTreniMenu = menuBar()->addMenu("Lis&ta treni");
    m_listaTreniMenu->addAction(m_apriAct);
    m_listaTreniMenu->addAction(m_salvaAct);
    m_listaTreniMenu->addAction(m_salvaConNomeAct);
    m_listaTreniMenu->addSeparator();
    m_listaTreniMenu->addAction(m_aggiungiTrenoAct);
    m_listaTreniMenu->addAction(m_rimuoviTrenoAct);
    m_listaTreniMenu->addAction(m_rimuoviTuttiAct);
    m_listaTreniMenu->addSeparator();
    m_listaTreniMenu->addAction(m_impostaTitoloAct);

    //nascondi i menu specifici per ogni scheda
    m_stazioneMenu->menuAction()->setVisible(false);
    m_trenoMenu->menuAction()->setVisible(false);
    m_listaTreniMenu->menuAction()->setVisible(false);

    m_opzioniMenu = menuBar()->addMenu(QString::fromUtf8("&Strumenti"));
    m_opzioniMenu->addAction(m_configuraAct);

    m_helpMenu = menuBar()->addMenu("&Aiuto");
    m_helpMenu->addAction(m_licenseAct);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutAct);
    m_helpMenu->addAction(m_aboutQtAct);
}

void QViaggiaTreno::creaToolBar()
{
    m_mainToolBar = addToolBar("Barra strumenti principale");
    m_mainToolBar->addAction(m_nuovaStazioneAct);
    m_mainToolBar->addAction(m_nuovoTrenoAct);
    m_mainToolBar->addAction(m_nuovaListaTreniAct);
    //TODO rimossaa icona scheda trenod da toolbar ripristinare
   // m_mainToolBar->addAction(m_avvisiTrenordAct);
    m_mainToolBar->addSeparator();
 //TODO: rimuovere commenti quando sarà pronto il codice per l'esportazione/stampa
//    m_mainToolBar->addAction(m_stampaSchedaAct);
//    m_mainToolBar->addAction(m_esportaSchedaAct);
//    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_aggiornaAct);
    m_mainToolBar->addAction(m_avviaAct);
    m_mainToolBar->addAction(m_fermaAct);
    m_spinIntervallo = new QSpinBox(this);
    m_spinIntervallo->setRange(1, 1440);
    m_spinIntervallo->setSuffix(" min");
    m_spinIntervalloAct = m_mainToolBar->addWidget(m_spinIntervallo);
    m_spinIntervalloAct->setDisabled(true);

    //connessioni
    connect(m_spinIntervallo, SIGNAL(valueChanged(int)), this, SLOT(modificaIntervalloConSpinBox(int)));

    m_listaToolbar = addToolBar("Barra strumenti lista treni");
    m_listaToolbar->addAction(m_apriAct);
    m_listaToolbar->addAction(m_salvaAct);
    m_listaToolbar->addAction(m_salvaConNomeAct);
    m_listaToolbar->addSeparator();
    m_listaToolbar->addAction(m_aggiungiTrenoAct);
    m_listaToolbar->addAction(m_rimuoviTrenoAct);
    m_listaToolbar->addAction(m_rimuoviTuttiAct);

    m_listaToolbar->setVisible(false);
}


void QViaggiaTreno::creaWidgetCentrale()
{
    m_schede = new QTabWidget(this);

    //rende le schede muovibili
    m_schede->setMovable(true);

    //aggiunge pulsante di chiusura della scheda corrente
    m_schede->setTabsClosable(true);

    setCentralWidget(m_schede);

    //connessioni
    connect(m_schede, SIGNAL(currentChanged(int)), this, SLOT(schedaCambiata(int)));
    connect(m_schede, SIGNAL(tabCloseRequested(int)), this, SLOT(chiudiScheda(int)));
}


void QViaggiaTreno::creaStatusBar()
{
    //crea un indicatore a LED per visualizzare lo stato della connessione a viaggiatreno
    m_ledStatoViaggiaTreno = new QLedIndicator(this);

    statusBar()->addPermanentWidget(m_ledStatoViaggiaTreno);

    statusBar()->showMessage("Pronto");
}

void QViaggiaTreno::leggiImpostazioniFinestra()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "fra74", "QViaggiaTreno");

    //legge impostazioni su posizione e dimensione finestra
    settings.beginGroup("Finestra principale");
    resize(settings.value("dimensione", QSize(400, 400)).toSize());
    move(settings.value("posizione", QPoint(200, 200)).toPoint());
    settings.endGroup();

    //legge lista delle ultime stazioni monitorate
    settings.beginGroup("Stazioni monitorate");
    maxStazioni = settings.value("max", 20).toInt();
    m_listaStazioniMonitorate = settings.value("Lista").toStringList();
    settings.endGroup();

}

//ripristina le schede aperte nella sessione precedente di QviaggiaTreno
void QViaggiaTreno::ripristinaSchede()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "fra74", "QViaggiaTreno");
    //recupera il numero di schede da ripristinare
    settings.beginGroup("Schede aperte");
    int numero = settings.value("numero", 0).toInt();
    //recupera l'indice della scheda corrente al momento della chiusura
    int schedaCorrente = settings.value("scheda corrente", -1).toInt();
    settings.endGroup();

    for (int i = 1; i <= numero; i++)
    {
        settings.beginGroup(QString("Scheda %1").arg(i));
        //recupera il tipo di scheda
        QString tipo = settings.value("tipo scheda").toString();

        if (tipo == "treno")
        {
            //recupera numero treno e intervallo di aggiornamento
            QString numero = settings.value("numero treno").toString();
            int intervallo = settings.value("intervallo").toInt();
            QString codice = settings.value("codice stazione origine").toString();
            //crea la scheda
            nuovoTreno(numero, codice, intervallo);
        }

        if (tipo == "stazione")
        {
            //recupera intervallo di aggiornamento e nome stazione
            int intervallo = settings.value("intervallo").toInt();
            QString stazione = settings.value("nome stazione").toString();
            QString codice = settings.value("codice stazione").toString();

            //crea la scheda ma NON la avvia
            SchedaStazione *scheda = nuovaStazione(stazione, false, intervallo, true);
            if (codice != "")
                scheda->cambiaCodiceStazione(codice);
            else
            {
                scheda->aggiorna();
                scheda->avvia();
                scheda->ripristinaFiltri(settings);
            }
        }

        if (tipo == "lista treni")
        {
            //recupera nome file
            QString nomefile = settings.value("nome file").toString();

            SchedaListaTreni * scheda = nuovaListaTreni();

            if (nomefile != "")
                scheda->apriFile(nomefile);
        }
        settings.endGroup();
    }
    //seleziona come scheda corrente la scheda che era selezionata al momento della chiusura
    m_schede->setCurrentIndex(schedaCorrente);
}

void QViaggiaTreno::salvaSchede()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "fra74", "QViaggiaTreno");

    //rimuove i gruppi relativi alle singole schede aperte in sessioni precedenti
    settings.beginGroup("Schede aperte");
    int schede = settings.value("numero", 0).toInt();
    settings.endGroup();

    for (int i = 1; i <= schede; i++)
    {
        settings.beginGroup(QString("Scheda %1").arg(i));
        settings.remove("");
        settings.endGroup();
    }

    //scrive numero di schede aperte
    settings.beginGroup("Schede aperte");
    schede = m_listaSchede.count();
    settings.setValue("numero", schede);
    //scrive il numero della scheda attualmente selezionata
    settings.setValue("scheda corrente", m_schede->currentIndex());
     settings.endGroup();


    //scrive informazioni sulle schede aperte
    for (int i = 0; i < m_schede->count(); i++)
    {
        settings.beginGroup(QString("Scheda %1").arg(i+1));
        SchedaQViaggiaTreno* scheda = qobject_cast<SchedaQViaggiaTreno*>
                (m_schede->widget(i));
        scheda->salvaScheda(settings);
        settings.endGroup();
    }

}


void QViaggiaTreno::scriviImpostazioniFinestra()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "fra74", "QViaggiaTreno");

    //scrive impostazioni su posizione e dimensione finestra
    settings.beginGroup("Finestra principale");
    settings.setValue("dimensione", size());
    settings.setValue("posizione", pos());
    settings.endGroup();

    //scrive lista delle ultime stazioni monitorate
    settings.beginGroup("Stazioni monitorate");
    settings.setValue("max", maxStazioni);
    settings.setValue("Lista", m_listaStazioniMonitorate);
    settings.endGroup();

}

//questo metodo imposta il proxy utilizzato per tutte le connessioni alla rete. viene chiamato all'avvio del programma ed ogni volkta che si modifica la configurazione
void QViaggiaTreno::impostaProxy()
{
    //se è stato selezionato utilizza il proxy di sistema
    if (configurazione().proxyDiSistemaUtilizzato())
            QNetworkProxyFactory::setUseSystemConfiguration(true);
    else
    {
        QNetworkProxy networkProxy;
            if (configurazione().proxyUtilizzato())
            {
                networkProxy.setType(QNetworkProxy::HttpProxy);
                networkProxy.setHostName(configurazione().hostProxy());
                networkProxy.setPort(configurazione().portaProxy());

                if (configurazione().proxyRichiedeAutenticazione())
                {
                    networkProxy.setUser(configurazione().nomeUtenteProxy());
                    networkProxy.setPassword(configurazione().passwordProxy());
                }
            }
            else
                networkProxy.setType(QNetworkProxy::NoProxy);
            QNetworkProxy::setApplicationProxy(networkProxy);
    }
}

QViaggiaTreno::~QViaggiaTreno()
{

}

//restituisce un puntatore alla scheda corrente (se esiste)
SchedaQViaggiaTreno * QViaggiaTreno::schedaCorrente()
{
    //non ci sono schede restituisce un puntatore nullo
    if (!m_schede->count())
        return 0L;
    else
        return m_listaSchede.value(qobject_cast<SchedaQViaggiaTreno*>(m_schede->currentWidget())->idScheda());
}

SchedaQViaggiaTreno* QViaggiaTreno::scheda(quint32 id)
{
    //non ci sono schede aperte, restituisci un puntatore nullo
    if (!m_schede->count())
        return 0L;

    //non esiste una scheda con questo id
    if (!m_listaSchede.contains(id))
        return 0L;
    else
        return m_listaSchede[id];
}

//eventi
void QViaggiaTreno::closeEvent(QCloseEvent *event)
{
    salvaSchede();
    scriviImpostazioniFinestra();
    //per il momento accettare senza condizioni l'evento di chiusura,
    //lo scopo è solo quello di richiamare le funzioni di salvataggio delle impostazioni
    event->accept();
}


//restituisce true se la scheda è ancora aperta
bool QViaggiaTreno::schedaAperta(SchedaQViaggiaTreno *scheda)
{
    //cerca l'indice della scheda nel tab widget
    //se è diverso da -1 allora la scheda esiste
    return (m_schede->indexOf(scheda) != -1);
}
