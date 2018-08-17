/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74                                           *
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


#include "schedatreno.h"
#include "parser_viaggiatreno_treno.h"

//questo widget si occupa di presentare all'utente una lista delle stazioni proposte da viaggiatreno per
//risolvere l'ambiguità del nome inserito dall'utente
WidgetDisambiguaNumeroTreno::WidgetDisambiguaNumeroTreno(SchedaQViaggiaTreno* parent, const QString& numero,
                                                         QMap<QString, QString> listaCodiciTreno) : QWidget(parent)
{
    //imposta il layout del dialogo
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    QLabel *messaggio = new QLabel(this);
    messaggio->setWordWrap(true);
    messaggio->setText(QString::fromUtf8("Il numero treno inserito ('<b>%1</b>') è ambiguo. Questo succede "
                                         " solitamente se su reti ferroviarie distinte (ad esempio RFI e FERROVIENORD) circolano due treni con"
                                         " lo stesso numero . Per risolvere questa ambiguità ViaggiaTreno propone una lista di treni con la "
                                         "relativa stazione di origine.<br/>Scegliere dalla lista sottostante"
                                         "  oppure fare clic sul pulsante di chiusura se desiderate semplicemente chiudere questa scheda").arg(numero));
    vbox->addWidget(messaggio);
    QHBoxLayout *hbox = new QHBoxLayout;
    lista= new QListWidget(this);
    QPushButton *btnSceltaStazione = new QPushButton(QString::fromUtf8("Scelta treno"), this);

    hbox->addWidget(lista);
    hbox->addWidget(btnSceltaStazione, 0, Qt::AlignCenter);
    vbox->addLayout(hbox);

    //scorre la lista dei codici stazione
    //per ogni elemento crea una apposita voce nella listbox
    QMapIterator<QString, QString> it(listaCodiciTreno);

    while (it.hasNext())
    {
        it.next();

        QListWidgetItem *item = new QListWidgetItem(it.key());
        item->setData(Qt::UserRole, it.value());
        lista->addItem(item);

    }

    lista->setSelectionMode(QAbstractItemView::SingleSelection);
    lista->setAlternatingRowColors(true);
    lista->setCurrentRow(0);

    //connessioni
    connect(btnSceltaStazione, SIGNAL(clicked()), this, SLOT(sceltaTreno()));
    connect(this, SIGNAL(numeroDisambiguato(const QString&)), parent, SLOT(cambiaCodiceOrigine(const QString&)));
}

//slot
//è stato scelto il nome della stazione dalla lista proposta, viene comunicato alla scheda con un segnale
void WidgetDisambiguaNumeroTreno::sceltaTreno()
{
    QListWidgetItem *item = lista->currentItem();

    emit numeroDisambiguato(item->data(Qt::UserRole).toString().section(";",1,1));
}


