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

#include "schedastazione.h"

//questo widget si occupa di presentare all'utente una lista delle stazioni proposte da viaggiatreno per
//risolvere l'ambiguità del nome inserito dall'utente
WidgetDisambiguaNome::WidgetDisambiguaNome(SchedaStazione* parent, QMap<QString, QString> listaCodiciStazioni) : QWidget(parent)
{
    //imposta il layout del dialogo
        QVBoxLayout *vbox = new QVBoxLayout;
        setLayout(vbox);
        QLabel *messaggio = new QLabel(this);
        messaggio->setWordWrap(true);
        messaggio->setText(QString::fromUtf8("Il nome della stazione ('<b>%1</b>') è ambiguo. Per risolvere"
            " questa ambiguità ViaggiaTreno ha proposto una lista di possibili stazioni. <br/>Scegliere dalla lista sottostante"
            " la stazione desiderata oppure fare clic sul pulsante di chiusura se desiderate semplicemente chiudere questa scheda").arg(parent->nomeStazione()));
        vbox->addWidget(messaggio);
        QHBoxLayout *hbox = new QHBoxLayout;
        listaNomi = new QListWidget(this);
        QPushButton *btnSceltaStazione = new QPushButton(QString::fromUtf8("Scelta stazione"), this);

        hbox->addWidget(listaNomi);
        hbox->addWidget(btnSceltaStazione, 0, Qt::AlignCenter);
        vbox->addLayout(hbox);

        //scorre la lista dei codici stazione
        //per ogni elemento crea una apposita voce nella listbox
        QMapIterator<QString, QString> it(listaCodiciStazioni);

        while (it.hasNext())
            {
            it.next();

            QListWidgetItem *item = new QListWidgetItem(it.key());
            item->setData(Qt::UserRole, it.value());
            listaNomi->addItem(item);

            }

        listaNomi->setSelectionMode(QAbstractItemView::SingleSelection);
        listaNomi->setAlternatingRowColors(true);
        listaNomi->setCurrentRow(0);

        //connessioni
        connect(btnSceltaStazione, SIGNAL(clicked()), this, SLOT(sceltaNomeStazione()));
        connect(this, SIGNAL(nomeDisambiguato(const QString&)), parent, SLOT(cambiaCodiceStazione(const QString&)));
}

//slot
//è stato scelto il nome della stazione dalla lista proposta, viene comunicato alla scheda con un segnale
void WidgetDisambiguaNome::sceltaNomeStazione()
{
    QListWidgetItem *item = listaNomi->currentItem();

    emit nomeDisambiguato(item->data(Qt::UserRole).toString());
}


WidgetStazione::WidgetStazione(QWidget *parent, QAbstractItemModel *modelloArrivi,
                               QAbstractItemModel* modelloPartenze) : QWidget(parent)
{
    setupUi(this);
    //imposta font con dimensioni del 50% maggiori di quella standard e in grassetto
    //non è possibile farlo direttamente in designer
    QFont font = labelTitolo->font();
    font.setBold(true);
        font.setPointSize(font.pointSize()*1.5);
    labelTitolo->setFont(font);

    tabellaArrivi->setModel(modelloArrivi);
    tabellaPartenze->setModel(modelloPartenze);

    tabellaArrivi->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tabellaPartenze->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //attiva il tracking dei movimenti del mouse
    tabellaArrivi->setMouseTracking(true);
    tabellaPartenze->setMouseTracking(true);
    tabellaArrivi->horizontalHeader()->setMouseTracking(true);
    tabellaPartenze->horizontalHeader()->setMouseTracking(true);


    connect (tabellaArrivi, SIGNAL(entered(QModelIndex)), this,
             SLOT(mouseEntered(const QModelIndex&)));
    connect (tabellaPartenze, SIGNAL(entered(QModelIndex)), this,
             SLOT(mouseEntered(const QModelIndex&)));

    connect (tabellaArrivi, SIGNAL(activated(QModelIndex)), parent,
             SLOT(itemAttivato(const QModelIndex&)));
    connect(tabellaPartenze, SIGNAL(activated(QModelIndex)), parent,
            SLOT(itemAttivato(const QModelIndex&)));

    connect(comboBoxTipoFiltroArrivi, SIGNAL(currentIndexChanged(int)), parent, SLOT(tipoFiltroArriviSelezionato(int)));
    connect(comboBoxTipoFiltroPartenze, SIGNAL(currentIndexChanged(int)), parent, SLOT(tipoFiltroPartenzeSelezionato(int)));
    connect(comboBoxFiltroArrivi, SIGNAL(activated(QString)), parent, SLOT(filtroArriviSelezionato(QString)));
    connect(comboBoxFiltroPartenze, SIGNAL(activated(QString)), parent, SLOT(filtroPartenzeSelezionato(QString)));

}

void WidgetStazione::impostaTitolo(const QString &titolo)
{
    labelTitolo->setText(titolo);
}

