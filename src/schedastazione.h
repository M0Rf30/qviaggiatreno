
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

#ifndef SCHEDASTAZIONE_H
#define SCHEDASTAZIONE_H

#include "schedaviaggiatreno.h"
#include "parser_viaggiatreno_stazione.h"

#include "ui_wgtstazione.h"

enum tipoFiltro {NessunFiltro=0, FiltraCategoria=1, FiltraStazione=2};

class QViaggiaTreno;

class WidgetStazione: public QWidget, private Ui::wgtStazione
{
    Q_OBJECT
public:
    WidgetStazione(QWidget *parent, QAbstractItemModel* modelloArrivi,
                   QAbstractItemModel *modelloPartenze);

    void impostaTitolo(const QString& titolo);
    void impostaAggiornamento(const QString& aggiornamento, bool errore = false);
    tipoFiltro tipoFiltroArriviAttivo() const {return (tipoFiltro)comboBoxTipoFiltroArrivi->currentIndex();}
    tipoFiltro tipoFiltroPartenzeAttivo() const {return (tipoFiltro)comboBoxTipoFiltroPartenze->currentIndex();}
    QString filtroArriviAttivo() const {return comboBoxFiltroArrivi->currentText();}
    QString  filtroPartenzeAttivo() const {return comboBoxFiltroPartenze->currentText();}
    void aggiornaComboFiltroArrivi(tipoFiltro filtro, QString filtroCategoria, QString filtroStazione, QStringList categorie, QStringList stazioni);
    void aggiornaComboFiltroPartenze(tipoFiltro filtro, QString filtroCategoria, QString filtroStazione, QStringList categorie, QStringList stazioni);
    void impostaComboFiltri(tipoFiltro arrivi, tipoFiltro partenze);

public slots:
    void mouseEntered(const QModelIndex& index);
};

class SchedaStazione : public SchedaQViaggiaTreno
{
    Q_OBJECT

public:
    SchedaStazione(QViaggiaTreno *parent, const QString& nome, bool nomeEsatto, const unsigned int intervalloStandard = 2);

    QString nomeStazione() const;
    QString codiceStazione() const { return m_codice; }
    virtual QString titolo( bool titoloBreve = false) const;
    virtual void salvaScheda(QSettings& settings);
    virtual void ripristinaFiltri(QSettings& settings);

public slots:
    virtual void aggiorna();
    void cambiaCodiceStazione(const QString& nuovoCodice);
    void modificaNomeStazione();

    virtual void downloadFinito(const QString& rispostaVT);


private:
    int m_idTabella, m_idNomeSbagliato, m_idNomeAmbiguo;
    bool m_nomeEsatto;
    QString m_stazione, m_codice;
    WidgetStazione* m_widgetStazione;
    QDomDocument *m_documentoDom;
    ParserStazioneViaggiaTreno *m_parser;

    ModelloStazione *m_arrivi,  *m_partenze;
    QSortFilterProxyModel *m_filtroArrivi, *m_filtroPartenze;


    QString m_testoFiltroCategorieArrivi, m_testoFiltroCategoriePartenze,
        m_testoFiltroStazioniArrivi, m_testoFiltroStazioniPartenze;

    QWidget* creaWidgetErroreNome();
    void aggiornaComboFiltroArrivi(tipoFiltro filtroAttivo);
    void aggiornaComboFiltroPartenze(tipoFiltro filtroAttivo);

    void riapplicaFiltroArrivi();
    void riapplicaFiltroPartenze();
    void eliminaFiltriNonApplicabili();

private slots:
    void itemAttivato(const QModelIndex& index);
    void tipoFiltroArriviSelezionato(int);
    void tipoFiltroPartenzeSelezionato(int);
    void filtroArriviSelezionato(QString);
    void filtroPartenzeSelezionato(QString);

signals:
    void downloadStazione(quint32, const QString&);
    void downloadStazioneConCodice(quint32, const QString&);

};


//widget che viene mostrato quando il nome della stazione introdotto risulta ambiguo
//codice nel file schedastazione_widget.cpp
class WidgetDisambiguaNome : public QWidget
{
    Q_OBJECT

public:
    WidgetDisambiguaNome(SchedaStazione *parent, QMap<QString, QString>listaCodiciStazioni);

private:
    QListWidget* listaNomi;

private slots:
    void sceltaNomeStazione();

signals:
    void nomeDisambiguato(const QString& nuovoCodice);
};


//widget che visualizza il prospetto arrivi/partenze della stazione

#endif
