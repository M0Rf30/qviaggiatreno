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


#ifndef PARSER_VIAGGIATRENO_TRENO_H
#define PARSER_VIAGGIATRENO_TRENO_H

#include <QtXml>
#include <QColor>

#include "parser_viaggiatreno_base.h"

class SchedaQViaggiaTreno;

namespace TrenoVT
{
enum StatoTreno {DatiSconosciuti, TrenoCancellato, TrenoNonPrevisto, TrenoNonPartito, TrenoInViaggio, TrenoArrivato};
enum Dati {dtPartenza, dtOrarioPartenzaProgrammato, dtOrarioPartenzaReale, dtBinarioPartenzaProgrammato, dtBinarioPartenzaReale,
           dtArrivo, dtOrarioArrivoProgrammato, dtOrarioArrivo, dtBinarioArrivoProgrammato, dtBinarioArrivoReale,
           dtOrarioTransito, dtRitardoTransito, dtUltimoRilevamento, dtProvvedimenti };

enum {colFermata = 0, colEffettuata, colBinarioProgrammato, colBinarioReale,
      colArrivoProgrammato, colArrivoStimato, colArrivoReale, colUltima=colArrivoReale};

enum {colLocalita = 0, colOrarioTransito, colRitardo};

class Fermata
{
public:
    Fermata();

    QString nomeFermata() const {return m_fermata;}
    QString binarioProgrammato() const {return m_binProgrammato;}
    QString binarioReale() const {return m_binReale;}
    QString oraArrivoProgrammata() const {return m_oraArrivoProgrammata;}
    QString oraArrivoStimata() const {return m_oraArrivoStimata;}
    QString oraArrivoReale() const {return m_oraArrivoReale;}
    bool effettuata() const {return m_effettuata;}
    bool soppressa() const {return m_soppressa;}

    void impostaNomeFermata(const QString& fermata) {m_fermata = fermata;}
    void impostaBinarioProgrammato(const QString& binarioProgrammato) {m_binProgrammato = binarioProgrammato;}
    void impostaBinarioReale(const QString& binarioReale) {m_binReale = binarioReale;}
    void impostaOraArrivoProgrammata(const QString& oraArrivoProgrammata) {m_oraArrivoProgrammata = oraArrivoProgrammata;}
    void impostaOraArrivoStimata(const QString& oraArrivoStimata) {m_oraArrivoStimata = oraArrivoStimata;}
    void impostaOraArrivoReale(const QString& oraArrivoReale) {m_oraArrivoReale = oraArrivoReale;}
    void impostaEffetuata(bool eff) {m_effettuata = eff;}
    void impostaSoppressa(bool sopp) {m_soppressa = sopp; m_effettuata = false;}

private:
    QString m_fermata;
    QString m_binProgrammato, m_binReale;
    QString m_oraArrivoProgrammata, m_oraArrivoStimata, m_oraArrivoReale;
    bool m_effettuata, m_soppressa;
};

class Transito
{
public:
    Transito();

    QString nomeLocalita() const {return m_localita;}
    QString orarioTransito() const {return m_orarioTransito;}
    int ritardo() const {return m_ritardo;}

    void impostaLocalita(const QString& localita) {m_localita = localita;}
    void impostaOrarioTransito(const QString& orarioTransito) {m_orarioTransito = orarioTransito;}
    void impostaRitardo(int ritardo) {m_ritardo = ritardo;}


private:
    QString m_localita;
    QString m_orarioTransito;
    int m_ritardo;
};

class DatiTreno
{
public:
    DatiTreno() {m_statoTreno = DatiSconosciuti;}

    void impostaStatoTreno(StatoTreno stato) {m_statoTreno = stato;}
    StatoTreno statoTreno() const {return m_statoTreno;}

    void inizializza();

    QString categoriaTreno() const {return m_categoria;}
    void impostaCategoriaTreno(const QString& cat) {m_categoria = cat;}

    QString numeroTreno() const {return m_numero; }
    void impostaNumeroTreno(const QString& numero) {m_numero = numero; }

    void impostaDato(Dati tipoDato, const QString& valore) {m_dati[tipoDato] = valore; }
    QString dato(Dati tipoDato) const {return m_dati[tipoDato]; }

    QList<Fermata*> fermate() const {return m_fermate;}
    void aggiungiFermata(Fermata* f) {m_fermate.append(f);}

private:
    StatoTreno m_statoTreno;
    //il numero treno contiene *anche* la categoria
    QString m_numero;
    QString m_categoria;
    QMap<int, QString> m_dati;
    QList<Fermata*> m_fermate;

};
};

