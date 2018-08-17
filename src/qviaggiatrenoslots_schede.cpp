/***************************************************************************
 *   Copyright (C) 2009-2010 by fra74                                           *
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

#include "schedalistatreni.h"
#include "schedastazione.h"
#include "schedatreno.h"
#include "qviaggiatreno.h"

//questo file sorgente raggruppa tutti gli slot richiamati da azioni specifiche per ogni scheda


//questo slot viene richiamato quando cambia il nome di una scheda
//(per esempio perché si è cambiato nome della stazione
void QViaggiaTreno::aggiornaNomeScheda(quint32 idScheda)
{
    SchedaQViaggiaTreno *scheda = m_listaSchede.value(idScheda);

    m_schede->setTabText(m_schede->indexOf(scheda), scheda->titolo());
}

//questo slot viene richiato quando si sceglie la voce per cambiare il nome della stazione
//nel menu stazione. La funzione richiama semplicemente la funzione omonima nella scheda corrente
void QViaggiaTreno::modificaNomeStazione()
{
    SchedaStazione *scheda = qobject_cast<SchedaStazione*>(schedaCorrente());
    scheda->modificaNomeStazione();
}

//questo slot viene richiamato quando si sceglie la voce "apri" nel menu specifico di una scheda
//lo slot richiama semplicemente la funzione corrispondente della scheda
void QViaggiaTreno::apri()
{
    //per il momento solo la scheda lista treni ha una funzione apri, non c'è bisogno di
    //"indovinare" il tipo della scheda
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->apri();
}

void QViaggiaTreno::salva()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->salva();
}

void QViaggiaTreno::salvaConNome()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->salvaConNome();
}


void QViaggiaTreno::aggiungiTreni()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->aggiungiTreni();
}

void QViaggiaTreno::rimuoviTreni()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->rimuoviTreni();
}

void QViaggiaTreno::rimuoviTuttiITreni()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->rimuoviTuttiITreni();
}

void QViaggiaTreno::impostaTitolo()
{
    SchedaListaTreni *scheda = qobject_cast<SchedaListaTreni*>(schedaCorrente());
    scheda->impostaTitolo();
}

