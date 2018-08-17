/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74                                           *
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


#ifndef SCHEDATRENO_H
#define SCHEDATRENO_H

#include "schedaviaggiatreno.h"
#include "parser_viaggiatreno_treno.h"

class QViaggiaTreno;
class WidgetDatiTreno;


class SchedaTreno : public SchedaQViaggiaTreno
{
    Q_OBJECT

public:
    SchedaTreno(QViaggiaTreno *parent, const QString & numero, const unsigned int intervalloStandard = 2);

    QString numero() const;
    virtual QString titolo( bool titoloBreve = false) const;
    QString codiceOrigine() const {return m_codiceStazioneOrigine;}
    virtual void salvaScheda(QSettings& );

public slots:
    virtual void aggiorna();
    virtual void downloadFinito(const QString& rispostaVT);
    void cambiaCodiceOrigine(const QString& nuovoCodice);

private slots:


private:
    QString m_numero, m_codiceStazioneOrigine;
    int m_idTabelle, m_idNumeroSbagliato, m_idNumeroAmbiguo;
    bool m_riepilogoScaricato;
    WidgetDatiTreno* m_widget;
    TrenoVT::DatiTreno m_orarioTreno;
    ParserTrenoViaggiaTreno * m_parser;

signals:
    void downloadRiepilogoTreno(quint32, const QString&);
    void downloadRiepilogoTreno(quint32, const QString&, const QString&);
    void downloadDettagliTreno(quint32, const QString&);
    void downloadDettagliTreno(quint32, const QString&, const QString&);
};

//widget che viene mostrato quando il numero treno introdotto risulta ambiguo
//codice nel file schedatreno_widget.cpp
class WidgetDisambiguaNumeroTreno : public QWidget
{
Q_OBJECT

public:
    WidgetDisambiguaNumeroTreno(SchedaQViaggiaTreno *parent, const QString& numero, QMap<QString, QString>listaCodiciTreno);

private:
    QListWidget* lista;

private slots:
    void sceltaTreno();

signals:
    void numeroDisambiguato(const QString& nuovoCodice);
};

class TabellaFermate;
class WidgetDatiTreno : public QWidget
{
        Q_OBJECT

public:
        WidgetDatiTreno(SchedaTreno* parent);

        void impostaNumeroTreno(const QString& numero);
        void impostaAggiornamento(const QString& messaggio, bool errore = false);
        void aggiornaTreno(const TrenoVT::DatiTreno& treno);
        void impostaStato(TrenoVT::StatoTreno stato);
        void impostaStato(TrenoVT::StatoTreno stato, TrenoVT::DatiTreno treno);
        void impostaProvvedimenti(const QString& provvedimenti);

        void inizializza();

private:
        QLabel *m_titolo, *m_aggiornamento;
        QLabel *m_stazionePartenza, *m_partenzaTeorica, *m_binPartenzaProgrammato, *m_partenzaReale, *m_binPartenzaReale;
        QLabel *m_stazioneArrivo, *m_arrivoProgrammato, *m_binArrivoProgrammato, *m_arrivo, *m_binArrivoReale;
        QLabel *m_stato, *m_provvedimenti;
        QTabWidget *m_dettagli;
    TabellaFermate * m_tabella;

private slots:
        void linkAttivato(const QString& testo);
        void itemAttivato(QTableWidgetItem* item);

signals:
    void apriSchedaStazione(const QString&, bool);
};

class TabellaFermate : public QTableWidget
{
Q_OBJECT

public:
        TabellaFermate(QWidget* parent);

        void ridimensionaColonne();

protected:
        virtual void mouseMoveEvent(QMouseEvent *event);
};


#endif // SCHEDATRENO_H
