/***************************************************************************
 *   Copyright (C) 2010-2013 by fra74                                           *
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

#include "parser_viaggiatreno_base.h"

//sostituisce il nome della stazione per ovviare ad alcuni bug di ViaggiaTreno che fanno si che in
//alcuni casi i nomi delle stazioni siano incoerenti
//per esempio sulla rete FNM i treni vengono riportati con origine destinazione "M N Cadorna" ma poi
// non esiste tale stazione in ViaggiaTreno, bensì "Milano Nord Cadorna"

QString ParserViaggiaTrenoBase::sostituisciNomeStazione(const QString &nome)
{
    if (nome.contains("M N CADORNA"))
        return QString(nome).replace("M N CADORNA", "MILANO NORD CADORNA", Qt::CaseInsensitive);
    if (nome.contains("CAMNAGO LENTATE"))
        return QString(nome).replace("CAMNAGO LENTATE", "CAMNAGO-LENTATE", Qt::CaseInsensitive);

    return nome;
}