namespace ListaVT
{
enum Dati {dtCategoria, dtOrigine, dtDestinazione, dtUltimaFermata, dtOrarioFermataProgrammato, dtOrarioFermataEffettivo,
           dtUltimoRilevamento, dtOrarioTransito, dtRitardoTransito, dtPartenzaProgrammata, dtPartenzaEffettiva,
           dtArrivoProgrammato, dtArrivoEffettivo};

enum {rigaNumero = 0, rigaCategoria, rigaStato, rigaOrigine, rigaPartenzaProgrammata, rigaPartenzaEffettiva,
      rigaDestinazione, rigaArrivoProgrammato, rigaArrivoEffettivo, rigaUltimaFermata, rigaOrarioFermataProgrammato,
      rigaOrarioFermataEffettivo, rigaUltimoRilevamento, rigaOrarioTransito, rigaRitardoTransito,  rigaUltima = rigaRitardoTransito};

//questa classe memorizza i dati di ogni singolo treno della lista di treni
class DatiTreno
{
public:
    DatiTreno(const QString& numero);

    void cancella();

    void impostaDato(Dati tipoDato, const QString& valore) {m_dati[tipoDato] = valore; }
    QString dato(Dati tipoDato) const {return m_dati[tipoDato]; }

    TrenoVT::StatoTreno statoTreno() const {return m_stato;}
    void impostaStatoTreno(TrenoVT::StatoTreno stato) {m_stato = stato;}
    QString stringaStatoTreno() const;

    QString numero() const {return m_numero;}
    void impostaNumero(const QString& numero) {m_numero = numero;}

    QString codiceOrigine() const {return m_codice;}
    void impostaCodiceOrigine(const QString& codice) {m_codice = codice;}


private:
    QString m_numero, m_codice;
    QMap<Dati, QString> m_dati;

    TrenoVT::StatoTreno m_stato;
};
};


class ParserTrenoViaggiaTreno: public ParserViaggiaTrenoBase
{
    Q_OBJECT
public:
    ParserTrenoViaggiaTreno(SchedaQViaggiaTreno *scheda);

    void inizializza();

    //imposta il testo della risposta di viaggiatreno contenente il riepilogo del treno
    //che dovrà essere analizzata dal parser
    void impostaRispostaVTRiepilogo(const QString& rispostaVT) {m_rispostaVTRiepilogo = rispostaVT;}
    //imposta il testo della risposta di viaggiatreno contenente i dettagli del treno
    //che dovrà essere analizzata dal parser
    void impostaRispostaVTDettagli(const QString& rispostaVT) {m_rispostaVTDettagli = rispostaVT;}

    //analizza il testo della risposta di viaggiatreno e restituisce true se non sono incontrati errori sintattici
    bool analizzaRiepilogo(TrenoVT::DatiTreno& orarioTreno);
    bool analizzaDettagli(TrenoVT::DatiTreno& orarioTreno);
    //analizza il riepilogo ma estrai dati necessari alla scheda Lista Treni
    bool analizzaRiepilogoPerLista(ListaVT::DatiTreno &treno);

    //restituisce true se il numero treno non è stato trovato
    bool trenoNonPrevisto() const;
    //restituisce true se il treno è stato soppresso totalmente
    bool trenoSoppressoTotalmente() const;
    //restituisce true se il numero del treno è ambiguo
    //cioè se esistono più treni con lo stesso numero
    bool numeroTrenoAmbiguo() const;
    //restituisce una QMap con una corrispodenza Numero/origine treno<->Codice treno
    //utile quando ViaggiaTreno segnala che il numero del treno è ambiguo e produce una lista con numero treno
    //seguito da origine del treno
    //esempio se si cerca "518" ViaggiaTreno restituirà
    //"518 - NAPOLI CENTRALE"
    //"518 - SARONNO"
    QMap<QString, QString> listaCodiciTreno(const QString& rispostaVT );

    //restituisce riga dove è stato incontrato errore
    int rigaErrore() const {return m_riga;}
    //restituisce colonna dove è stato incontrato errore
    int colErrore() const {return m_col;}
    //restituisce il messaggio di errore
    QString errore() const {return m_err;}
    //restituisce il testo della risposta di viaggiatreno con il riepilogo
    QString rispostaVTRiepilogo() const {return m_rispostaVTRiepilogo;}
    // restituisce il testo della risposta di viaggiatreno con i dettagli
    QString rispostaVTDettagli() const {return m_rispostaVTDettagli;}
    //restituisce il testo dell'ultima risposta di viaggiatreno analizzata
    QString rispostaVTAnalizzata() const {return m_rispostaVTAnalizzata;}


private:
    SchedaQViaggiaTreno *m_scheda;
    QString m_rispostaVTDettagli, m_rispostaVTRiepilogo, m_rispostaVTAnalizzata;
    int m_riga, m_col;
    QString m_err;
};


#endif
