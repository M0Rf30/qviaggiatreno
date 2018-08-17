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


#ifndef SCHEDAVIAGGIATRENO_H
#define SCHEDAVIAGGIATRENO_H
//
#include <QtWidgets>
#include <QtNetwork>
#include <QtXml>

class QViaggiaTreno;

//Questa è la classe base da cui derivano tutte le classi per le schede di monitoraggio
class SchedaQViaggiaTreno : public QStackedWidget
{
Q_OBJECT
public:
    enum TipoScheda {tsStazione, tsTreno, tsListaTreni, tsAvvisiTrenord};

    SchedaQViaggiaTreno(QViaggiaTreno *parent, TipoScheda ts, const unsigned int intervalloStandard);
    //restituisce un puntatore all'istanza della classe QViaggiaTreno
    QViaggiaTreno* qViaggiaTreno() {return m_qviaggiatreno;}

    virtual void avvia();
    virtual void ferma();

    //restituisce true se la scheda può essere stampata, false in caso contrario
    bool stampabile() const {return m_stampabile;}
    //restituisce  true se la scheda può essere esportata, false in caso contrario
    bool esportabile() const {return m_esportabile;}

    int intervallo() const;
    void impostaIntervallo(int nuovoInt);

    bool fermata() const {return !m_timer->isActive();}

    TipoScheda tipoScheda() const {	return m_tipo; }
    quint32 idScheda() const { return m_id;	}

    QString statoScheda() const { return m_stato; }

    virtual QString titolo(bool = false) const { return QString();}

    QDateTime ultimoAggiornamento() const { return m_ultimoAgg; }

    virtual void salvaScheda(QSettings & /*settings*/) {}

public slots:
    virtual void aggiorna();
    virtual void downloadFinito(const QString&) {}
    virtual void stampa() {}
    virtual void esporta() {}


protected:
    QTimer *m_timer;
    QString m_stato;
    QDateTime m_ultimoAgg;
    //


    //alcune costanti che rappresentano lo stato della scheda
    static QString statoNuovaScheda;
    static QString statoInAggiornamento;
    static QString statoMonitoraggioAttivo;
    static QString statoMonitoraggioFermato;
    static QString statoErrore;

    void cambiaStato(QString nuovoStato);

protected slots:

private:
    QViaggiaTreno* m_qviaggiatreno;
    quint32 m_id;
    TipoScheda m_tipo;
    static unsigned long s_id;
    bool m_stampabile, m_esportabile;

private:
   virtual QTextDocument* creaTextDocument() {return  new QTextDocument(this);}

signals:
    void GuiNonSincronizzata(quint32 id);
    void statoCambiato(quint32 id);
    void nomeSchedaCambiato(quint32 id);
    void apriSchedaStazione(const QString& stazione, bool nomeEsatto);
    void apriSchedaTreno(const QString& treno);
    void apriSchedaTreno(const QString& treno, const QString& codice);
    void messaggioStatus(const QString& msg);
};

int larghezzaItem(const QTableWidgetItem* item);
void itemSottolineato(QTableWidgetItem* item, bool sottolineato = true);

#endif
