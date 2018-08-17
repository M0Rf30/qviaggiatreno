/***************************************************************************
 *   Copyright (C) 2008-2011 by fra74                                      *
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

#include "utils.h"

//funzioni varie per il debug dei nodi DOM
QString dumpNodo(QDomNode nodo)
{
    QString temp;
    QTextStream stream(&temp, QIODevice::WriteOnly);
    nodo.save(stream, 3);
    return temp;
}

void debugNodo(QDomNode nodo)
{
    QMessageBox msg;
    msg.setTextFormat(Qt::PlainText);
    msg.setText(dumpNodo(nodo));
    msg.exec();
}

void debugStringa(QString stringa)
{
    QMessageBox msg;
    msg.setTextFormat(Qt::PlainText);
    msg.setText(stringa);
    msg.exec();
}





