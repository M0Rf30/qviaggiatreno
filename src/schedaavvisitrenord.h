/***************************************************************************
 *   Copyright (C) 2008-2012 by fra74   *
 *   francesco.b74@gmail.com   *
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


#ifndef SCHEDAAVVISITRENORD_H
#define SCHEDAAVVISITRENORD_H

#include "parser_trenord.h"
#include "schedaviaggiatreno.h"
#include "ui_wgtavvisitrenord.h"

class QViaggiaTreno;

class WidgetAvvisiTrenord: public QWidget, private Ui::wgtAvvisiTrenord
{
    Q_OBJECT

public:
    WidgetAvvisiTrenord(QWidget *parent, ModelloAvvisiTrenord* modello);
};

class SchedaAvvisiTrenord: public SchedaQViaggiaTreno
{
    Q_OBJECT

public:
    SchedaAvvisiTrenord(QViaggiaTreno* parent, const unsigned int intervalloStandard = 5);

    QString titolo(bool = false) const {return QString::fromUtf8("Avvisi Trenord");}

    virtual void avvia();
    virtual void ferma();
    virtual void aggiorna();

    virtual void downloadFinito(const QString &);

    virtual ~SchedaAvvisiTrenord();


private:
    WidgetAvvisiTrenord* m_widgetAvvisi;
    ParserTrenord* m_parser;
    ModelloAvvisiTrenord *m_avvisi;

 signals:
    void aggiornaListaDirettrici();
};

#endif // SCHEDAAVVISITRENORD_H
