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
#ifndef PARSER_TRENORD_H
#define PARSER_TRENORD_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QStringList>

class SchedaQViaggiaTreno;

namespace TrenordVT
{
    //stato della circolazione su una linea, come indicato dal sito Trenord
    enum StatoLinea {LineaRegolare, LineaConCriticita, LineaConGraviCriticita, LineaStatoSconosciuto};

    //gravità di un singolo avviso
    enum GravitaAvviso {AvvisoInformativo, AvvisoAttenzione, AvvisoCritico};

    //una linea (direttrice) Trenord con il suo stato di circolazione
    struct Linea
    {
        QString codice;       //es. "S11", "RE_5"
        QString nome;         //es. "Como-Milano-Rho"
        QString direttrice;   //es. "D003"
        StatoLinea stato = LineaStatoSconosciuto;
        QString descrizioneStato; //es. "Circolazione con criticità"
    };
}

class AvvisoTrenord
{
public:
    void impostaOrario(const QDateTime& orario) {m_orario = orario;}
    QDateTime orario() const {return m_orario;}

    //nome della linea (o delle linee, separate da virgola) a cui si riferisce l'avviso
    void impostaDirettrice(const QString& direttrice) {m_direttrice = direttrice;}
    QString direttrice() const {return m_direttrice;}

    void impostaTesto(const QString& testo) {m_testo = testo;}
    QString testo() const {return m_testo;}

    void impostaGravita(TrenordVT::GravitaAvviso gravita) {m_gravita = gravita;}
    TrenordVT::GravitaAvviso gravita() const {return m_gravita;}

    //etichetta dello stato riportata dal sito (es. "Criticità", "Regolare")
    void impostaStato(const QString& stato) {m_stato = stato;}
    QString stato() const {return m_stato;}

    //prima riga significativa del testo, da usare come riassunto
    QString sommario() const;

private:
    QDateTime m_orario;
    QString m_direttrice, m_testo, m_stato;
    TrenordVT::GravitaAvviso m_gravita = TrenordVT::AvvisoInformativo;
};

class ParserTrenord : public QObject
{
    Q_OBJECT
public:
    explicit ParserTrenord(SchedaQViaggiaTreno* scheda);

    //analizza la risposta di /rest/render/shoulder-lines (lista delle linee con il loro stato).
    //Restituisce false se la risposta non contiene alcuna linea; in caso di successo aggiorna
    //linee() e listaDirettrici(), cioè i codici delle linee la cui circolazione non è regolare
    bool analizzaListaDirettrici(const QString& rispostaTN);
    QQueue<QString> listaDirettrici() const {return m_direttrici;}
    QList<TrenordVT::Linea> linee() const {return m_linee;}

    //analizza la risposta di /rest/render/line-details per la linea indicata e aggiunge ad
    //avvisi gli avvisi trovati. Restituisce false se la risposta non è una scheda di linea
    static bool analizzaDettagliLinea(const QString& rispostaTN, const QString& nomeLinea,
                                      QList<AvvisoTrenord>& avvisi);

    //unisce gli avvisi identici pubblicati su più linee e li ordina dal più recente
    static QList<AvvisoTrenord> unisciAvvisi(const QList<AvvisoTrenord>& avvisi);

private:
    QQueue<QString> m_direttrici;
    QList<TrenordVT::Linea> m_linee;
    SchedaQViaggiaTreno* m_scheda = nullptr;
};

class ModelloAvvisiTrenord : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Colonne {colOrario, colLinea, colStato, colAvviso, numeroColonne};

    explicit ModelloAvvisiTrenord(QObject* parent);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    void impostaAvvisi(const QList<AvvisoTrenord>& avvisi);
    AvvisoTrenord avviso(int riga) const {return m_avvisi.value(riga);}

 private:
    QList<AvvisoTrenord> m_avvisi;
};

#endif // PARSER_TRENORD_H