WidgetDatiTreno::WidgetDatiTreno(SchedaTreno *parent) : QWidget(parent)
{
        QVBoxLayout* layout = new QVBoxLayout;

        //costruisce l'etichetta per il titolo
        m_titolo = new QLabel(this);
        //l'etichetta sarà centrata
        m_titolo->setAlignment(Qt::AlignCenter);
        //imposta font con dimensioni del 50% maggiori di quella standard e in grassetto
        QFont font = m_titolo->font();
        font.setBold(true);
        font.setPointSize(font.pointSize()*1.5);
        m_titolo->setFont(font);
        //aggiunge l'etichetta al layout
        layout->addWidget(m_titolo, 0);

        //costruisce l'etichetta con ora/data di aggiornamento
        m_aggiornamento = new QLabel(this);
        //l'etichetta sarà centrata
        m_aggiornamento->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_aggiornamento, 0);

        //imposta il frame con i dati di partenza
        QFrame *framePartenza = new QFrame(this);
        framePartenza->setFrameStyle(QFrame::Panel|QFrame::Raised);
        layout->addWidget(framePartenza, 0);
        QGridLayout *layoutFramePartenza = new QGridLayout;
        framePartenza->setLayout(layoutFramePartenza);
        m_stazionePartenza = new QLabel(framePartenza);
        layoutFramePartenza->addWidget(m_stazionePartenza, 1,0);
        m_partenzaTeorica = new QLabel(framePartenza);
        layoutFramePartenza->addWidget(m_partenzaTeorica, 0, 2);
        m_binPartenzaProgrammato = new QLabel(framePartenza);
        layoutFramePartenza->addWidget(m_binPartenzaProgrammato, 0, 4);
        m_partenzaReale = new QLabel(framePartenza);
        layoutFramePartenza->addWidget(m_partenzaReale, 2, 2);
        m_binPartenzaReale = new QLabel(framePartenza);
        layoutFramePartenza->addWidget(m_binPartenzaReale, 2, 4);

        //imposta il frame con i dati di arrivo
        QFrame *frameArrivo = new QFrame(this);
        frameArrivo->setFrameStyle(QFrame::Panel|QFrame::Raised);
        layout->addWidget(frameArrivo, 0);
        QGridLayout *layoutFrameArrivo = new QGridLayout;
        frameArrivo->setLayout(layoutFrameArrivo);
        m_stazioneArrivo = new QLabel(frameArrivo);
        layoutFrameArrivo->addWidget(m_stazioneArrivo, 1,0);
        m_arrivoProgrammato = new QLabel(frameArrivo);
        layoutFrameArrivo->addWidget(m_arrivoProgrammato, 0, 2);
        m_binArrivoProgrammato = new QLabel(frameArrivo);
        layoutFrameArrivo->addWidget(m_binArrivoProgrammato, 0, 4);
        m_arrivo = new QLabel(frameArrivo);
        layoutFrameArrivo->addWidget(m_arrivo, 2, 2);
        m_binArrivoReale = new QLabel(frameArrivo);
        layoutFrameArrivo->addWidget(m_binArrivoReale, 2, 4);



        m_stato = new QLabel(this);
        m_stato->setWordWrap(true);
        m_stato->setFrameStyle(QFrame::Panel|QFrame::Raised);
        layout->addWidget(m_stato, 0);

        m_provvedimenti = new QLabel(this);
        m_provvedimenti->setWordWrap(true);
        m_provvedimenti->setFrameStyle(QFrame::Panel|QFrame::Raised);
        layout->addWidget(m_provvedimenti);

        //crea un nuovo tab widget che conterrà tutti i dettagli sulla marcia del treno
        m_dettagli = new QTabWidget(this);
        m_dettagli->setMovable(true);
        layout->addWidget(m_dettagli, 1);

        //aggiungi al tab widget una linguetta con l'elenco delle fermate intermedie
        m_tabella = new TabellaFermate(this);
        m_dettagli->addTab(m_tabella, QString::fromUtf8("Elenco fermate intermedie"));

        //aggiungi al tab widget una linguetta con i dettagli della marcia del treno, per il momento vuota
        //m_dettagli->addTab(new QWidget, QString::fromUtf8("Marcia treno"));
        setLayout(layout);

        //conessioni
        connect(m_stazionePartenza, SIGNAL(linkActivated(const QString& )), this, SLOT(linkAttivato(const QString&)));
        connect(m_stazioneArrivo, SIGNAL(linkActivated(const QString& )), this, SLOT(linkAttivato(const QString&)));
        connect(this, SIGNAL(apriSchedaStazione(const QString&, bool)), parent, SIGNAL(apriSchedaStazione(const QString&, bool)));
        //connect(m_tabella, SIGNAL(itemActivated(QTableWidgetItem *)), this, SLOT(itemAttivato(QTableWidgetItem*)));
        connect(m_tabella, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(itemAttivato(QTableWidgetItem*)));
}

void WidgetDatiTreno::impostaNumeroTreno(const QString& numero)
{
        m_titolo->setText(QString::fromUtf8("Treno %1").arg(numero));
}

void WidgetDatiTreno::impostaAggiornamento(const QString& messaggio, bool errore)
{
    //se errore è true allora il parsing non è andato a buon fine, imposta il messaggio in rosso
    //e grassetto per evidenziarlo
    if (errore)
       m_aggiornamento->setText(QString("<font color=\"red\"><b>%1</b></font>").arg(messaggio));
    else
        m_aggiornamento->setText(messaggio);

}

