/***************************************************************************
 *   Copyright (C) 2011 by fra74                                           *
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


#ifndef PARSER_VIAGGIATRENO_STAZIONE_H
#define PARSER_VIAGGIATRENO_STAZIONE_H

#include <QtXml>
#include <QColor>
#include <QAbstractTableModel>

#include "parser_viaggiatreno_base.h"

class SchedaQViaggiaTreno;

namespace StazioneVT
{
enum {colCategoria = 0, colNumero = 1, colStazione = 2, colBinProgrammato = 3,
      colBinReale = 4, colOrario = 5, colRitardo = 6, colUltima = colRitardo};

//classe che memorizza i dati di ogni singolo treno in arrivo/partenza
class DatiTreno
{
public:
    // restituisce la categoria del treno
    QString categoria() const {return m_categoria;}
    // imposta la categoria del treno
    void impostaCategoria(const QString& cat) { m_categoria = cat;}

    //restiuisce numero del treno
    QString numero() const { return m_numero; }
    //imposta il numero
    void impostaNumero(const QString& num) { m_numero = num; }

    //restituisce l'eventuale codice della stazione di origine del treno
    QString codiceOrigine() const {return m_codice;}
    void impostaCodiceOrigine(const QString& cod) {m_codice = cod;}

    //restituisce il nome della stazione di origine/destinazione
    QString stazione() const { return m_stazione; }
    //imposta il nome della stazione di origine destinazione
    void impostaStazione(const QString& staz) {m_stazione = staz;}

    //restituisce il binario di arrivo/partenza previsto
    QString binarioProgrammato() const {return m_binario_prog;}
    //imposta il binario di arrivo/partenza previsto
    void impostaBinarioProgrammato(const QString& bp) {m_binario_prog = bp; }

    //restituisce il binario di arrivo/partenza reale
    QString binarioReale() const { return m_binario_reale; }
    //imposta il binario di arrivo/partenza reale
    void impostaBinarioReale(const QString& br) { m_binario_reale = br; }

    //restituisce l'orario di arrivo/partenza previsto
    QString orario() const { return m_orario; }
    //imposta l'orario di arrivo/partenza previsto
    void impostaOrario(const QString& ora) { m_orario = ora; }

    //restituisce il ritardo
    QString ritardo() const { return m_ritardo; }
    //imposta il ritardo
    void impostaRitardo(const QString& rit) { m_ritardo = rit; }

  private:
    QString m_categoria, m_numero, m_stazione, m_binario_prog, m_binario_reale, m_orario, m_ritardo, m_codice;
};
};

//Questa classe si occupa del parsing della pagina di ViaggiaTreno
//con il quadro Arrivi/Partenze di una stazione

class ParserStazioneViaggiaTreno : public ParserViaggiaTrenoBase
{
    Q_OBJECT
public:
    ParserStazioneViaggiaTreno(SchedaQViaggiaTreno * scheda);

    //imposta il testo della risposta di ViaggiaTreno che dovrà essere analizzato dal parser
    void impostaRispostaVT(const QString& rispostaVT);
    //true se ViaggiaTreno segnala che la stazione non esiste
    bool stazioneNonTrovata() const;
    //true se ViaggiaTreno segnala che il nome della stazione è ambiguo
    bool nomeStazioneAmbiguo() const;
    //restituisce una QMap con una corrispodenza Nome stazione<->Codice stazione di tutte le stazioni
    //che viaggiatreno segnala come stazioni possibili vista la stringa passata come nome
    //esempio se si cerca "Monza" passerà come nomi stazioni sia "Monza" che "Monza sobborghi"
    QMap<QString, QString> listaCodiciStazioni(const QString& rispostaVT );

    //restituisce il testo della risposta di viaggiatreno che viene attualmente analizzata
    QString rispostaVT() const {return m_rispostaVT;}

    //analizza il testo della risposta di viaggiatreno e restituisce true se non sono incontrati errori sintattici
    bool analizza();

    //restituisce il nome della stazione
    QString stazione() const {return m_stazione;}

    //restituisce riga dove è stato incontrato errore
    int rigaErrore() const {return m_riga;}
    //restituisce colonna dove è stato incontrato errore
    int colErrore() const {return m_col;}
    //restituisce il messaggio di errore
    QString errore() const {return m_err;}

    //restituisce il prospetto degli arrivi, attenzione richiamare la funzione
    //solo dopo aver eseguito analizza() ed essersi assicurato che restituisca true
    QList<StazioneVT::DatiTreno> arrivi() const
    {
        return m_arrivi;
    }

    //restituisce il prospetto delle partenze, valgono gli stessi avvertimenti di arrivi()
    QList<StazioneVT::DatiTreno> partenze() const
    {
        return m_partenze;
    }
private:
    SchedaQViaggiaTreno *m_scheda;
    QString m_rispostaVT;
    int m_riga, m_col;
    QString m_err;
    QDomDocument m_docDOM;
    QString m_stazione;
    QList<StazioneVT::DatiTreno> m_arrivi, m_partenze;
};

//Classe che implementa il modello di dati che viene visualizzato in una tabella
//della scheda Stazione
// se partenze è true allora il modello rappresenta il prospetto delle partenze
//altrimenti rappresenta il prospetto degli arrivi
class ModelloStazione: public QAbstractTableModel
{
    Q_OBJECT
public:
    ModelloStazione(QWidget *parent, bool partenze);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    void aggiornaModello(const QList<StazioneVT::DatiTreno> &treni);

    QStringList categorie() const {return m_categorie;}
    QStringList stazioni() const {return m_stazioni;}

private:
    bool m_partenze;
    QStringList m_stazioni, m_categorie;
    QList<StazioneVT::DatiTreno> m_treni;
};

#endif // PARSER_VIAGGIATRENO_STAZIONE_H
