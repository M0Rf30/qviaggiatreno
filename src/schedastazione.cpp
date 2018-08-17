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

#include "schedastazione.h"
#include "qviaggiatreno.h"
#include "download_viaggiatreno.h"
#include "utils.h"


SchedaStazione::SchedaStazione(QViaggiaTreno* parent, const QString& nome, bool nomeEsatto, const unsigned int intervalloStandard)
    : SchedaQViaggiaTreno(parent, tsStazione, intervalloStandard)
{
    m_stazione = nome;
    m_codice = "";
    m_stato = statoNuovaScheda;
    m_documentoDom = 0L;
    m_idTabella = m_idNomeSbagliato = m_idNomeAmbiguo = -1;
    m_nomeEsatto = nomeEsatto;

    //imposta il parser
    m_parser = new ParserStazioneViaggiaTreno(this);

    //imposta i modelli
    m_arrivi = new ModelloStazione(this, false);
    m_partenze = new ModelloStazione(this, true);
    m_filtroArrivi = new QSortFilterProxyModel();
    m_filtroPartenze = new QSortFilterProxyModel();
    m_filtroArrivi->setDynamicSortFilter(true);
    m_filtroPartenze->setDynamicSortFilter(true);
    m_filtroArrivi->setSourceModel(m_arrivi);
    m_filtroPartenze->setSourceModel(m_partenze);

    //crea il widget della scheda
    m_widgetStazione = new WidgetStazione(this, m_filtroArrivi, m_filtroPartenze);
    m_idTabella = addWidget(m_widgetStazione);
    setCurrentIndex(m_idTabella);

    //imposta i valori iniziali dei filtri a "Tutte"
    m_testoFiltroCategorieArrivi = m_testoFiltroCategoriePartenze =
            m_testoFiltroStazioniArrivi = m_testoFiltroStazioniPartenze = "Tutte";
    //connessioni
    connect(this, SIGNAL(statoCambiato(quint32)), parent, SLOT(aggiornaStatoScheda(quint32)));
    connect(this, SIGNAL(nomeSchedaCambiato(quint32)), parent, SLOT(aggiornaNomeScheda(quint32)));
    connect(this, SIGNAL(apriSchedaStazione(const QString&, bool)), parent, SLOT(nuovaStazione(const QString&, bool)));
    connect(this, SIGNAL(apriSchedaTreno(const QString&, const QString&)), parent, SLOT(nuovoTreno(const QString&, const QString&)));
    connect(this, SIGNAL(messaggioStatus(const QString&)), parent, SLOT(mostraMessaggioStatusBar(const QString&)));
    connect(this, SIGNAL(downloadStazione(quint32,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadStazione(quint32,QString)));
    connect(this, SIGNAL(downloadStazioneConCodice(quint32,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadStazioneCodice(quint32,QString)));

}

//restituisce il nome della stazione
//seguendo la convenzione di ViaggiaTreno il nome della stazione viene restituito in maiuscolo
QString SchedaStazione::nomeStazione() const
{
    return m_stazione.toUpper();
}

//restituisce una stringa con il "titolo" di questa scheda, che viene ad esempio usato come etichetta della TabBar
// se titoloItem == true restituisce un titolo per l'item dell'albero
//seguendo la convenzione di Viaggiatreno il nome della stazione verrà scritto tutto in maiuscolo
QString SchedaStazione::titolo( bool titoloBreve ) const
{
    if (titoloBreve)
        return nomeStazione();


    return QString::fromUtf8("Stazione di %1").arg(nomeStazione());
}



//questo slot viene richiamato quando scatta il timer
void SchedaStazione::aggiorna()
{
    cambiaStato(statoInAggiornamento);

    //cancella dallo StackedWidget i widget creati per gestire gli errori
    if (m_idNomeSbagliato != -1)
    {
        QWidget *widgetDaRimuovere = widget(m_idNomeSbagliato);
        widgetDaRimuovere->deleteLater();
        m_idNomeSbagliato = -1;
    }

    if (m_idNomeAmbiguo != -1)
    {
        QWidget *widgetDaRimuovere = widget(m_idNomeAmbiguo);
        widgetDaRimuovere->deleteLater();
        m_idNomeAmbiguo = -1;
    }


    if (m_codice.isEmpty())
        emit (downloadStazione(idScheda(), nomeStazione()));
    else
        emit (downloadStazioneConCodice(idScheda(), codiceStazione()));

}

//questo slot viene richiamato quando è finito il download della risposta dal sito di viaggiatreno
void SchedaStazione::downloadFinito(const QString& rispostaVT)
{

    //imposta nel parser la risposta di ViaggiaTreno
    m_parser->impostaRispostaVT(rispostaVT);


    //controlla se è stata trovata la stazione e in caso contrario mostra finestra di errore
    if (m_parser->stazioneNonTrovata())
    {
        ferma();
        m_idNomeSbagliato = addWidget(creaWidgetErroreNome());
        setCurrentIndex(m_idNomeSbagliato);
        cambiaStato(statoErrore);
        return;
    }


    //controlla se ViaggiaTreno ha segnalato che il nome è ambiguo
    //in tale caso prepara un widget per la selezione
    if (m_parser->nomeStazioneAmbiguo())
    {
        //si è chiesto di aprire una scheda usando esattamente il nome passato?
        if (m_nomeEsatto)
        {
            //ottiene dal parser la lista dei codici stazioni
            QMap<QString, QString> listaCodici = m_parser->listaCodiciStazioni(rispostaVT);

            //scorre la lista e cerca se esiste una stazione con ESATTAMENTE lo stesso nome
            QMapIterator<QString, QString> it(listaCodici);
            while (it.hasNext())
            {
                it.next();
                if (!QString::compare(m_stazione, it.key(), Qt::CaseInsensitive))
                {
                    //trovato nome stazione identico, cambia il codice stazione ed esci
                    cambiaCodiceStazione(it.value());
                    return;
                }

            }

        }

        ferma();
        m_idNomeAmbiguo = addWidget(new WidgetDisambiguaNome(this, m_parser->listaCodiciStazioni(rispostaVT)));
        setCurrentIndex(m_idNomeAmbiguo);
        cambiaStato(statoErrore);
        return;
    }

    if (!m_parser->analizza())
    {
        //errore nel parsing
        cambiaStato(statoErrore);
        m_widgetStazione->impostaAggiornamento(QString::fromUtf8("Aggiornamento scheda non riuscito: errore nell'analisi della risposta di ViaggiaTreno"), true);
        return;
    }

    m_stazione = m_parser->stazione();
    emit(nomeSchedaCambiato(idScheda()));
    m_widgetStazione->impostaTitolo(titolo());

    m_arrivi->aggiornaModello(m_parser->arrivi());
    m_partenze->aggiornaModello(m_parser->partenze());

    eliminaFiltriNonApplicabili();

    m_widgetStazione->aggiornaComboFiltroArrivi(m_widgetStazione->tipoFiltroArriviAttivo(),
                                                m_testoFiltroCategorieArrivi, m_testoFiltroStazioniArrivi,
                                                m_arrivi->categorie(), m_arrivi->stazioni());
    m_widgetStazione->aggiornaComboFiltroPartenze(m_widgetStazione->tipoFiltroPartenzeAttivo(),
                                                m_testoFiltroCategoriePartenze, m_testoFiltroStazioniPartenze,
                                                m_partenze->categorie(), m_partenze->stazioni());

    riapplicaFiltroArrivi();
    riapplicaFiltroPartenze();


     m_ultimoAgg = QDateTime::currentDateTime();
    m_widgetStazione->impostaAggiornamento(QString::fromUtf8("Ultimo aggiornamento: %1").
                                           arg(m_ultimoAgg.toString("dd/MM/yyyy hh:mm")));

    cambiaStato(statoMonitoraggioAttivo);
    emit messaggioStatus(QString::fromUtf8("Aggiornati dati stazione di %1").arg(titolo(true)));
}

void SchedaStazione::modificaNomeStazione()
{
    bool ok;
    QString nuovoNome = QInputDialog::getText(this, QString::fromUtf8("Modifica nome stazione"), QString::fromUtf8("Inserire il nuovo nome della stazione:"),
                                              QLineEdit::Normal, nomeStazione(), &ok);

    if (ok & !nuovoNome.isEmpty())
    {
        m_stazione = nuovoNome;
        m_codice = "";
        m_widgetStazione->impostaTitolo(titolo());

        //assicurarsi che venga visualizzato il widget della stazione e non il widget di errore
        setCurrentIndex(m_idTabella);
        emit(nomeSchedaCambiato(idScheda()));
        aggiorna();
        avvia();
    }
}

//questo slot risponde ad un attivazione (click, doppio click o quello che
//prevede il sistema operativo) di una cella nelle viste con il prospetto degli arrivi o partenze
//e se il click è avvvenuto in una delle due colonne con link simulati emette un segnale
//che fa aprire la scheda corrispondente
void SchedaStazione::itemAttivato(const QModelIndex& index)
{
    //Utilizzando un modello proxy per il filtraggio sorge una complicazione
    //L'indice ad un elemento del modello proxy non necessariamente
    //corrisponde allo stesso indice del modello originale ed in ogni caso la funzione model()
    //restituisce il modello proxy
    //Di conseguenza si deve per prima cosa recuperare un puntatore al modello filtrato (il proxy)
    const QSortFilterProxyModel *modelloFiltrato = qobject_cast<const QSortFilterProxyModel*>(index.model());
    //Una volta ottenuto utilizzando mapToSource si può convertire l'indice al corrispondente
    //indice nel modello originale
    QModelIndex indice_originale = modelloFiltrato->mapToSource(index);

    int col = indice_originale.column();

    //recupera il puntatore al modello dall'indice
     const ModelloStazione *modello = qobject_cast<const ModelloStazione*>(indice_originale.model());

     if (col == StazioneVT::colStazione)
         emit(apriSchedaStazione(ParserViaggiaTrenoBase::sostituisciNomeStazione(modello->data(indice_originale, Qt::DisplayRole).toString()), true));

     if (col == StazioneVT::colNumero)
         emit(apriSchedaTreno(modello->data(indice_originale, Qt::DisplayRole).toString(),
                              modello->data(indice_originale, Qt::UserRole).toString()));
}

//slot
//cambia il codice della stazione e riaggiorna la scheda
void SchedaStazione::cambiaCodiceStazione(const QString& nuovoCodice)
{
    m_codice = nuovoCodice;
    setCurrentIndex(m_idTabella);
    aggiorna();
    avvia();
}


//Salva i dati necessari per poter ripristinare la scheda alla prossima apertura
void SchedaStazione::salvaScheda(QSettings& settings)
{
    settings.setValue("tipo scheda", "stazione");
    settings.setValue("intervallo", intervallo());
    settings.setValue("nome stazione", nomeStazione());
    settings.setValue("codice stazione", codiceStazione());

    //salva le impostazioni dei filtri
    settings.setValue("filtro arrivi attivo", m_widgetStazione->tipoFiltroArriviAttivo());
    settings.setValue("filtro categorie arrivi selezionato", m_testoFiltroCategorieArrivi);
    settings.setValue("filtro stazioni arrivi selezionato", m_testoFiltroStazioniArrivi);
    settings.setValue("filtro partenze attivo",m_widgetStazione->tipoFiltroPartenzeAttivo());
    settings.setValue("filtro categorie partenze selezionato", m_testoFiltroCategoriePartenze);
    settings.setValue("filtro stazioni partenze selezionato", m_testoFiltroStazioniPartenze);


}

//riapplica il filtro alla tabella degli arrivi
void SchedaStazione::riapplicaFiltroArrivi()
{
    //per prima cosa invalida il filtro attuale
    m_filtroArrivi->invalidate();
    m_filtroArrivi->setFilterFixedString("");
    m_filtroArrivi->setFilterKeyColumn(-1);



    switch(m_widgetStazione->tipoFiltroArriviAttivo())
    {
    case NessunFiltro: break;
    case FiltraCategoria:
    {
     //si è scelto di filtrare sulla categorie, è inutile procedere però se si è scelto di mostrare tutte le categorie
    if (!m_testoFiltroCategorieArrivi.contains("Tutte"))
    {
    //seleziona la colonna per il filtro
    m_filtroArrivi->setFilterKeyColumn(StazioneVT::colCategoria);
    m_filtroArrivi->setFilterFixedString(m_widgetStazione->filtroArriviAttivo());
     }
    } break;
    case FiltraStazione:
    {
    if (!m_testoFiltroStazioniArrivi.contains("Tutte"))
    {
    m_filtroArrivi->setFilterKeyColumn(StazioneVT::colStazione);
    m_filtroArrivi->setFilterFixedString(m_widgetStazione->filtroArriviAttivo());
    }
    } break;
    }
}

//riapplica il filtro alla tabella delle partenze
void SchedaStazione::riapplicaFiltroPartenze()
{
    //per prima cosa invalida il filtro attuale
    m_filtroPartenze->invalidate();
    m_filtroPartenze->setFilterFixedString("");
    m_filtroPartenze->setFilterKeyColumn(-1);

    switch(m_widgetStazione->tipoFiltroPartenzeAttivo())
    {
    case NessunFiltro: break;
    case FiltraCategoria:
    {
     //si è scelto di filtrare sulla categorie, è inutile procedere però se si è scelto di mostrare tutte le categorie
    if (!m_testoFiltroCategoriePartenze.contains("Tutte"))
    {
    //seleziona la colonna per il filtro
    m_filtroPartenze->setFilterKeyColumn(StazioneVT::colCategoria);
    m_filtroPartenze->setFilterFixedString(m_widgetStazione->filtroPartenzeAttivo());
     }
    } break;
    case FiltraStazione:
    {
    if (!m_testoFiltroStazioniPartenze.contains("Tutte"))
    {
    m_filtroPartenze->setFilterKeyColumn(StazioneVT::colStazione);
    m_filtroPartenze->setFilterFixedString(m_widgetStazione->filtroPartenzeAttivo());
    }
    } break;
    }
}

//questo slot viene attivato quando si cambia il tipo di filtro.
void SchedaStazione::tipoFiltroArriviSelezionato(int tipo)
{
    m_widgetStazione->aggiornaComboFiltroArrivi((tipoFiltro)tipo, m_testoFiltroCategorieArrivi, m_testoFiltroStazioniArrivi,
                                                m_arrivi->categorie(), m_arrivi->stazioni());

   riapplicaFiltroArrivi();
}

void SchedaStazione::tipoFiltroPartenzeSelezionato(int tipo)
{
    m_widgetStazione->aggiornaComboFiltroPartenze((tipoFiltro)tipo, m_testoFiltroCategoriePartenze, m_testoFiltroStazioniPartenze,
                                                m_partenze->categorie(), m_partenze->stazioni());

    riapplicaFiltroPartenze();
}

void SchedaStazione::filtroArriviSelezionato(QString filtro)
{
    if (m_widgetStazione->tipoFiltroArriviAttivo() == FiltraCategoria)
        m_testoFiltroCategorieArrivi = filtro;
    else if (m_widgetStazione->tipoFiltroArriviAttivo() == FiltraStazione)
        m_testoFiltroStazioniArrivi = filtro;

    riapplicaFiltroArrivi();
}

void SchedaStazione::filtroPartenzeSelezionato(QString filtro)
{
    if (m_widgetStazione->tipoFiltroPartenzeAttivo() == FiltraCategoria)
        m_testoFiltroCategoriePartenze = filtro;
    else if (m_widgetStazione->tipoFiltroPartenzeAttivo() == FiltraStazione)
        m_testoFiltroStazioniPartenze = filtro;

    riapplicaFiltroPartenze();
}


//ripristina i filtri che erano selezionati al momento della chiusura di viaggiatreno
void SchedaStazione::ripristinaFiltri(QSettings &settings)
{
    m_testoFiltroCategorieArrivi = settings.value("filtro categorie arrivi selezionato", "Tutte").toString();
    m_testoFiltroStazioniArrivi = settings.value("filtro stazioni arrivi selezionato", "Tutte").toString();
    m_testoFiltroCategoriePartenze = settings.value("filtro categorie partenze selezionato", "Tutte").toString();
    m_testoFiltroStazioniPartenze =    settings.value("filtro stazioni partenze selezionato", "Tutte").toString();

    tipoFiltro arrivi =  (tipoFiltro)settings.value("filtro arrivi attivo", NessunFiltro).toInt();
    tipoFiltro partenze = (tipoFiltro)settings.value("filtro partenze attivo", NessunFiltro).toInt();

    m_widgetStazione->aggiornaComboFiltroArrivi(arrivi,
                                                m_testoFiltroCategorieArrivi, m_testoFiltroStazioniArrivi,
                                                m_arrivi->categorie(), m_arrivi->stazioni());

    m_widgetStazione->aggiornaComboFiltroPartenze(partenze,
                                                m_testoFiltroCategoriePartenze, m_testoFiltroStazioniPartenze,
                                                m_partenze->categorie(), m_partenze->stazioni());

    m_widgetStazione->impostaComboFiltri(arrivi, partenze);

    riapplicaFiltroArrivi();
    riapplicaFiltroPartenze();
}

//questo metodo controlla se il testo di ciascuno dei 4 filtri attivo è ancora presente nella corrispondente lista
//in caso contrario lo reimposta al valore di default perché non è più applicabile
void SchedaStazione::eliminaFiltriNonApplicabili()
{
    if (!m_arrivi->categorie().contains(m_testoFiltroCategorieArrivi))
        m_testoFiltroCategorieArrivi = "Tutte";
    if (!m_arrivi->stazioni().contains(m_testoFiltroStazioniArrivi))
        m_testoFiltroStazioniArrivi = "Tutte";
    if (!m_partenze->categorie().contains(m_testoFiltroCategoriePartenze))
        m_testoFiltroCategoriePartenze = "Tutte";
    if (!m_partenze->stazioni().contains(m_testoFiltroStazioniPartenze))
        m_testoFiltroStazioniPartenze = "Tutte";
}
