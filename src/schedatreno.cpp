/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74                                      *
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

#include <QtNetwork>
#include "schedatreno.h"
#include "qviaggiatreno.h"
#include "download_viaggiatreno.h"

#include "utils.h"

SchedaTreno::SchedaTreno(QViaggiaTreno *parent, const QString &numero, const unsigned int intervalloStandard) :
        SchedaQViaggiaTreno(parent, tsTreno, intervalloStandard)
{
    m_numero = numero;
    m_codiceStazioneOrigine = "";
    m_idTabelle = m_idNumeroSbagliato = m_idNumeroAmbiguo = -1;
    m_riepilogoScaricato = false;

    //crea il widget con le tabelle
    m_widget = new WidgetDatiTreno(this);
    m_idTabelle = addWidget(m_widget);
    setCurrentIndex(m_idTabelle);

    //imposta il parser
    m_parser = new ParserTrenoViaggiaTreno(this);

    m_orarioTreno.inizializza();
    m_widget->inizializza();
    m_widget->impostaNumeroTreno(numero);

    //connessioni
    connect(this, SIGNAL(statoCambiato(quint32)), parent, SLOT(aggiornaStatoScheda(quint32)));
    connect(this, SIGNAL(nomeSchedaCambiato(quint32)), parent, SLOT(aggiornaNomeScheda(quint32)));
    connect(this, SIGNAL(apriSchedaStazione(const QString&, bool)), parent, SLOT(nuovaStazione(const QString&, bool)));
    connect(this, SIGNAL(messaggioStatus(const QString&)), parent, SLOT(mostraMessaggioStatusBar(const QString&)));
    connect(this, SIGNAL(downloadRiepilogoTreno(quint32, QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadRiepilogoTreno(quint32, QString)));
    connect(this, SIGNAL(downloadRiepilogoTreno(quint32,QString,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadRiepilogoTreno(quint32, QString, QString)));
    connect(this, SIGNAL(downloadDettagliTreno(quint32,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadDettagliTreno(quint32, QString)));
    connect(this, SIGNAL(downloadDettagliTreno(quint32,QString,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadDettagliTreno(quint32, QString, QString)));
}


//restituisce una stringa contente il numero del treno monitorato
QString SchedaTreno::numero() const
{
    return m_numero;
}

//restituisce il titolo della scheda
//nel caso di una scheda per il monitoraggio di un treno restituisce semplicemente il numero del treno
QString SchedaTreno::titolo( bool /*titoloBreve*/) const
{
    return m_numero;
}

//slot
//questo slot viene richiamato quando scatta il timer
void SchedaTreno::aggiorna()
{
    cambiaStato(statoInAggiornamento);
    //cancella dallo StackedWidget i widget creati per gestire gli errori

    if (m_idNumeroAmbiguo != -1)
    {
        QWidget *widgetDaRimuovere = widget(m_idNumeroAmbiguo);
        widgetDaRimuovere->deleteLater();
        m_idNumeroAmbiguo = -1;
    }

    //è impostato il codice di origine della stazione?
    if (m_codiceStazioneOrigine.isEmpty())
        //no, allora effettua un download "normale
        emit (downloadRiepilogoTreno(idScheda(), numero()));
    else
        //si, allora passa al segnale di download anche il codice
        emit (downloadRiepilogoTreno(idScheda(), numero(), codiceOrigine()));
}

void SchedaTreno::downloadFinito(const QString &rispostaVT)
{
    //inizializza solo se si sta scaricando il riepilogo;
    if (!m_riepilogoScaricato)
        {
        m_widget->inizializza();
        m_parser->inizializza();
        //TODO per il momento inizializza l'orario del treno ad ogni nuovo download della scheda di riepilogo
        //ma quando si memorizzeranno i transiti bisogna analizzare caso per caso la situazione...
        m_orarioTreno.inizializza();
        }

    if (!m_riepilogoScaricato)
      {
        m_parser->impostaRispostaVTRiepilogo(rispostaVT);
        //cerca prima di tutto se il treno è previsto, in caso contrario esci subito
        if (m_parser->trenoNonPrevisto())
        {
            m_widget->impostaStato(TrenoVT::TrenoNonPrevisto);
            m_ultimoAgg = QDateTime::currentDateTime();
            m_widget->impostaAggiornamento(m_ultimoAgg.toString(Qt::DefaultLocaleShortDate));
            return;
        }

        //ora cerca se per caso il treno è stato soppresso e se è così esci subito

        if (m_parser->trenoSoppressoTotalmente())
        {
            m_widget->impostaStato(TrenoVT::TrenoCancellato);
            m_ultimoAgg = QDateTime::currentDateTime();
            m_widget->impostaAggiornamento(m_ultimoAgg.toString(Qt::DefaultLocaleShortDate));
            cambiaStato(statoMonitoraggioAttivo);
            return;
        }

        //controlla se ViaggiaTreno ha segnalato che il numero è ambiguo
        //in tale caso prepara un widget per la selezione
        if (m_parser->numeroTrenoAmbiguo())
        {
            ferma();
            m_idNumeroAmbiguo = addWidget(new WidgetDisambiguaNumeroTreno(this, numero(),
                                                                          m_parser->listaCodiciTreno(rispostaVT)));
            setCurrentIndex(m_idNumeroAmbiguo);
            cambiaStato(statoErrore);
            return;
        }



        //a questo punto si può analizzare il riepilogo
        if (!m_parser->analizzaRiepilogo(m_orarioTreno))
        {
             //c'è stato un errore, visualizza un testo di avviso
            cambiaStato(statoErrore);
            m_widget->impostaAggiornamento(QString::fromUtf8("Aggiornamento scheda non riuscito: errore nell'analisi della risposta di ViaggiaTreno"), true);
             m_riepilogoScaricato = false;
            return;
        }

        // è andato tutto bene, scarica la scheda con i dettagli
        if (m_codiceStazioneOrigine.isEmpty())
            emit (downloadDettagliTreno(idScheda(), numero()));
        else
            emit (downloadDettagliTreno(idScheda(), numero(), codiceOrigine()));
        m_riepilogoScaricato = true;
        //ed esci
        return;
    }

    //analizza la scheda con i dettagli del treno
    m_parser->impostaRispostaVTDettagli(rispostaVT);
    m_riepilogoScaricato = false;
    if (!m_parser->analizzaDettagli(m_orarioTreno))
    {
        //c'è stato un errore, visualizza un testo di avviso
        cambiaStato(statoErrore);
        m_widget->impostaAggiornamento(QString::fromUtf8("Aggiornamento scheda non riuscito: errore nell'analisi della risposta di ViaggiaTreno"), true);
        return;
    }

    m_ultimoAgg = QDateTime::currentDateTime();
    m_widget->aggiornaTreno(m_orarioTreno);
    cambiaStato(statoMonitoraggioAttivo);
    emit messaggioStatus(QString::fromUtf8("Aggiornati dati treno %1").arg(titolo(true)));
    m_widget->impostaAggiornamento(m_ultimoAgg.toString("dd/MM/yyyy hh:mm"));
}


//Salva i dati necessari per poter ripristinare la scheda alla prossima apertura
void SchedaTreno::salvaScheda(QSettings& settings)
{
    settings.setValue("tipo scheda", "treno");
    settings.setValue("numero treno", numero());
    settings.setValue("intervallo", intervallo());
    settings.setValue("codice stazione origine", codiceOrigine());
}

//cambia il codice della stazione di origine ed effettua una nuova richiesta a ViaggiaTreno
void SchedaTreno::cambiaCodiceOrigine(const QString& nuovoCodice)
{
    m_codiceStazioneOrigine = nuovoCodice;
    aggiorna();
    avvia();
}

