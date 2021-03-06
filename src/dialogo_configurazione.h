/***************************************************************************
 *   Copyright (C) 2010 by fra74                                           *
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

#ifndef DIALOGO_CONFIGURAZIONE_H
#define DIALOGO_CONFIGURAZIONE_H

#include "ui_dlgconfigurazione.h"
#include "config.h"

class QViaggiaTreno;

class DialogoConfigurazione: public QDialog, private Ui::dlgConfigurazione
{
    Q_OBJECT
public:
    DialogoConfigurazione(QViaggiaTreno *qvt);

    void impostaConfigurazione();
    void applicaConfigurazione();

private slots:
    void pulsantePremuto(QAbstractButton* pulsante);
    void checkBoxAutenticazioneCliccato(bool selezionato);
    void checkBoxUsareProxyCliccato(bool selezionato);
    void checkBoxUsareProxySistemaCliccato(bool selezionato);

private:
    void ripristinaValoriPredefiniti();
    QViaggiaTreno* m_qvt;
};

#endif // DIALOGO_CONFIGURAZIONE_H
