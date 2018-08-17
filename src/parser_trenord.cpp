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

#include "parser_trenord.h"
#include "schedaviaggiatreno.h"
#include "utils.h"
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebPage>

ParserTrenord::ParserTrenord(SchedaQViaggiaTreno *scheda)
{
    m_scheda = scheda;
}

//analizza il testo della pagina all'URL http://www.trenord.it/mobile/it/breaking-news.aspx
// che contiene la lista delle direttrici trenord con alla destra del nome della direttrice
// una icona che può essere rossa o verde nel caso siano o no presenti avvisi aggiornati
//restituisce true se l'analisi è andata a buon fine e nel caso crea anche una coda di QString
//ciascuna contente l'indirizzo con la pagina dei dettagli di una direttrice per cui
//sono presenti avvisi
bool ParserTrenord::analizzaListaDirettrici(const QString &rispostaTN)
{
    QWebPage page;

    page.mainFrame()->setHtml(rispostaTN);

    //ottiene la lista di tutti i tag <li>, ciascuno contiene il dato di una linea...
    QWebElementCollection lista = page.mainFrame()->findAllElements("li.direttrici-item");

    if(lista.count() == 0)
        //qualcosa è andato storto, restituisci false
        return false;

   m_direttrici.clear();

   //per ogni direttrice controlla l'elemonto <li> ed in particolare il tag<img>
   //se l'immagine linkata dal tag contiene nel nome "indicator-on" allora la direttrice ha
   //avvisi aggiornati, altrimenti non fare nulla
   for (int i = 0; i<lista.count(); i++)
   {
       QWebElement img = lista.at(i).findFirst("img.indicator");
       if (!img.isNull())
       {
           if (img.attribute("src").contains("indicator-on"))
           {
               //si ci sono avvisi aggiornati
               //estraggo dal tag <a> l'indirizzo della pagina con gli avvisi e l'aggiungo alla coda
               //di pagine da scaricare
               QWebElement a = lista.at(i).findFirst("a");
               m_direttrici.enqueue(a.attribute("href"));
           }
       }
   }

    return true;
}

ModelloAvvisiTrenord::ModelloAvvisiTrenord(QWidget *parent) : QAbstractTableModel(parent)
{

}

//restituisce il numero di righe nel modello, cioè il numero complessivo di avvisi
int ModelloAvvisiTrenord::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_avvisi.count();
}

//restituisce il numero di colonne nel modello, attualmente 3: data/ora avviso - direttrice - avviso
int ModelloAvvisiTrenord::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 3;
}

QVariant ModelloAvvisiTrenord::headerData(int section, Qt::Orientation orientation, int role) const
{
    // usa l'implementazione di default per restituire i dati dell'header verticale
    if (orientation == Qt::Vertical)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (role == Qt::DisplayRole)
    {
        switch(section)
        {
        case 0: return QString::fromUtf8("Data/ora"); break;
        case 1: return QString::fromUtf8("Direttrice"); break;
        case 2: return QString::fromUtf8("Testo avviso"); break;
        default: return QVariant();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant ModelloAvvisiTrenord::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    //TODO: da implementare
    return QVariant();
}
