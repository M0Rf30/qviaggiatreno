/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74   *
 *   francesco.b74@gmail.com   *
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


#ifndef QVIAGGIATRENO_H
#define QVIAGGIATRENO_H

#include <QtWidgets>
#include <QtNetwork>
#include "config.h"

class SchedaStazione;
class SchedaTreno;
class SchedaListaTreni;
class SchedaAvvisiTrenord;
class SchedaQViaggiaTreno;
class DownloadViaggiaTreno;
class DownloadTrenord;
class QLedIndicator;


class QViaggiaTreno:public QMainWindow
{
        Q_OBJECT

public:
    QViaggiaTreno();
    ~QViaggiaTreno();

    //restituisce un puntatore al QNetworkAccessManager che è globale per l'applicazione
    QNetworkAccessManager* networkAccessManager() {return m_nam;}

    //restituisce un puntatore alla classe per il download da viaggiatreno
    DownloadViaggiaTreno * downloadViaggiaTreno() {return m_downloadViaggiaTreno;}

    //restituisce un puntatore alla classe per il download dal sito Trenord
    DownloadTrenord* downloadTrenord() {return m_downloadTrenord;}

    //restituisce un puntatore ad una scheda dato il suo id
    SchedaQViaggiaTreno* scheda(quint32 id);

    //restituisce true se il puntatore punta ad una scheda ancora aperta
    bool schedaAperta(SchedaQViaggiaTreno* scheda);


    Configurazione& configurazione() {return m_configurazione;}

protected:

private slots:
    void about();
    void visualizzaLicenza();
    void nuovaStazione();
    SchedaStazione* nuovaStazione(const QString& stazione, bool nomeEsatto = false, int intervallo = 0, bool nonAvviare = false);
    SchedaListaTreni* nuovaListaTreni();
    SchedaAvvisiTrenord* nuovaSchedaAvvisiTrenord();
    void nuovoTreno();
    SchedaTreno* nuovoTreno(const QString& numeroTreno, int intervallo = 0);
    SchedaTreno* nuovoTreno(const QString& numeroTreno, const QString& codiceOrigine, int intervallo = 0);
    void chiudiScheda(int id);
    void avvia();
    void avviaTutte();
    void ferma();
    void fermaTutte();
    void aggiorna();
    void configura();
    void schedaCambiata(int indice);
    void modificaIntervallo();
    void modificaIntervalloConSpinBox(int intervallo);
    void aggiornaStatoScheda(quint32 idScheda);
    void aggiornaNomeScheda(quint32 idScheda);
    void sincronizzaGUI(quint32 idScheda);
    void mostraMessaggioStatusBar(const QString& msg);
    void stampaSchedaCorrente();
    void esportaSchedaCorrente();


    //azioni per menu/toolbar specifici per tipo di scheda
    //implementazione in qviaggiatrenoslots_schede.cpp
    void modificaNomeStazione();
    void apri();
    void salva();
    void salvaConNome();
    void aggiungiTreni();
    void rimuoviTreni();
    void rimuoviTuttiITreni();
    void impostaTitolo();
    void statoViaggiaTrenoCambiato(bool);

private:
    void creaWidgetCentrale();
    void creaAzioni();
    void creaMenu();
    void creaToolBar();
    void creaStatusBar();
    void leggiImpostazioniFinestra();
    void ripristinaSchede();
    void salvaSchede();
    void scriviImpostazioniFinestra();
    SchedaQViaggiaTreno * schedaCorrente();
    void impostaProxy();

    void closeEvent(QCloseEvent *event);

    QMenu *m_viaggiatrenoMenu, *m_opzioniMenu, *m_helpMenu;
    QMenu *m_stazioneMenu, *m_trenoMenu, *m_listaTreniMenu;
    QToolBar *m_mainToolBar, *m_listaToolbar;
    QAction *m_exitAct, *m_nuovaStazioneAct, *m_nuovoTrenoAct, *m_nuovaListaTreniAct;
    QAction *m_avvisiTrenordAct;
    QAction *m_configuraAct;
    QAction *m_aboutAct, *m_aboutQtAct, *m_licenseAct;
    QAction *m_fermaAct, *m_avviaAct, *m_fermaTutteAct, *m_avviaTutteAct;
    QAction *m_aggiornaAct, *m_intervalloAct, *m_aggiungiTrenoAct;
    QAction *m_riprovaComunicazioneConVTAct;
    QAction *m_rimuoviTrenoAct, *m_rimuoviTuttiAct, *m_impostaTitoloAct;
    QAction *m_spinIntervalloAct;
    QAction *m_cambiaNomeStazioneAct, *m_apriAct, *m_salvaAct, *m_salvaConNomeAct;
    QAction *m_stampaSchedaAct, *m_esportaSchedaAct;
    QTabWidget *m_schede;
    QSpinBox *m_spinIntervallo;
    QDialog *m_licenseDialog;
    QLedIndicator *m_ledStatoViaggiaTreno;

    int maxStazioni;
    QStringList m_listaStazioniMonitorate;

    QMap<quint32, SchedaQViaggiaTreno*> m_listaSchede;

    DownloadViaggiaTreno* m_downloadViaggiaTreno;
    DownloadTrenord* m_downloadTrenord;
    QNetworkAccessManager* m_nam;

    //impostazioni
    Configurazione m_configurazione;

    //numero di versione
    static QString s_versione;
};

#endif
