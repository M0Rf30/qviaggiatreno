/***************************************************************************
 *   Copyright (C) 2008-2012 by fra74                                           *
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


#include <QtWidgets>

#include "dialogo_nomestazione.h"
#include "dialogo_configurazione.h"
#include "schedalistatreni.h"
#include "schedastazione.h"
#include "schedatreno.h"
#include "schedaavvisitrenord.h"
#include "qviaggiatreno.h"
#include "utils.h"
#include "qledindicator.h"

// slot
void QViaggiaTreno::about()
{
    QMessageBox::about(this, QString::fromUtf8("Informazioni su QViaggiaTreno"),
      QString::fromUtf8("QViaggiaTreno versione %1\n\n(C) 2008-2012 fra74\n\n%2").arg(s_versione)
      .arg(QString::fromUtf8("QViaggiaTreno è un'interfaccia a ViaggiaTreno, il servizio web di Trenitalia per monitorare la marcia dei treni passeggeri\n\n"
                             "Questo programma è distribuito secondo i termini della licenza GNU GPL v.2"
                             "\n\nQViaggiaTreno utilizza il widget 'QLedIndicator' scritto da 'Tn'")));
}

void QViaggiaTreno::visualizzaLicenza()
{
    if (!m_licenseDialog)
        {
            //costruzione della finestra di dialogo solo se non è già stata costruita precedentemente
            m_licenseDialog = new QDialog(this);
            m_licenseDialog->setWindowTitle(QString::fromUtf8("Testo licenza GNU GPL v.2"));
            QVBoxLayout *vbox = new QVBoxLayout;
            QTextEdit *textEdit = new QTextEdit(m_licenseDialog);
            //imposta una larghezza minima per leggere decentemente il testo della licenza
            //senza eccessivo a capo
            textEdit->setMinimumWidth(500);
            vbox->addWidget(textEdit);
            QPushButton *button = new QPushButton(QString::fromUtf8("Ok"), this);
            vbox->addWidget(button, 0, Qt::AlignCenter);
            m_licenseDialog->setLayout(vbox);
            connect(button, SIGNAL(clicked()), m_licenseDialog, SLOT(accept()));

            //lettura del file di testo contenente il testo della licenza GPL
            textEdit->setReadOnly(true);
            textEdit->setCursorWidth(0); //cursore non visibile
            QString linea;
            QFile testoLicenza(":/doc/GPL-2.txt");
            testoLicenza.open(QFile::ReadOnly);
            QString txt = testoLicenza.readAll();
            textEdit->insertPlainText(txt);
            //riporta il cursore all'inizio del documento
            textEdit->moveCursor(QTextCursor::Start);
        }
    m_licenseDialog->show();
}

//avvia il monitoraggio nella scheda corrente
void QViaggiaTreno::avvia()
{
    schedaCorrente()->avvia();

    //attiva/disattiva le azioni coerentemente con il fatto che nella scheda corrente ora il monitoraggio è attivo
    m_avviaAct->setDisabled(true);
    m_fermaAct->setEnabled(true);

    statusBar()->showMessage(QString::fromUtf8("Monitoraggio avviato nella scheda '%1'").arg(schedaCorrente()->titolo()), 3000);
}

void QViaggiaTreno::ferma()
{
    schedaCorrente()->ferma();

    //attiva/disattiva le azioni coerentemente con il fatto che nella scheda corrente ora il monitoraggio è attivo
    m_avviaAct->setEnabled(true);
    m_fermaAct->setDisabled(true);


    statusBar()->showMessage(QString::fromUtf8("Monitoraggio fermato nella scheda '%1'").arg(schedaCorrente()->titolo()), 3000);
}

//slot
//crea una nuova scheda per il monitoraggio di un treno
void QViaggiaTreno::nuovoTreno()
{
    bool ok;

    int num = QInputDialog::getInt(this, QString::fromUtf8("Monitoraggio treno"), QString::fromUtf8("Inserire il numero del treno da monitorare con ViaggiaTreno"), 0, 1, 99999, 1, &ok);
    if (ok)
    {
        //converte il numero del treno in una stringa
        QString numero;
        numero.setNum(num);
        nuovoTreno(numero);
    }
}

void QViaggiaTreno::nuovaStazione()
{
    //ordina la lista delle stazioni già monitorate
    QStringList listaOrdinata = m_listaStazioniMonitorate;
    listaOrdinata.sort();

    DialogoNomeStazione *dialogo = new DialogoNomeStazione(this, listaOrdinata);
   //	QString stazione = QInputDialog::getItem(this, QString::fromUtf8("Monitoraggio stazione"), QString::fromUtf8("Inserire il nome della stazione da monitorare con ViaggiaTreno:"), listaOrdinata, -1, true, &ok);
    if (dialogo->exec())
        {
        QString stazione = dialogo->nomeStazione();

            //verifica che non ci sia già una scheda aperta per questa stazione
            //se esiste allora la relativa scheda diventerà la scheda corrente
            QMapIterator<quint32, SchedaQViaggiaTreno*> it(m_listaSchede);
            while (it.hasNext())
                {
                    it.next();

                    //la scheda è una scheda di stazione?
                    if (it.value()->tipoScheda() == SchedaQViaggiaTreno::tsStazione)
                        {
                            SchedaStazione *scheda = qobject_cast<SchedaStazione*>(it.value());
                            if (!QString::compare(stazione, scheda->nomeStazione(), Qt::CaseInsensitive))
                            {
                                m_schede->setCurrentWidget(scheda);
                                return;
                            }
                        }

                }

            //non ci sono schede già aperte, aprine una nuova
            nuovaStazione(stazione, !(dialogo->cercaTutteStazioni()));
        }

    delete dialogo;
}

SchedaTreno* QViaggiaTreno::nuovoTreno(const QString& treno, int intervallo)
{
    //verifica che non esista già una scheda aperta per questo treno
    QMapIterator<quint32, SchedaQViaggiaTreno*> it(m_listaSchede);
    while (it.hasNext())
    {
        it.next();
        if (it.value()->tipoScheda() == SchedaQViaggiaTreno::tsTreno)
        {
            SchedaTreno* scheda = qobject_cast<SchedaTreno*>(it.value());
            // c'è un problema
            //supponiamo di avere un treno con numero ambiguo (esempio 518)
            //apro la scheda per il 518 su rete RFI
            //ma se poi controllo solo il numero treno allora non potrò mai aprire una scheda
            //per quello su rete FNM
            //workaround: controllo che il codice origine sia nullo, in tal caso impedisco
            //l'apertura di una nuova scheda

            if (!QString::compare(treno, scheda->numero()) && (scheda->codiceOrigine().isEmpty()))
            {
                //esiste già una scheda con questo nome, selezionala ed esci
                m_schede->setCurrentWidget(scheda);
                return 0L;
            }
        }
    }

    //crea la scheda della stazione
    SchedaTreno *scheda = new SchedaTreno(this, treno);

    //aggiorna la lista delle schede e degli item
    m_listaSchede.insert(scheda->idScheda(), scheda);

    //cambia intervallo di aggiornamento se intervallo != 0
    if (intervallo)
        scheda->impostaIntervallo(intervallo);

    //aggiorna la scheda e avvia il timer
    scheda->aggiorna();
    scheda->avvia();

    //aggiunge la scheda al tabwidget assicurandosi che diventi la scheda corrente
    m_schede->addTab(scheda, QIcon(":/img/treno.png"), scheda->titolo());
    m_schede->setCurrentWidget(scheda);

    return scheda;
}

//crea una nuova scheda Treno specificando anche il codice di origine
SchedaTreno* QViaggiaTreno::nuovoTreno(const QString& treno, const QString &codiceOrigine, int intervallo)
{
    //verifica che non esista già una scheda aperta per questo treno
    QMapIterator<quint32, SchedaQViaggiaTreno*> it(m_listaSchede);
    while (it.hasNext())
    {
        it.next();
        if (it.value()->tipoScheda() == SchedaQViaggiaTreno::tsTreno)
        {
            SchedaTreno* scheda = qobject_cast<SchedaTreno*>(it.value());
            if (!QString::compare(treno, scheda->numero()) && !QString::compare(codiceOrigine, scheda->codiceOrigine()))
            {
                //esiste già una scheda con questo nome, selezionala ed esci
                m_schede->setCurrentWidget(scheda);
                return 0L;
            }
        }
    }

    //crea la scheda della stazione
    SchedaTreno *scheda = new SchedaTreno(this, treno);

    //aggiorna la lista delle schede
    m_listaSchede.insert(scheda->idScheda(), scheda);

    //cambia intervallo di aggiornamento se intervallo != 0
    if (intervallo)
        scheda->impostaIntervallo(intervallo);

    // il metodo cambiaCodiceOrigine automaticamente avvia la scheda
    scheda->cambiaCodiceOrigine(codiceOrigine);
    //aggiorna la scheda e avvia il timer
    //scheda->aggiorna();
    //scheda->avvia();

    //aggiunge la scheda al tabwidget assicurandosi che diventi la scheda corrente
    m_schede->addTab(scheda, QIcon(":/img/treno.png"), scheda->titolo());
    m_schede->setCurrentWidget(scheda);

    return scheda;
}

SchedaStazione* QViaggiaTreno::nuovaStazione(const QString& stazione, bool nomeEsatto, int intervallo, bool nonAvviare)
{
    //verifica che non esista già una scheda aperta per questa stazione
    QMapIterator<quint32, SchedaQViaggiaTreno*> it(m_listaSchede);
    while (it.hasNext())
    {
        it.next();
        if (it.value()->tipoScheda() == SchedaQViaggiaTreno::tsStazione)
        {
            SchedaStazione* scheda = qobject_cast<SchedaStazione*>(it.value());
            if (!QString::compare(stazione, scheda->nomeStazione()))
            {
                //esiste già una scheda con questo nome, selezionala ed esci
                m_schede->setCurrentWidget(scheda);
                return 0L;
            }
        }
    }

    //verifica che la stazione non sia già nella lista delle stazioni monitorate
    if (!m_listaStazioniMonitorate.contains(stazione, Qt::CaseInsensitive))
        {
        //aggiunge la stazione alla lista delle stazioni monitorate, assicurandosi di non superare
        //il massimo numero di stazioni previste
        if (m_listaStazioniMonitorate.size() < maxStazioni)
            m_listaStazioniMonitorate.append(stazione);
        else
            {
                m_listaStazioniMonitorate.removeFirst();
                m_listaStazioniMonitorate.append(stazione);
            }
        }

    //se nel nome è presente un accento, sostituiscilo con l'entità
    QString temp = stazione;
    temp.replace("'", "&#039;");
    //crea la scheda della stazione
    SchedaStazione *scheda = new SchedaStazione(this, temp, nomeEsatto);


    //aggiorna la lista delle schede e degli item
    m_listaSchede.insert(scheda->idScheda(), scheda);

    //cambia intervallo di aggiornamento se intervallo != 0
    if (intervallo)
        scheda->impostaIntervallo(intervallo);

    //aggiorna la scheda e avvia il timer
    if (!nonAvviare)
    {
        scheda->aggiorna();
        scheda->avvia();
    }

    //aggiunge la scheda al tabwidget assicurandosi che diventi la scheda corrente
    m_schede->addTab(scheda, QIcon(":/img/stazione.png"), scheda->titolo());
    m_schede->setCurrentWidget(scheda);

    return scheda;

}

SchedaListaTreni* QViaggiaTreno::nuovaListaTreni()
{
    //crea la scheda per la nuova lista dei treni
    SchedaListaTreni* scheda = new SchedaListaTreni(this);

    //aggiorna la lista delle schede
    m_listaSchede.insert(scheda->idScheda(), scheda);


    //aggiorna la scheda e avvia il timer
    scheda->aggiorna();
    scheda->avvia();

    //aggiunge la scheda al tabwidget assicurandosi che diventi la scheda corrente
    m_schede->addTab(scheda, QIcon(":/img/listatreni.png"), scheda->titolo());
    m_schede->setCurrentWidget(scheda);

    //sincronizza la GUI con la scheda
    sincronizzaGUI(scheda->idScheda());

    return scheda;
}

SchedaAvvisiTrenord* QViaggiaTreno::nuovaSchedaAvvisiTrenord()
{
    //crea la scheda
    SchedaAvvisiTrenord* scheda = new SchedaAvvisiTrenord(this);

    //aggiorna la lista delle schede e aggiunge la scheda al tabwidget,
    //assicurandosi che diventi la scheda corrente
    m_listaSchede.insert(scheda->idScheda(), scheda);
    m_schede->addTab(scheda, QIcon(":img/trenord.png"), scheda->titolo());
    m_schede->setCurrentWidget(scheda);

    //avvia la scheda ed aggiorna il timer
    scheda->aggiorna();
    scheda->avvia();

    sincronizzaGUI(scheda->idScheda());

    //disattiva la voce di menu per creare scheda avvisi trenord visto che non ha senso averne più di una
    m_avvisiTrenordAct->setDisabled(true);

    return scheda;
}

void QViaggiaTreno::chiudiScheda(int id)
{
    //ottiene un puntatore alla scheda corrente
    SchedaQViaggiaTreno *scheda = qobject_cast<SchedaQViaggiaTreno*>(m_schede->widget(id));
    if (scheda)
        {
            //rimuove la scheda dal tab widget
            m_schede->removeTab(id);


            //elimina scheda dalle rispettiva lista
            m_listaSchede.remove(scheda->idScheda());

            //verifica se la scheda era la scheda con gli avvisi Trenord
            //ed in caso positivo riabilita la corrispondente voce di menu
            if (scheda->tipoScheda() == SchedaQViaggiaTreno::tsAvvisiTrenord)
                m_avvisiTrenordAct->setEnabled(true);

            //infine cancella scheda e item
            delete scheda;
        }
}


//questo slot viene richiamato quando cambia la scheda selezionata
//questo può succedere quando viene chiusa la scheda corrente,
//o quando ne viene selezionata un'altra cliccando sulla rispettiva linguetta
void QViaggiaTreno::schedaCambiata(int indice)
{
    //nasconde menu specifico per schede
    m_trenoMenu->menuAction()->setVisible(false);
    m_stazioneMenu->menuAction()->setVisible(false);
    m_listaTreniMenu->menuAction()->setVisible(false);

    //nascondi anche la Toolbar
    m_listaToolbar->setVisible(false);

    if (indice == -1)
        //non ci sono più schede, disattiva tutti i controlli
        {
            m_avviaAct->setDisabled(true);
            m_avviaTutteAct->setDisabled(true);
            m_fermaAct->setDisabled(true);
            m_fermaTutteAct->setDisabled(true);
            m_aggiornaAct->setDisabled(true);
            m_intervalloAct->setDisabled(true);
            m_spinIntervalloAct->setDisabled(true);
            m_esportaSchedaAct->setDisabled(true);
            m_stampaSchedaAct->setDisabled(true);

            setWindowTitle(QString::fromUtf8("QViaggiaTreno"));

            return;
        }


    //ottiene un puntatore alla scheda corrente
    SchedaQViaggiaTreno *scheda = qobject_cast<SchedaQViaggiaTreno*>(m_schede->currentWidget());

    //cambia titolo della finestra di QViaggiaTreno
    setWindowTitle(QString::fromUtf8("QViaggiaTreno - %1").arg(scheda->titolo()));

    //attiva/disattiva le azioni che non dipendono dallo stato della scheda
    m_avviaTutteAct->setEnabled(true);
    m_fermaTutteAct->setEnabled(true);
    m_aggiornaAct->setEnabled(true);
    m_intervalloAct->setEnabled(true);
    m_spinIntervalloAct->setEnabled(true);
    m_spinIntervallo->setValue(scheda->intervallo());

    //attiva/disattiva le azioni che dipendono dallo stato della scheda
    if (scheda->fermata())
    {
        m_avviaAct->setEnabled(true);
        m_fermaAct->setEnabled(false);

    }
    else
    {
        m_avviaAct->setEnabled(false);
        m_fermaAct->setEnabled(true);
    }
    m_esportaSchedaAct->setEnabled(scheda->esportabile());
    m_stampaSchedaAct->setEnabled(scheda->stampabile());

    //attiva/disattiva azioni o menu a seconda del tipo di scheda e dello stato della scheda
    switch(scheda->tipoScheda())
    {
        //non visualizzare il menu a tendina treno perché attualmente è senza voci
        case SchedaQViaggiaTreno::tsTreno: m_trenoMenu->menuAction()->setVisible(false); break;
        case SchedaQViaggiaTreno::tsStazione: m_stazioneMenu->menuAction()->setVisible(true); break;
        case SchedaQViaggiaTreno::tsListaTreni:
            {
                m_listaTreniMenu->menuAction()->setVisible(true);

                m_listaToolbar->setVisible(true);
            }; break;
    case SchedaQViaggiaTreno::tsAvvisiTrenord: break;
    }

}

//Questo slot viene chiamato se l'utente ha scelto di modificare l'intervallo di aggiornamento
//attraverso la corrispondente voce di menu
void QViaggiaTreno::modificaIntervallo()
{
    bool Ok;
    SchedaQViaggiaTreno *scheda = schedaCorrente();
    int intervallo = QInputDialog::getInt(this, QString::fromUtf8("Modifica intervallo di aggiornamento"), QString::fromUtf8("Inserire il nuovo intervallo di aggiornamento in minuti:"), scheda->intervallo(), 1, 1440, 1, &Ok);
    if (Ok)
        {
            scheda->impostaIntervallo(intervallo);
            m_spinIntervallo->setValue(intervallo);
        }

}

//Questo slot viene richiamato quando si modifica l'intervallo di aggiornamento attraverso la spin box
void QViaggiaTreno::modificaIntervalloConSpinBox(int valore)
{
    schedaCorrente()->impostaIntervallo(valore);
}



//questo slot viene richiamato quando lo stato di una scheda cambia
void QViaggiaTreno::aggiornaStatoScheda(quint32 idScheda)
{
    SchedaQViaggiaTreno *scheda = m_listaSchede.value(idScheda);

    //attiva/disattiva pulsanti per fermare/riavviare il timer a seconda dello stato della scheda...
    m_avviaAct->setEnabled(scheda->fermata());
    m_fermaAct->setDisabled(scheda->fermata());

}

//la scheda ha emesso un segnale per avvisare che la GUI potrebbe non essere più sincronizzata
//con lo stato della scheda. Questo slot si occupa di risincronizzare lo stato della
//GUI con quello della scheda
void QViaggiaTreno::sincronizzaGUI(quint32 idScheda)
{
    //ottieni puntatore alla scheda
    SchedaQViaggiaTreno *scheda = m_listaSchede.value(idScheda);

    switch(scheda->tipoScheda())
    {
        case SchedaQViaggiaTreno::tsListaTreni:
        {
            SchedaListaTreni* listaTreni = qobject_cast<SchedaListaTreni*>(scheda);
            //se la lista non è modificata disattiva l'azione per salvare
            if (!listaTreni->modificata())
                m_salvaAct->setDisabled(true);
            else
                m_salvaAct->setEnabled(true);

            //disattiva le azioni per rimuovere uno o più treni se la lista dei treni è vuota
            if (listaTreni->numeroTreni()== 0)
            {
                m_rimuoviTrenoAct->setDisabled(true);
                m_rimuoviTuttiAct->setDisabled(true);
            }
            else
            {
                m_rimuoviTrenoAct->setEnabled(true);
                m_rimuoviTuttiAct->setEnabled(true);
            }

        }; break;

        default: ; //per le altre tipologie di schede non c'è bisogno di far niente
    }
}

//slot
//mostra un messaggio nella status bar
void QViaggiaTreno::mostraMessaggioStatusBar(const QString& msg)
{
    statusBar()->showMessage(msg);
}


//slot
//mostra la finestra di dialogo di configurazione
void QViaggiaTreno::configura()
{
    DialogoConfigurazione *dlg = new DialogoConfigurazione(this);
    dlg->impostaConfigurazione();
    if (dlg->exec())
    {
        dlg->applicaConfigurazione();

        //reimposta il proxy
        impostaProxy();
    }

    delete dlg;
}

//aggiorna la scheda corrente
void QViaggiaTreno::aggiorna()
{
    schedaCorrente()->aggiorna();
}


//ferma tutte le schede
void QViaggiaTreno::fermaTutte()
{
    SchedaQViaggiaTreno* scheda;
    foreach(scheda, m_listaSchede)
        scheda->ferma();
}

void QViaggiaTreno::avviaTutte()
{
    SchedaQViaggiaTreno* scheda;
    foreach(scheda, m_listaSchede)
        scheda->avvia();
}

//Questo slot viene richiamato durante il controllo periodico del corretto funzionamento di viaggiatreno
void QViaggiaTreno::statoViaggiaTrenoCambiato(bool stato)
{
    //attiva/disattiva la voce del menu a seconda dello stato della comunicazione con VT
    m_riprovaComunicazioneConVTAct->setDisabled(stato);

    //cambia lo stato del LED
    m_ledStatoViaggiaTreno->setChecked(stato);

    //cambia il tooltip
    if (stato)
        m_ledStatoViaggiaTreno->setToolTip(QString::fromUtf8("ViaggiaTreno funziona normalmente"));
    else
        m_ledStatoViaggiaTreno->setToolTip(QString::fromUtf8("ViaggiaTreno al momento non sembra funzionare"));
}

//stampa la scheda corrente richiamandone il rispettivo metodo
void QViaggiaTreno::stampaSchedaCorrente()
{
    schedaCorrente()->stampa();
}

//esporta la scheda corrente chiamandone il rispettivo metodo
void QViaggiaTreno::esportaSchedaCorrente()
{
    schedaCorrente()->esporta();
}