void WidgetDatiTreno::aggiornaTreno(const TrenoVT::DatiTreno& treno)
{
        //aggiorna numero e categoria treno
        m_titolo->setText(treno.numeroTreno());

        //aggiorna i dati di partenza, estratti dal riepilogo
        m_stazionePartenza->setText(QString::fromUtf8("Stazione di partenza: <a href=\"%1\">%1</a>").arg(treno.dato(TrenoVT::dtPartenza)));
        m_partenzaTeorica->setText(QString::fromUtf8("Orario di partenza programmato: <b>%1</b>").arg(treno.dato(TrenoVT::dtOrarioPartenzaProgrammato)));
        m_partenzaReale->setText(QString::fromUtf8("Orario di partenza reale: <b>%1</b>").arg(treno.dato(TrenoVT::dtOrarioPartenzaReale)));
        m_binPartenzaProgrammato->setText(QString::fromUtf8("Binario di partenza programmato: <b>%1</b>").arg(treno.dato(TrenoVT::dtBinarioPartenzaProgrammato)));
        m_binPartenzaReale->setText(QString::fromUtf8("Binario di partenza reale: <b>%1</b>").arg(treno.dato(TrenoVT::dtBinarioPartenzaReale)));

        //aggiorna i dati di arrivo
        m_stazioneArrivo->setText(QString::fromUtf8("Stazione di arrivo: <a href=\"%1\">%1</a>").arg(treno.dato(TrenoVT::dtArrivo)));
        m_arrivoProgrammato->setText(QString::fromUtf8("Orario di arrivo programmato: <b>%1</b>").arg(treno.dato(TrenoVT::dtOrarioArrivoProgrammato)));
        if (treno.statoTreno() == TrenoVT::TrenoArrivato )
                m_arrivo->setText(QString::fromUtf8("Orario di arrivo reale: <b>%1</b>").arg(treno.dato(TrenoVT::dtOrarioArrivo)));
        else
                m_arrivo->setText(QString::fromUtf8("Orario di arrivo stimato: <b>%1</b>").arg(treno.dato(TrenoVT::dtOrarioArrivo)));

        m_binArrivoProgrammato->setText(QString::fromUtf8("Binario di arrivo programmato <b>%1</b>").arg(treno.dato(TrenoVT::dtBinarioArrivoProgrammato)));
        m_binArrivoReale->setText(QString::fromUtf8("Binario di arrivo reale <b>%1</b>").arg(treno.dato(TrenoVT::dtBinarioArrivoReale)));

        //aggiorna i dati sull'ultimo rilevamento
        impostaStato(treno.statoTreno(), treno);
        impostaProvvedimenti(treno.dato(TrenoVT::dtProvvedimenti));

        //riempie la tabella
        QList<TrenoVT::Fermata*> fermate = treno.fermate();
        m_tabella->setRowCount(fermate.count());
        for (int i = 0; i<fermate.count(); i++)
        {
                TrenoVT::Fermata *fermata = fermate.at(i);
                QTableWidgetItem *item = new QTableWidgetItem(fermata->nomeFermata().simplified());
                itemSottolineato(item, true);
                m_tabella->setItem(i, TrenoVT::colFermata, item);
                if (fermata->effettuata())
                        item = new QTableWidgetItem(QString::fromUtf8("Sì"));
                else
                {
                        if (fermata->soppressa())
                        {
                                item = new QTableWidgetItem(QString::fromUtf8("Soppressa"));
                                item->setTextColor(Qt::red);
                        }
                        else
                                item = new QTableWidgetItem(QString::fromUtf8("No"));
                }
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colEffettuata, item);

                item = new QTableWidgetItem(fermata->binarioProgrammato().simplified());
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colBinarioProgrammato, item);

                item = new QTableWidgetItem(fermata->binarioReale().simplified());
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colBinarioReale, item);

                item = new QTableWidgetItem(fermata->oraArrivoProgrammata().simplified());
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colArrivoProgrammato, item);

                item = new QTableWidgetItem(fermata->oraArrivoStimata().simplified());
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colArrivoStimato, item);

                item = new QTableWidgetItem(fermata->oraArrivoReale().simplified());
                item->setTextAlignment(Qt::AlignCenter);
                m_tabella->setItem(i, TrenoVT::colArrivoReale, item);
        }

        m_tabella->ridimensionaColonne();
}

//imposta l'etichetta con lo stato del treno
void WidgetDatiTreno::impostaStato(TrenoVT::StatoTreno stato)
{
        switch(stato)
        {
                case TrenoVT::TrenoNonPrevisto:
                        m_stato->setText(QString::fromUtf8("Il numero del treno non è stato trovato. Probabilmente il treno"
                                                                                           " non è previsto oggi, o circola con"
                                                                                           " un numero differente. Controllare se il numero è corretto.")); break;

                case TrenoVT::TrenoCancellato:
                        m_stato->setText(QString::fromUtf8("Il treno oggi è stato soppresso")); break;

                default: ;
        }

}

