/***************************************************************************
 *   Copyright (C) 2009-2011 by fra74   *
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


#ifndef SCHEDALISTATRENI_H
#define SCHEDALISTATRENI_H

#include "schedaviaggiatreno.h"
#include "parser_viaggiatreno_treno.h"

class QViaggiaTreno;
class WidgetListaTreni;



class SchedaListaTreni: public SchedaQViaggiaTreno
{
    Q_OBJECT

public:
    SchedaListaTreni(QViaggiaTreno* parent, const unsigned int intervalloStandard = 3);

    virtual void avvia();
    virtual void ferma();
    virtual void aggiorna();
    virtual QString titolo(bool = false) const;
    void impostaTitolo();

    bool modificata() const {return m_modificata; }
    int numeroTreni() const {return m_listatreni.count();}

    void apriFile(const QString& filename);

    virtual void salvaScheda(QSettings& settings);

public slots:
    void apri();
    void salva();
    void salvaConNome();
    void aggiungiTreni();
    void rimuoviTreni();
    void rimuoviTuttiITreni();
    virtual void downloadFinito(const QString& rispostaVT);



private slots:
    void cambiaCodiceOrigine(const QString& nuovoCodice);

private:
    static int s_count;
    WidgetListaTreni* m_widget;
    int m_idTabella, m_idNumeroAmbiguo;

    bool m_modificata;
    QString m_titoloLista;
    QString m_nomefile;
    QDomDocument m_dom;
    ParserTrenoViaggiaTreno *m_parser;

    QMap<QString, ListaVT::DatiTreno*> m_listatreni;
    QQueue<QString> m_codatreni;
    QString m_trenoAttuale, m_trenoAmbiguo;

private:
    void aggiungiTreno(const QString& numero, const QString& codice = "");
    void rimuoviTreno(const QString& numero);

    void prossimoTreno();
    void analizzaRispostaVT(const QString& risposta, ListaVT::DatiTreno *treno);

    void salvaFile(const QString& filename);

signals:
    void downloadRiepilogoTreno(quint32, const QString&);
    void downloadRiepilogoTreno(quint32, const QString&, const QString&);
};

class TabellaLista;
class WidgetListaTreni: public QWidget
{
    Q_OBJECT


public:
    WidgetListaTreni(SchedaListaTreni *parent);
    void aggiungiTreno(ListaVT::DatiTreno *treno);
    void rimuoviTreno(const QString& treno);
    void ridimensionaColonne();
    void impostaTitolo(const QString& titolo);
    void aggiornaTreno(const ListaVT::DatiTreno* treno);
    void impostaAggiornamento(const QString& testo, bool errore = false);

    void rimuoviTuttiITreni();


private:
    TabellaLista* m_tabella;
    QLabel *m_labelTitolo, *m_labelAggiornamento;
    void impostaCella(int riga, int colonna, const QString& testo);
    int larghezzaItem(const QTableWidgetItem* item);


private slots:
    void itemAttivato(QTableWidgetItem* item);

signals:
    void apriSchedaStazione(const QString& stazione, bool nomeEsatto);
    void apriSchedaTreno(const QString& treno);
};

class DialogoAggiuntaTreni : public QDialog
{
    Q_OBJECT

public:
    DialogoAggiuntaTreni(QWidget* parent);
    bool listaVuota() const;
    QStringList listaTreni() const;

private:
    QListWidget *m_listwidget;
    QLineEdit * m_lineedit;

private slots:
    void aggiungiTreno();
};

class DialogoRimozioneTreni: public QDialog
{
    Q_OBJECT

public:
    DialogoRimozioneTreni(const QStringList& lista, QWidget* parent);
    bool listaVuota() const;
    QStringList listaTreni() const;

private:
    QListWidget *m_listwidget;
    QDialogButtonBox *m_buttonBox;

private slots:
    void selezioneModificata();
};

class TabellaLista : public QTableWidget
{
    Q_OBJECT

public:
    TabellaLista(QWidget* parent);

    void mouseMoveEvent(QMouseEvent *event);
};


#endif // SCHEDALISTATRENI_H
