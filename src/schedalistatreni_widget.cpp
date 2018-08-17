/***************************************************************************
 *   Copyright (C) 2009-2011 by fra74                                           *
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

#include "schedalistatreni.h"
#include "items.h"

WidgetListaTreni::WidgetListaTreni(SchedaListaTreni* parent): QWidget(parent)
{
    //imposta il layout del widget;
    QVBoxLayout * layout = new QVBoxLayout;
    setLayout(layout);

    //costruisce l'etichetta per il titolo
    m_labelTitolo = new QLabel(this);
    //l'etichetta sarà centrata
    m_labelTitolo->setAlignment(Qt::AlignCenter);
    //imposta font con dimensioni del 50% maggiori di quella standard e in grassetto
    QFont font = m_labelTitolo->font();
    font.setBold(true);
    font.setPointSize(font.pointSize()*1.5);
    m_labelTitolo->setFont(font);
    //aggiunge l'etichetta al layout
    layout->addWidget(m_labelTitolo, 0);

    //costruisce l'etichetta con ora/data di aggiornamento
    m_labelAggiornamento = new QLabel(this);
    //l'etichetta sarà centrata
    m_labelAggiornamento->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_labelAggiornamento, 0);

    m_tabella = new TabellaLista(this);

    //imposta le colonne della tabella
    QStringList righe;
    righe<<QString::fromUtf8("Numero")<<QString::fromUtf8("Categoria")<<QString::fromUtf8("Stato")
        <<QString::fromUtf8("Origine")<<QString::fromUtf8("Orario partenza programmato")<<QString::fromUtf8("Orario partenza effettivo")
       <<QString::fromUtf8("Destinazione")<<QString::fromUtf8("Orario arrivo programmato")<<QString::fromUtf8("Orario arrivo effettivo")<<
         QString::fromUtf8("Ultima fermata")
      <<QString::fromUtf8("Orario fermata programmato")<<QString::fromUtf8("Orario fermata effettivo")<<QString::fromUtf8("Ultimo rilevamento")
     <<QString::fromUtf8("Orario transito")<<QString::fromUtf8("Ritardo");

    m_tabella->setRowCount(righe.count());
    m_tabella->setVerticalHeaderLabels(righe);

    layout->addWidget(m_tabella, 1 );



    //connessioni
    //	connect(m_tabella, SIGNAL(cellEntered(int, int)), this, SLOT(ingressoCella(int, int)));
    //connect(m_tabella, SIGNAL(itemActivated(QTableWidgetItem *)), this, SLOT(itemAttivato(QTableWidgetItem*)));
    connect(m_tabella, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(itemAttivato(QTableWidgetItem*)));

    connect(this, SIGNAL(apriSchedaStazione(const QString&, bool)), parent, SIGNAL(apriSchedaStazione(const QString&, bool)));
    connect(this, SIGNAL(apriSchedaTreno(const QString&)), parent, SIGNAL(apriSchedaTreno(const QString&)));
}

void WidgetListaTreni::itemAttivato(QTableWidgetItem* item)
{
    int riga = item->row();

    if (riga == ListaVT::rigaNumero)
        if (item->text() != "")
            emit(apriSchedaTreno(item->text()));

    if ((riga == ListaVT::rigaOrigine) || (riga == ListaVT::rigaDestinazione) || (riga == ListaVT::rigaUltimaFermata))
        if (item->text() != "")
            emit(apriSchedaStazione(ParserViaggiaTrenoBase::sostituisciNomeStazione(item->text()), true));
}

//aggiunge un treno al widget
void WidgetListaTreni::aggiungiTreno(ListaVT::DatiTreno *treno)
{
    int col;
    for (col = 0; col < m_tabella->columnCount(); col++)
    {
        QTableWidgetItem *item = m_tabella->item(ListaVT::rigaNumero, col);
        if (!item)
            break;
        else
            if (treno->numero().toInt() < item->text().toInt())
                break;
    }
    //aggiungi una nuova riga in fondo alla tabella
    m_tabella->insertColumn(col);

    //imposta le celle
    impostaCella(ListaVT::rigaNumero, col, treno->numero());
    impostaCella(ListaVT::rigaStato, col, treno->stringaStatoTreno());
}

//rimuove un treno dal widget
void WidgetListaTreni::rimuoviTreno(const QString& treno)
{
    for (int col = 0; col < m_tabella->columnCount(); col++)
    {
        QTableWidgetItem *item = m_tabella->item(ListaVT::rigaNumero, col);
        if (item)
            if (item->text() == treno)
            {
                m_tabella->removeColumn(col);
                return;
            }
    }
}


//imposta il testo della cella impostando contemporaneamente ogni eventuale formattazione necessaria
void WidgetListaTreni::impostaCella(int riga, int colonna, const QString& testo)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(testo.simplified());

    QFont font = item->font();

    //sottolinea il testo nelle colonne in cui è necessario ed usa il colore dei link
    if (riga == ListaVT::rigaNumero || riga == ListaVT::rigaOrigine || riga == ListaVT::rigaDestinazione || riga == ListaVT::rigaUltimaFermata)
    {
        font.setUnderline(true);
        item->setForeground(QApplication::palette().link());
    }

    item->setFont(font);

    m_tabella->setItem(riga, colonna, item);
}

int WidgetListaTreni::larghezzaItem(const QTableWidgetItem* item)
{
    if (!item)
        return -1;

    QFontMetrics fm(item->font());
    int larghezza = fm.width(item->text());
    return larghezza;
}

//aggiusta la dimensione delle colonne della tabella in modo che il contenuto delle celle sia visibile
//non usa resizeColumnsToContents perché funziona solo sulla parte visibile delle colonne
void WidgetListaTreni::ridimensionaColonne()
{
    int larghezza;

    //calcola la larghezza massima
    for (int col = 0; col <= m_tabella->columnCount(); col++)
    {
        larghezza = m_tabella->horizontalHeader()->sectionSizeHint(col);
        for (int riga = 0; riga < m_tabella->rowCount(); riga++)
            larghezza = qMax(larghezza, larghezzaItem(m_tabella->item(riga, col)));

        m_tabella->setColumnWidth(col, larghezza+10);
    }
}

void WidgetListaTreni::impostaTitolo(const QString& titolo)
{
    m_labelTitolo->setText(titolo);
}

void WidgetListaTreni::aggiornaTreno(const ListaVT::DatiTreno* treno)
{
    int colonna = -1;
    //ricerca la riga relativa al treno da aggiornare
    for (int i=0; i<m_tabella->columnCount();i++)
    {
        QTableWidgetItem* item = m_tabella->item(ListaVT::rigaNumero, i);
        if (!(item->text().compare(treno->numero())))
            colonna = i;
    }

    //aggiorna tutte le colonne (tranne quella del numero)
    impostaCella(ListaVT::rigaCategoria, colonna, treno->dato(ListaVT::dtCategoria));
    impostaCella(ListaVT::rigaStato, colonna, treno->stringaStatoTreno());
    impostaCella(ListaVT::rigaOrigine, colonna, treno->dato(ListaVT::dtOrigine));
    impostaCella(ListaVT::rigaPartenzaProgrammata, colonna, treno->dato(ListaVT::dtPartenzaProgrammata));
    impostaCella(ListaVT::rigaPartenzaEffettiva, colonna, treno->dato(ListaVT::dtPartenzaEffettiva));
    impostaCella(ListaVT::rigaDestinazione, colonna, treno->dato(ListaVT::dtDestinazione));
    impostaCella(ListaVT::rigaArrivoProgrammato, colonna, treno->dato(ListaVT::dtArrivoProgrammato));
    impostaCella(ListaVT::rigaArrivoEffettivo, colonna, treno->dato(ListaVT::dtArrivoEffettivo));
    impostaCella(ListaVT::rigaUltimaFermata, colonna, treno->dato(ListaVT::dtUltimaFermata));
    impostaCella(ListaVT::rigaOrarioFermataProgrammato, colonna, treno->dato(ListaVT::dtOrarioFermataProgrammato));
    impostaCella(ListaVT::rigaOrarioFermataEffettivo, colonna, treno->dato(ListaVT::dtOrarioFermataEffettivo));
    impostaCella(ListaVT::rigaUltimoRilevamento, colonna, treno->dato(ListaVT::dtUltimoRilevamento));
    impostaCella(ListaVT::rigaOrarioTransito, colonna, treno->dato(ListaVT::dtOrarioTransito));
    impostaCella(ListaVT::rigaRitardoTransito, colonna, treno->dato(ListaVT::dtRitardoTransito));

    ridimensionaColonne();

}

//imposta l'etichetta con ora e data del completamento dell'ultimo treno
void WidgetListaTreni::impostaAggiornamento(const QString& testo, bool errore)
{
    //se errore è true allora il parsing non è andato a buon fine, imposta il messaggio in rosso
    //e grassetto per evidenziarlo
    if (errore)
        m_labelAggiornamento->setText(QString("<font color=\"red\"><b>%1</b></font>").arg(testo));
    else
        m_labelAggiornamento->setText(testo);
}

//Questo dialogo viene visualizzato quando si è scelto da menu o toolbar la voce per
//aggiungere uno o più treni alla lista dei treni da monitorare
DialogoAggiuntaTreni::DialogoAggiuntaTreni(QWidget *parent) : QDialog(parent)
{
    //imposta il titolo
    setWindowTitle(QString::fromUtf8("Aggiunta treni alla lista"));
    //layout principale del dialogo
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    //layout orizzontale
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout, 0);

    //casella di testo per l'inserimento del numero del treno
    QLabel *labelLineEdit = new QLabel(QString::fromUtf8("Numero treno:"), this);
    hlayout->addWidget(labelLineEdit, 0);
    m_lineedit = new QLineEdit(this);
    //la casella accetta solo numeri interi tra 1 e 99999
    QIntValidator *validator = new QIntValidator(1, 99999, this);
    m_lineedit->setValidator(validator);
    QPushButton *aggiungi = new QPushButton(QString::fromUtf8("Aggiungi treno"), this);
    aggiungi->setDefault(true);

    hlayout->addWidget(m_lineedit, 1);
    hlayout->addWidget(aggiungi, 0);

    QLabel *labelListWidget = new QLabel(QString::fromUtf8("Elenco treni da aggiungere:"));
    layout->addWidget(labelListWidget, 0);
    m_listwidget = new QListWidget(this);
    layout->addWidget(m_listwidget, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(QString::fromUtf8("&Aggiungi treni"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    qobject_cast<QPushButton*>(buttonBox->buttons().at(0))->setAutoDefault(true);
    layout->addWidget(buttonBox, 0);

    //abilita l'ordinamento
    m_listwidget->setSortingEnabled(true);

    //connessioni
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(aggiungi, SIGNAL(clicked()), this, SLOT(aggiungiTreno()));
}

//elimina tutti i treni dalla tabella
void WidgetListaTreni::rimuoviTuttiITreni()
{
    m_tabella->clearContents();
    m_tabella->setColumnCount(0);
}

//slot

void DialogoAggiuntaTreni::aggiungiTreno()
{
    //richiesto da QIntValidator ma non utilizzato
    int pos;
    QString treno = m_lineedit->text();
    if (m_lineedit->validator()->validate(treno, pos) == QValidator::Acceptable)
    {
        //verifica che il numero non sia già nell'elenco dei numeri da aggiungere
        for (int i = 0; i < m_listwidget->count(); i++)
        {// i numeri sono uguali, esci dalla funzione senza far niente
            if (treno.compare(m_listwidget->item(i)->text()) == 0)
                return;
        }
        SortedAsIntListWidgetItem * item = new SortedAsIntListWidgetItem(treno);
        m_listwidget->addItem(item);
        m_lineedit->clear();
    }
    m_listwidget->sortItems();
}

//restituisce una lista di stringa contentente tutti i numeri dei treni
//di cui è stata richiesta l'aggiunta
QStringList DialogoAggiuntaTreni::listaTreni() const
{
    QStringList lista;
    for (int i = 0; i < m_listwidget->count(); i++ )
        lista.append(m_listwidget->item(i)->text());

    return lista;
}

//restituisce true se non ci sono treni da aggiungere, false in caso contrario
bool DialogoAggiuntaTreni::listaVuota() const
{
    return (m_listwidget->count() == 0);
}

//Questo dialogo viene visualizzato quando si è scelto da menu o toolbar la voce per
//rimuovere uno o più treni dalla lista dei treni da monitorare
DialogoRimozioneTreni::DialogoRimozioneTreni(const QStringList& lista, QWidget * parent) : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("Rimozione treni dalla lista"));
    //layout principale del dialogo
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QLabel *labelLista = new QLabel(QString::fromUtf8("Elenco treni:"));
    layout->addWidget(labelLista, 0);

    m_listwidget = new QListWidget(this);
    layout->addWidget(m_listwidget, 1);
    m_listwidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listwidget->setSortingEnabled(true);

    m_buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    m_buttonBox->addButton(QString::fromUtf8("&Rimuovi treni"), QDialogButtonBox::AcceptRole);
    m_buttonBox->addButton(QDialogButtonBox::Cancel);

    layout->addWidget(m_buttonBox, 0);

    //riempie il listwidget
    QStringListIterator it(lista);

    while (it.hasNext())
        m_listwidget->addItem(new SortedAsIntListWidgetItem(it.next()));


    //disabilita il pulsante "rimuovi treni"
    m_buttonBox->buttons().at(0)->setEnabled(false);

    //connessioni
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_listwidget, SIGNAL(itemSelectionChanged()), this, SLOT(selezioneModificata()));
}

//slot
//questo slot attiva il pulsante "rimuovi treni" solo nel caso che ci sia almeno
//un item selezionato nel listwidget, altrimenti lo disattiva
void DialogoRimozioneTreni::selezioneModificata()
{
    m_buttonBox->buttons().at(0)->setEnabled(m_listwidget->selectedItems().count());
}

//restituisce la lista dei treni attualmente selezionati per la rimozione
QStringList DialogoRimozioneTreni::listaTreni() const
{
    QStringList lista;

    QListIterator<QListWidgetItem *> it(m_listwidget->selectedItems());

    while (it.hasNext())
    {
        lista.append(it.next()->text());
    }

    return lista;
}

TabellaLista::TabellaLista(QWidget *parent) : QTableWidget(parent)
{
    //imposta le caratteristiche della tabella
    //righe a colori alternati
    setAlternatingRowColors(true);
    //disattiva selezione
    setSelectionMode(QAbstractItemView::NoSelection);
    //disattiva modifiche delle celle
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    //attiva il tracking del mouse
    setMouseTracking(true);
    verticalHeader()->setMouseTracking(true);

    horizontalHeader()->setVisible(false);
}

void TabellaLista::mouseMoveEvent(QMouseEvent *event)
{
    //cambia cursore solo se c'è almeno un treno in lista
    if (columnCount() != 0)
    {
        int riga = rowAt(event->y());
        if (riga == ListaVT::rigaNumero || riga == ListaVT::rigaOrigine || riga == ListaVT::rigaDestinazione || riga == ListaVT::rigaUltimaFermata )
            setCursor(Qt::PointingHandCursor);
        else
            setCursor(Qt::ArrowCursor);
    }

    event->accept();
}
