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

#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Configurazione
{
public:
    Configurazione();

    //intervallo del controllo periodico del corretto funzionamento di ViaggiaTreno (in minuti)
    int intervalloControlloVT();
    int intervalloControlloVTDefault() const {return m_def_intervalloControlloVT;}
    void impostaIntervalloControlloVT(int intervallo);

    //intervallo di timeout del controllo (in secondi)
    int timeoutControlloVT();
    int timeoutControlloVTDefault() const {return m_def_timeoutControlloVT;}
    void impostaTimeoutControllVT(int timeout);

    //intervallo tra una query a viaggiatreno e la successiva (in decimi di secondo)
    double intervalloQueryVT();
    double intervalloQueryVTDefault() const {return m_def_intervalloQueryVT;}
    void impostaIntervalloQueryVT(double intervallo);

    //impostazioni relative al proxy
    bool proxyUtilizzato();
    bool proxyUtilizzatoDefault() const {return m_def_usareProxy;}
    void impostaUtilizzoProxy(bool usareProxy);
    bool proxyRichiedeAutenticazione();
    bool proxyRichiedeAutenticazioneDefault() const {return m_def_autenticazioneProxyRichiesta;}
    void impostaRichiestaAutenticazioneProxy(bool autenticazioneProxy);
    bool proxyDiSistemaUtilizzato();
    bool proxyDiSistemaUtilizzatoDefault() const {return m_def_usareProxySistema;}
    void impostaUtilizzoProxyDiSistema(bool usareProxyDiSistema);
    QString hostProxy();
    QString hostProxyDefault() const {return m_def_hostProxy;}
    void impostaHostProxy(const QString& host);
    quint16 portaProxy();
    quint16 portaProxyDefault() const {return m_def_portaProxy;}
    void impostaPortaProxy(quint16 porta);
    QString nomeUtenteProxy();
    QString nomeUtenteProxyDefault() const {return m_def_nomeUtenteProxy;}
    void impostaNomeUtenteProxy(const QString& nome);
    QString passwordProxy();
    QString passwordProxyDefault() const {return m_def_passwordProxy;}
    void impostaPasswordProxy(const QString& password);

private:
    int m_def_intervalloControlloVT, m_def_timeoutControlloVT;
    double m_def_intervalloQueryVT;
    bool m_def_usareProxy, m_def_autenticazioneProxyRichiesta, m_def_usareProxySistema;
    QString m_def_hostProxy, m_def_nomeUtenteProxy, m_def_passwordProxy;
    quint16 m_def_portaProxy;

    QSettings m_settings;

};

#endif // CONFIG_H