void WidgetDatiTreno::impostaStato(TrenoVT::StatoTreno stato, TrenoVT::DatiTreno treno)
{
        switch(stato)
        {
                case TrenoVT::TrenoNonPrevisto : impostaStato(TrenoVT::TrenoNonPrevisto); break;
                case TrenoVT::TrenoNonPartito : m_stato->setText(QString::fromUtf8("Il treno non risulta ancora partito dalla stazione di partenza")); break;
                case TrenoVT::TrenoInViaggio :
                        {
                                QString temp;
                                temp = QString::fromUtf8("Il treno viaggia %1. ").arg(treno.dato(TrenoVT::dtRitardoTransito));
                                if (treno.dato(TrenoVT::dtUltimoRilevamento) != "")
                                        temp += QString::fromUtf8("Ultimo rilevamento a %1 alle ore %2").arg(treno.dato(TrenoVT::dtUltimoRilevamento)).arg(treno.dato(TrenoVT::dtOrarioTransito));
                                m_stato->setText(temp);
                        } break;
                case TrenoVT::TrenoArrivato :
                        {
                                QString temp;
                                temp = QString::fromUtf8("Il treno è arrivato %1. ").arg(treno.dato(TrenoVT::dtRitardoTransito));
                                m_stato->setText(temp);
                        } break;
                default: break;
        }
}

//cancella tutte le scritte nel widget
void WidgetDatiTreno::inizializza()
{
        //inizializza le scritte nel frame per la partenza
        m_stazionePartenza->setText(QString::fromUtf8("Stazione di partenza:"));
        m_partenzaTeorica->setText(QString::fromUtf8("Orario di partenza programmato:"));
        m_partenzaReale->setText(QString::fromUtf8("Orario di partenza reale:"));
        m_binPartenzaProgrammato->setText(QString::fromUtf8("Binario di partenza programmato:"));
        m_binPartenzaReale->setText(QString::fromUtf8("Binario di partenza reale:"));
        //inizializza le scritte nel frame per l'arrivo
        m_stazioneArrivo->setText(QString::fromUtf8("Stazione di arrivo:"));
        m_arrivoProgrammato->setText(QString::fromUtf8("Orario di arrivo programmato:"));
        m_arrivo->setText(QString::fromUtf8("Orario di arrivo reale:"));
        m_binArrivoProgrammato->setText(QString::fromUtf8("Binario di arrivo programmato:"));
        m_binArrivoReale->setText(QString::fromUtf8("Binario di arrivo reale:"));

        m_stato->clear();
        m_provvedimenti->clear();
        m_tabella->clearContents();

}

//slot
//Questo slot viene attivato quando si fa click sul nome di una stazione. Viene emesso un segnale per attivare l'apertura della scheda della stazione corrispondente
void WidgetDatiTreno::linkAttivato(const QString& testo)
{
    emit apriSchedaStazione(ParserViaggiaTrenoBase::sostituisciNomeStazione(testo), true);
}

void WidgetDatiTreno::itemAttivato(QTableWidgetItem* item)
{
        int col = item->column();

        if (col == TrenoVT::colFermata)
        emit(apriSchedaStazione(ParserViaggiaTrenoBase::sostituisciNomeStazione(item->text()), true));

}

TabellaFermate::TabellaFermate(QWidget *parent) : QTableWidget(parent)
{
        setAlternatingRowColors(true);
        setSelectionMode(QAbstractItemView::NoSelection);
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        verticalHeader()->setVisible(false);
        setMouseTracking(true);
        horizontalHeader()->setMouseTracking(true);

        QStringList etichette;
        etichette<<QString::fromUtf8("Fermata")<<QString::fromUtf8("Effettuata")<<QString::fromUtf8("Binario programmato")<<QString::fromUtf8("Binario reale")
                        <<QString::fromUtf8("Arrivo Programmato")<<QString::fromUtf8("Arrivo stimato")<<QString::fromUtf8("Arrivo reale")<<"";
        setColumnCount(etichette.count());
        setHorizontalHeaderLabels(etichette);
        horizontalHeader()->setStretchLastSection(true);

        ridimensionaColonne();
}

void TabellaFermate::mouseMoveEvent(QMouseEvent *event)
{

        int col = columnAt(event->x());
        if (col == TrenoVT::colFermata )
                setCursor(Qt::PointingHandCursor);
        else
                setCursor(Qt::ArrowCursor);

        event->accept();
}

//aggiusta la dimensione delle colonne della tabella in modo che il contenuto delle celle sia visibile
//non usa resizeColumnsToContents perché funziona solo sulla parte visibile delle colonne
void TabellaFermate::ridimensionaColonne()
{
        int larghezza;

        //calcola la larghezza massima
        for (int col = 0; col <= columnCount(); col++)
        {
                larghezza = horizontalHeader()->sectionSizeHint(col);
                for (int riga = 0; riga < rowCount(); riga++)
                        larghezza = qMax(larghezza, larghezzaItem(item(riga, col)));

                setColumnWidth(col, larghezza+10);
        }
}

//imposta il frame con la descrizione dei provvedimenti
void WidgetDatiTreno::impostaProvvedimenti(const QString& provvedimenti)
{
        m_provvedimenti->setText(provvedimenti);
}
