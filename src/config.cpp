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

#include "config.h"

Configurazione::Configurazione() : m_settings(QSettings::IniFormat, QSettings::UserScope, "fra74", "QViaggiaTreno")
{
   //inizializzazione i valori di default

    m_def_intervalloControlloVT = 5; // minuti
    m_def_intervalloQueryVT = 0.1; // secondi;
    m_def_timeoutControlloVT = 30; //secondi

    //impostazioni del proxy
    m_def_usareProxy = false;
    m_def_autenticazioneProxyRichiesta = false;
    m_def_usareProxySistema = false;
    m_def_hostProxy = QString::null;
    m_def_portaProxy = 65535;
    m_def_nomeUtenteProxy = QString::null;
    m_def_passwordProxy = QString::null;
}

int Configurazione::intervalloControlloVT()
{
    int valore;

    m_settings.beginGroup("ViaggiaTreno");
    valore = m_settings.value("Frequenza controllo ViaggiaTreno", m_def_intervalloControlloVT).toInt();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaIntervalloControlloVT(int intervallo)
{
    m_settings.beginGroup("ViaggiaTreno");
    //salva le impostazioni solo se sono diverse dal valore di default
    if (intervallo != m_def_intervalloControlloVT)
        m_settings.setValue("Frequenza controllo ViaggiaTreno", intervallo);
    else
        m_settings.remove("Frequenza controllo ViaggiaTreno");
    m_settings.endGroup();
}

int Configurazione::timeoutControlloVT()
{
    int valore;

    m_settings.beginGroup("ViaggiaTreno");
    valore = m_settings.value("Timeout controllo ViaggiaTreno", m_def_timeoutControlloVT).toInt();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaTimeoutControllVT(int timeout)
{
    m_settings.beginGroup("ViaggiaTreno");
    //salva le impostazioni solo se sono diverse dal valore di default
    if (timeout != m_def_timeoutControlloVT)
        m_settings.setValue("Timeout controllo ViaggiaTreno", timeout);
    else
        m_settings.remove("Timeout controllo ViaggiaTreno");
    m_settings.endGroup();
}

double Configurazione::intervalloQueryVT()
{
    double valore;

    m_settings.beginGroup("ViaggiaTreno");
    valore = m_settings.value("Frequenza query ViaggiaTreno", m_def_intervalloQueryVT).toDouble();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaIntervalloQueryVT(double intervallo)
{
    m_settings.beginGroup("ViaggiaTreno");
    //salva le impostazioni solo se sono diverse dal valore di default
    if (intervallo != m_def_intervalloQueryVT)
        m_settings.setValue("Frequenza query ViaggiaTreno", intervallo);
    else
        m_settings.remove("Frequenza query ViaggiaTreno");
    m_settings.endGroup();
}

bool Configurazione::proxyUtilizzato()
{
    bool valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Utilizzare proxy", m_def_usareProxy).toBool();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaUtilizzoProxy(bool usareProxy)
{
    m_settings.beginGroup("Proxy");
    if (usareProxy != m_def_usareProxy)
        m_settings.setValue("Utilizzare proxy", usareProxy);
    else
        m_settings.remove("Utilizzare proxy");
    m_settings.endGroup();
}

bool Configurazione::proxyRichiedeAutenticazione()
{
    bool valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Proxy richiede autenticazione", m_def_autenticazioneProxyRichiesta).toBool();
    m_settings.endGroup();

    return valore;
}

bool Configurazione::proxyDiSistemaUtilizzato()
{
    bool valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Utilizzare proxy di sistema", m_def_usareProxySistema).toBool();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaUtilizzoProxyDiSistema(bool usareProxySistema)
{
    m_settings.beginGroup("Proxy");
    if (usareProxySistema != m_def_usareProxySistema)
        m_settings.setValue("Utilizzare proxy di sistema", usareProxySistema);
    else
        m_settings.remove("Utilizzare proxy di sistema");
    m_settings.endGroup();
}

void Configurazione::impostaRichiestaAutenticazioneProxy(bool autenticazioneProxy)
{
    m_settings.beginGroup("Proxy");
    if (autenticazioneProxy != m_def_autenticazioneProxyRichiesta)
        m_settings.setValue("Proxy richiede autenticazione", autenticazioneProxy);
    else
        m_settings.remove("Proxy richiede autenticazione");
    m_settings.endGroup();
}

QString Configurazione::hostProxy()
{
    QString valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Host proxy", m_def_hostProxy).toString();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaHostProxy(const QString &host)
{
    m_settings.beginGroup("Proxy");
    if (host != m_def_hostProxy)
        m_settings.setValue("Host proxy", host);
    else
        m_settings.remove("Host proxy");
    m_settings.endGroup();
}

quint16 Configurazione::portaProxy()
{
    quint16 valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Porta proxy", m_def_portaProxy).toUInt();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaPortaProxy(quint16 porta)
{
    m_settings.beginGroup("Proxy");
    if (porta != m_def_portaProxy)
        m_settings.setValue("Porta proxy", porta);
    else
        m_settings.remove("Porta proxy");
    m_settings.endGroup();
}


QString Configurazione::nomeUtenteProxy()
{
    QString valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Nome utente proxy", m_def_nomeUtenteProxy).toString();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaNomeUtenteProxy(const QString &nome)
{
    m_settings.beginGroup("Proxy");
    if (nome != m_def_nomeUtenteProxy)
        m_settings.setValue("Nome utente proxy", nome);
    else
        m_settings.remove("Nome utente proxy");
    m_settings.endGroup();
}


QString Configurazione::passwordProxy()
{
    QString valore;

    m_settings.beginGroup("Proxy");
    valore = m_settings.value("Password proxy", m_def_passwordProxy).toString();
    m_settings.endGroup();

    return valore;
}

void Configurazione::impostaPasswordProxy(const QString &password)
{
    m_settings.beginGroup("Proxy");
    if (password != m_def_passwordProxy)
        m_settings.setValue("Password proxy", password);
    else
        m_settings.remove("Password proxy");
    m_settings.endGroup();
}
