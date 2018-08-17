/***************************************************************************
 *   Copyright (C) 2010-2011 by fra74                                           *
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

#include "dialogo_configurazione.h"
#include "qviaggiatreno.h"

DialogoConfigurazione::DialogoConfigurazione(QViaggiaTreno* qvt) : QDialog(qvt)
{
    m_qvt = qvt;
    setupUi(this);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(pulsantePremuto(QAbstractButton*)));
    connect(checkBoxUsareProxy, SIGNAL(clicked(bool)), this, SLOT(checkBoxUsareProxyCliccato(bool)));
    connect(checkBoxAutenticazione, SIGNAL(clicked(bool)), this, SLOT(checkBoxAutenticazioneCliccato(bool)));
    connect(checkBoxUsareProxySistema, SIGNAL(clicked(bool)), this, SLOT(checkBoxUsareProxySistemaCliccato(bool)));
}

//questo metodo legge i valori di configurazione ed imposta i corrispondenti controlli
//nella finestra di dialogo
void DialogoConfigurazione::impostaConfigurazione()
{
    //scheda ViaggiaTreno
    spinBoxControlloVT->setValue(m_qvt->configurazione().intervalloControlloVT());
    spinBoxTimeoutControlloVT->setValue(m_qvt->configurazione().timeoutControlloVT());
    doubleSpinBoxQueryVT->setValue(m_qvt->configurazione().intervalloQueryVT());
    //scheda Proxy
    lineEditHost->setText(m_qvt->configurazione().hostProxy());
    spinBoxPorta->setValue(m_qvt->configurazione().portaProxy());
    lineEditNomeUtente->setText(m_qvt->configurazione().nomeUtenteProxy());
    lineEditPassword->setText(m_qvt->configurazione().passwordProxy());
    checkBoxUsareProxy->setChecked(m_qvt->configurazione().proxyUtilizzato());
    checkBoxAutenticazione->setChecked(m_qvt->configurazione().proxyRichiedeAutenticazione());
    checkBoxUsareProxySistema->setChecked(m_qvt->configurazione().proxyDiSistemaUtilizzato());
    // simula la selezione delle due checkbox nella scheda Proxy
    checkBoxUsareProxySistemaCliccato(m_qvt->configurazione().proxyDiSistemaUtilizzato());
    checkBoxUsareProxyCliccato(m_qvt->configurazione().proxyUtilizzato());
    checkBoxAutenticazioneCliccato(m_qvt->configurazione().proxyRichiedeAutenticazione());
}

//questo metodo, richiamato se il dialogo è stato chiuso premendo il pulsante Ok imposta i
//parametri di configurazione leggendo i valodi dei controli della finestra di dialogo
void DialogoConfigurazione::applicaConfigurazione()
{
    //scheda ViaggiaTreno
    m_qvt->configurazione().impostaIntervalloControlloVT(spinBoxControlloVT->value());
    m_qvt->configurazione().impostaTimeoutControllVT(spinBoxTimeoutControlloVT->value());
    m_qvt->configurazione().impostaIntervalloQueryVT(doubleSpinBoxQueryVT->value());
    //scheda proxy
    m_qvt->configurazione().impostaUtilizzoProxy(checkBoxUsareProxy->isChecked());
    m_qvt->configurazione().impostaRichiestaAutenticazioneProxy(checkBoxAutenticazione->isChecked());
    m_qvt->configurazione().impostaHostProxy(lineEditHost->text());
    m_qvt->configurazione().impostaPortaProxy(spinBoxPorta->value());
    m_qvt->configurazione().impostaNomeUtenteProxy(lineEditNomeUtente->text());
    m_qvt->configurazione().impostaPasswordProxy(lineEditPassword->text());
    m_qvt->configurazione().impostaUtilizzoProxyDiSistema(checkBoxUsareProxySistema->isChecked());
}

void DialogoConfigurazione::pulsantePremuto(QAbstractButton *pulsante)
{
    //controlla che sia stato premuto il pulsante per reimpostare i valori predefiniti
    //a questo scopo è sufficiente controllare il corrispondente ButtonRole
    if (buttonBox->buttonRole(pulsante) == QDialogButtonBox::ResetRole)
        ripristinaValoriPredefiniti();

}

//ripristina i valori predefiniti leggendoli dalla coppia privata della classe che memorizza la configurazione
void DialogoConfigurazione::ripristinaValoriPredefiniti()
{
    //scheda ViaggiaTreno
    spinBoxControlloVT->setValue(m_qvt->configurazione().intervalloControlloVTDefault());
    spinBoxTimeoutControlloVT->setValue(m_qvt->configurazione().timeoutControlloVTDefault());
    doubleSpinBoxQueryVT->setValue(m_qvt->configurazione().intervalloQueryVTDefault());
    //scheda Proxy
    lineEditHost->setText(m_qvt->configurazione().hostProxyDefault());
    spinBoxPorta->setValue(m_qvt->configurazione().portaProxyDefault());
    lineEditNomeUtente->setText(m_qvt->configurazione().nomeUtenteProxyDefault());
    lineEditPassword->setText(m_qvt->configurazione().passwordProxyDefault());
    checkBoxUsareProxy->setChecked(m_qvt->configurazione().proxyUtilizzatoDefault());
    checkBoxAutenticazione->setChecked(m_qvt->configurazione().proxyRichiedeAutenticazioneDefault());
    checkBoxUsareProxySistema->setChecked(m_qvt->configurazione().proxyDiSistemaUtilizzatoDefault());
    // simula la selezione delle due checkbox nella scheda Proxy
    checkBoxAutenticazioneCliccato(m_qvt->configurazione().proxyRichiedeAutenticazioneDefault());
    checkBoxUsareProxyCliccato(m_qvt->configurazione().proxyUtilizzatoDefault());

}

//abilita/disabilita i controlli a seconda dello stato del check box checkBoxUsareProxy
void DialogoConfigurazione::checkBoxUsareProxyCliccato(bool selezionato)
{

    if (selezionato)
    {
        checkBoxUsareProxySistema->setEnabled(true);
        if (!checkBoxUsareProxySistema->isChecked())
        {
            lineEditHost->setEnabled(true);
           spinBoxPorta->setEnabled(true);
           labelHost->setEnabled(true);
           labelPorta->setEnabled(true);
           checkBoxAutenticazione->setEnabled(true);

           checkBoxAutenticazioneCliccato(checkBoxAutenticazione->isChecked());
        }
    }
    else //disattiva tutto
        {
           checkBoxUsareProxySistema->setDisabled(true);
            lineEditHost->setDisabled(true);
           spinBoxPorta->setDisabled(true);
           checkBoxAutenticazione->setDisabled(true);
           lineEditNomeUtente->setDisabled(true);
           lineEditPassword->setDisabled(true);
           labelHost->setDisabled(true);
           labelPorta->setDisabled(true);
           labelNomeUtente->setDisabled(true);
           labelPassword->setDisabled(true);
        }

}

//abilita/disabilita i controlli a seconda dello stato del check box checkBoxAutenticazione
void DialogoConfigurazione::checkBoxAutenticazioneCliccato(bool selezionato)
{
    if (selezionato)
    {
        lineEditNomeUtente->setEnabled(true);
        lineEditPassword->setEnabled(true);
        labelNomeUtente->setEnabled(true);
        labelPassword->setEnabled(true);
    }
    else
    {
        lineEditNomeUtente->setDisabled(true);
        lineEditPassword->setDisabled(true);
        labelNomeUtente->setDisabled(true);
        labelPassword->setDisabled(true);
    }
}

//abilita/disabilita i controlli a seconda dello stato del check box checkBoxUsareProxySistema
void DialogoConfigurazione::checkBoxUsareProxySistemaCliccato(bool selezionato)
{
    //il checkbox è stato selezionato, disattivare tutti i controlli per la configurazione
    //manuale del proxy
    if (selezionato)
    {
        lineEditHost->setDisabled(true);
        spinBoxPorta->setDisabled(true);
        checkBoxAutenticazione->setDisabled(true);
        lineEditNomeUtente->setDisabled(true);
        lineEditPassword->setDisabled(true);
        labelHost->setDisabled(true);
        labelPorta->setDisabled(true);
        labelNomeUtente->setDisabled(true);
        labelPassword->setDisabled(true);
    }
    else
        //il checkBox non è stato selezionato, attivare i controlli
    {
        lineEditHost->setEnabled(true);
        spinBoxPorta->setEnabled(true);
        labelHost->setEnabled(true);
        labelPorta->setEnabled(true);

        checkBoxAutenticazione->setEnabled(true);
        //abilita i controlli per l'autenticazione solo se il checkbox corrispondente è selezionato
        if (checkBoxAutenticazione->isChecked())
        {
            labelNomeUtente->setEnabled(true);
            labelPassword->setEnabled(true);
            lineEditNomeUtente->setEnabled(true);
            lineEditPassword->setEnabled(true);
        }
    }

}