void WidgetStazione::impostaAggiornamento(const QString &aggiornamento, bool errore)
{
    //se errore è true allora il parsing non è andato a buon fine, imposta il messaggio in rosso
    //e grassetto per evidenziarlo
    if (errore)
        labelAggiornamento->setText(QString("<font color=\"red\"><b>%1</b></font>").arg(aggiornamento));
    else
        labelAggiornamento->setText(aggiornamento);
}


QWidget* SchedaStazione::creaWidgetErroreNome()
{
    QWidget* widget = new QWidget(this);

    QVBoxLayout * vbox = new QVBoxLayout;
    widget->setLayout(vbox);
    QLabel *labelErrore = new QLabel(this);
    labelErrore->setWordWrap(true);
    labelErrore->setText(QString::fromUtf8("Il nome della stazione ('<b>%1</b>') non è stato riconosciuto"
        " da ViaggiaTreno. Premete il pulsante sottostante per modificare il nome della stazione, "
        "oppure il pulsante di chiusura se desiderate semplicemente chiudere questa scheda").arg(nomeStazione()));
    QPushButton *btnCambiaNome = new QPushButton(QString::fromUtf8("Cambia nome"), this);
    connect(btnCambiaNome, SIGNAL(clicked()), this, SLOT(modificaNomeStazione()));
    vbox->addWidget(labelErrore);
    vbox->addWidget(btnCambiaNome, 0, Qt::AlignCenter);
    vbox->addStretch();

    return widget;
}


//imposta il cursore opportuno a seconda della colonna in cui ci si trova
void WidgetStazione::mouseEntered(const QModelIndex &index)
{
    int col = index.column();

    QTableView * view = qobject_cast<QTableView*>(sender());

    if ((col == StazioneVT::colNumero)||(col == StazioneVT::colStazione))
        view->setCursor(Qt::PointingHandCursor);
    else
        view->setCursor(Qt::ArrowCursor);
}

//aggiorna l'elenco delle stazioni o delle categorie nelle combo del filtro e riapplica il filtro
void WidgetStazione::aggiornaComboFiltroArrivi(tipoFiltro filtro, QString filtroCategoria,
                                               QString filtroStazione,QStringList categorie, QStringList stazioni)
{
    int idx = -1;

    switch (filtro)
    {
    case NessunFiltro: comboBoxFiltroArrivi->clear(); break;
    case FiltraCategoria:
    {
        comboBoxFiltroArrivi->clear();
        comboBoxFiltroArrivi->addItem("Tutte");
        comboBoxFiltroArrivi->addItems(categorie);
        //cerca se il filtro attuale è presente, in caso positiva lo seleziona, altrimenti seleziona "Tutte"
        idx = comboBoxFiltroArrivi->findText(filtroCategoria);
        if (idx == -1)
            comboBoxFiltroArrivi->setCurrentIndex(0);
        else
            comboBoxFiltroArrivi->setCurrentIndex(idx);
    } break;
   case FiltraStazione:
    {
        comboBoxFiltroArrivi->clear();
        comboBoxFiltroArrivi->addItem("Tutte");
        comboBoxFiltroArrivi->addItems(stazioni);
        idx = comboBoxFiltroArrivi->findText(filtroStazione);
        if (idx == -1)
            comboBoxFiltroArrivi->setCurrentIndex(0);
        else
            comboBoxFiltroArrivi->setCurrentIndex(idx);
    } break;


    }

}

void WidgetStazione::aggiornaComboFiltroPartenze(tipoFiltro filtro, QString filtroCategoria,
                                               QString filtroStazione,QStringList categorie, QStringList stazioni)
{
    int idx = -1;

    switch (filtro)
    {
    case NessunFiltro: comboBoxFiltroPartenze->clear(); break;
    case FiltraCategoria:
    {
        comboBoxFiltroPartenze->clear();
        comboBoxFiltroPartenze->addItem("Tutte");
        comboBoxFiltroPartenze->addItems(categorie);
        //cerca se il filtro attuale è presente, in caso positiva lo seleziona, altrimenti seleziona "Tutte"
        idx = comboBoxFiltroPartenze->findText(filtroCategoria);
        if (idx == -1)
            comboBoxFiltroPartenze->setCurrentIndex(0);
        else
            comboBoxFiltroPartenze->setCurrentIndex(idx);
    } break;
   case FiltraStazione:
    {
        comboBoxFiltroPartenze->clear();
        comboBoxFiltroPartenze->addItem("Tutte");
        comboBoxFiltroPartenze->addItems(stazioni);
        idx = comboBoxFiltroPartenze->findText(filtroStazione);
        if (idx == -1)
            comboBoxFiltroPartenze->setCurrentIndex(0);
        else
            comboBoxFiltroPartenze->setCurrentIndex(idx);
    } break;


    }

}

void WidgetStazione::impostaComboFiltri(tipoFiltro arrivi, tipoFiltro partenze)
{
    comboBoxTipoFiltroArrivi->setCurrentIndex(arrivi);
    comboBoxTipoFiltroPartenze->setCurrentIndex(partenze);
}
