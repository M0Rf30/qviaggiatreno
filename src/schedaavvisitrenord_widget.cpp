/***************************************************************************
 *   Copyright (C) 2010-2012 by fra74                                           *
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

#include "schedaavvisitrenord.h"

#include <QHeaderView>
#include <QItemSelectionModel>

WidgetAvvisiTrenord::WidgetAvvisiTrenord(QWidget *parent, ModelloAvvisiTrenord* avvisi)
    : QWidget(parent), m_modello(avvisi)
{
    setupUi(this);

    QFont font = labelTitolo->font();
    font.setBold(true);
    font.setPointSize(qRound(font.pointSize()*1.5));
    labelTitolo->setFont(font);

    label->setText(QString::fromUtf8("In attesa del primo aggiornamento..."));
    labelAvvisi->setWordWrap(true);
    labelAvvisi->setText(QString());

    tabellaAvvisi->setModel(avvisi);
    tabellaAvvisi->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabellaAvvisi->setSelectionMode(QAbstractItemView::SingleSelection);
    tabellaAvvisi->setWordWrap(false);
    tabellaAvvisi->verticalHeader()->hide();
    QHeaderView* header = tabellaAvvisi->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(ModelloAvvisiTrenord::colAvviso, QHeaderView::Stretch);
    //i nomi delle linee unite possono essere molto lunghi: larghezza fissa, testo completo nel dettaglio
    header->setSectionResizeMode(ModelloAvvisiTrenord::colLinea, QHeaderView::Interactive);
    tabellaAvvisi->setColumnWidth(ModelloAvvisiTrenord::colLinea, 300);
    tabellaAvvisi->setTextElideMode(Qt::ElideRight);

    testoAvviso->setPlaceholderText(QString::fromUtf8("Selezionare un avviso per leggerne il testo completo"));

    connect(tabellaAvvisi->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &WidgetAvvisiTrenord::mostraAvvisoSelezionato);
    //dopo ogni aggiornamento seleziona il primo avviso (il più grave/recente)
    connect(avvisi, &QAbstractItemModel::modelReset, this, [this] {
        testoAvviso->clear();
        if (m_modello->rowCount(QModelIndex()) > 0)
            tabellaAvvisi->selectRow(0);
    });
}

void WidgetAvvisiTrenord::impostaAggiornamento(const QString &messaggio, bool errore)
{
    label->setText(messaggio);
    label->setStyleSheet(errore ? QStringLiteral("color: red; font-weight: bold;") : QString());
}

void WidgetAvvisiTrenord::impostaRiepilogo(const QString &riepilogo)
{
    labelAvvisi->setText(riepilogo);
}

void WidgetAvvisiTrenord::mostraAvvisoSelezionato()
{
    const QModelIndex corrente = tabellaAvvisi->selectionModel()->currentIndex();
    if (!corrente.isValid())
    {
        testoAvviso->clear();
        return;
    }

    const AvvisoTrenord avviso = m_modello->avviso(corrente.row());
    testoAvviso->setPlainText(QString::fromUtf8("%1 — %2\n%3\n\n%4")
                              .arg(QLocale().toString(avviso.orario(), QLocale::LongFormat),
                                   avviso.stato(), avviso.direttrice(), avviso.testo()));
}
