/***************************************************************************
 *   Copyright (C) 2011 by fra74                                                                                                           *
 *   francesco.b74@gmail.com                                                                                                             *
 *                                                                                                                                                                     *
 *   This program is free software; you can redistribute it and/or modify                             *
 *   it under the terms of the GNU General Public License as published by                         *
 *   the Free Software Foundation; either version 2 of the License, or                                 *
 *   (at your option) any later version.                                                                                                *
 *                                                                                                                                                                     *
 *   This program is distributed in the hope that it will be useful,                                             *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *   GNU General Public License for more details.                                                                        *
 *                                                                                                                                                                    *
 *   You should have received a copy of the GNU General Public License                           *
 *   along with this program; if not, write to the                                                                            *
 *   Free Software Foundation, Inc.,                                                                                                  *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                                            *
 ***************************************************************************/

#include "parser_viaggiatreno_treno.h"
#include "utils.h"
#include <QApplication>
#include <QPalette>

ParserTrenoViaggiaTreno::ParserTrenoViaggiaTreno(SchedaQViaggiaTreno* scheda)
{
        m_scheda = scheda;
        m_err = "";
        m_riga = m_col = -1;
    m_rispostaVTAnalizzata = "";
    m_rispostaVTDettagli = "";
    m_rispostaVTRiepilogo = "";

}

void ParserTrenoViaggiaTreno::inizializza()
{
    m_err = "";

    m_riga = m_col = -1;
}

//cerca nel testo la stringa "non valido" perché VT quando trova un numero non valido visualizza il messaggio
//"numero treno non valido"
bool ParserTrenoViaggiaTreno::trenoNonPrevisto() const
{
    return m_rispostaVTRiepilogo.contains("non valido", Qt::CaseInsensitive);
}

//cerca nel testo la stringa "option". Essa è presente quando il numero del treno è ambiguo
//e ViaggiaTreno presenta una lista di alternative
bool ParserTrenoViaggiaTreno::numeroTrenoAmbiguo() const
{
    return m_rispostaVTRiepilogo.contains("option", Qt::CaseInsensitive);
}

QMap<QString, QString> ParserTrenoViaggiaTreno::listaCodiciTreno(const QString& rispostaVT)
{
    QDomDocument docDom;
    QString msg;
    int riga, col;
    QMap<QString, QString> lista;

    //ottiene la lista di tutti gli elementi figli del tag "select"
    //ogni nodo è un elemento "option" che contiene il numero del treno seguito dall'origine
    //ad esempio "518 - NAPOLI CENTRALE" e nell'attributo "value" un codice ottenuto combinando
    //il numero del treno con il codice di origine della stazione, ad esempio "518;S09218";
    docDom.setContent(rispostaVT, &msg, &riga, &col);
    QDomElement body = docDom.documentElement().firstChildElement("body");
    QDomElement corpocentrale = body.firstChildElement("div").nextSiblingElement("div");
    QDomElement form = corpocentrale.firstChildElement("form");
    QDomElement select = form.firstChildElement("p").firstChildElement("select");
    QDomNodeList nodi = select.childNodes();

    //scorre la lista dei nodi
    //per ogni elemento "option" crea un elemento della lista, la cui chiave è il nome della stazioone
    //mentre il codice è memorizzato nella voce corrispondente alla chiame
    for (int j = 0; j < nodi.count(); j++)
        lista[nodi.at(j).toElement().text()] = nodi.at(j).toElement().attribute("value");

    return lista;

}

//cerca nel testo la stringa "cancellato" e anche "<span class="errore">"
//entrambe le stringhe sono presenti nella pagina che viene generata quando si introduce il
//numero di un treno cancellato

bool ParserTrenoViaggiaTreno::trenoSoppressoTotalmente() const
{

    return (m_rispostaVTRiepilogo.contains("cancellato") && m_rispostaVTRiepilogo.
            contains("<span class=\"errore\">"));
}
//analizza il testo della scheda con il riepilogo del treno
bool ParserTrenoViaggiaTreno::analizzaRiepilogo(TrenoVT::DatiTreno& orarioTreno)
{
    QString temp;
    QList<QDomNode> listaNodi;
    QDomDocument documentoDOM;
    int idx;

    m_rispostaVTAnalizzata = m_rispostaVTRiepilogo;

    if (!documentoDOM.setContent(m_rispostaVTAnalizzata, &m_err, &m_riga, &m_col))
        return false;

    //inizia l'analisi dell'albero DOM
    //rintraccia il tag body
    QDomElement body = documentoDOM.documentElement().firstChildElement("body");
    //rintraccia il tag H1 che contiene categoria e numero treno
    temp = body.firstChildElement("h1").text();
    orarioTreno.impostaNumeroTreno(temp);
    orarioTreno.impostaCategoriaTreno(temp.section(" ", 0, 0));

    //ottiene una lista di tutti gli elementi div
    QDomNodeList elementiDiv = body.elementsByTagName("div");

    QDomElement div;

    if (m_rispostaVTAnalizzata.contains("arrivato"))
    {
        orarioTreno.impostaStatoTreno(TrenoVT::TrenoArrivato);
    }

    if (m_rispostaVTAnalizzata.contains("ancora partito"))
    {
        orarioTreno.impostaStatoTreno(TrenoVT::TrenoNonPartito);
    }

    if (m_rispostaVTAnalizzata.contains("viaggia "))
    {
        orarioTreno.impostaStatoTreno(TrenoVT::TrenoInViaggio);
    }

    //rintraccia l'elemento DIV con gli eventuali provvedimenti di limitazione/soppressione
    for (int i = 0; i < elementiDiv.count(); i++)
        if (elementiDiv.at(i).toElement().attribute("style").contains("B02D2D"))
            orarioTreno.impostaDato(TrenoVT::dtProvvedimenti, elementiDiv.at(i).toElement().text());

    //rintraccia l'elemento DIV con i dati della stazione di partenza
    for (int i = 0; i < elementiDiv.count(); i++)
            if (elementiDiv.at(i).toElement().text().contains("Partenza"))
                div = elementiDiv.at(i).toElement();

    //estrai i dati sulla stazione di partenza dall'elemento appena ottenuto
    orarioTreno.impostaDato(TrenoVT::dtPartenza, div.firstChildElement("h2").text());
    orarioTreno.impostaDato(TrenoVT::dtOrarioPartenzaProgrammato, div.firstChildElement("p").firstChildElement("strong").text());
    orarioTreno.impostaDato(TrenoVT::dtOrarioPartenzaReale, div.firstChildElement("p").nextSiblingElement("p")
                              .firstChildElement("strong").text());

    //ottiene la lista di tutti i nodi di tipo testo
    for(QDomNode n = div.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomText t = n.toText();
        if (!t.isNull())
            listaNodi.append(n);
    }

    //se la lista dei nodi è vuota allora la pagina non è completa, per evitare
    //crash ritorna false segnalando che c'è stato un errore inaspettato
    if (listaNodi.isEmpty())
        return false;

    orarioTreno.impostaDato(TrenoVT::dtBinarioPartenzaProgrammato, listaNodi.at(1).toText().data());
    if (listaNodi.count() == 4)
        orarioTreno.impostaDato(TrenoVT::dtBinarioPartenzaReale, listaNodi.at(3).toText().data());
    else
        orarioTreno.impostaDato(TrenoVT::dtBinarioPartenzaReale, div.firstChildElement("strong").text());

    //rintraccia l'elemento DIV con i dati della stazione di arrivo
    for (int i = 0; i < elementiDiv.count(); i++)
        if (elementiDiv.at(i).toElement().text().contains("Arrivo"))
            div = elementiDiv.at(i).toElement();

    //estrai i dati sulla stazione di arrivo dall'elemento appena ottenuto
    orarioTreno.impostaDato(TrenoVT::dtArrivo, div.firstChildElement("h2").text());
    orarioTreno.impostaDato(TrenoVT::dtOrarioArrivoProgrammato, div.firstChildElement("p").firstChildElement("strong").text());
    orarioTreno.impostaDato(TrenoVT::dtOrarioArrivo, div.firstChildElement("p").nextSiblingElement("p")
                              .firstChildElement("strong").text());

    //ottiene la lista di tutti i nodi di tipo testo
    listaNodi.clear();
    for(QDomNode n = div.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomText t = n.toText();
        if (!t.isNull())
            listaNodi.append(n);
    }
    orarioTreno.impostaDato(TrenoVT::dtBinarioArrivoProgrammato, listaNodi.at(1).toText().data());
    if (listaNodi.count() == 4)
        orarioTreno.impostaDato(TrenoVT::dtBinarioArrivoReale, listaNodi.at(3).toText().data());
    else
        orarioTreno.impostaDato(TrenoVT::dtBinarioArrivoReale, div.firstChildElement("strong").text());


    //estra il ritardo attuale per un treno in viaggio
    if (m_rispostaVTAnalizzata.contains("viaggia "))
    {
        //rintraccia l'elemento DIV con il ritardo attuale
        idx = -1;
        for (int i = 0; i < elementiDiv.count(); i++)
            if (elementiDiv.at(i).toElement().text().contains("Il treno viaggia"))
                idx = i;


        if (idx != -1)
        {
            QString ritardo, transito;
            div = elementiDiv.at(idx).toElement();
            temp = div.firstChildElement("strong").text();
            idx = temp.indexOf("Ultimo");
            if (idx == -1)
            {
                ritardo = temp;
                transito = "";
            }
            else
            {
                ritardo = temp.left(idx);
                transito = temp.mid(idx);
            }

            //esistono i dati sull'ora e la località dell'ultimo transito
            //TODO qui dovrà essere aggiunto il codice per l'aggiunta di questo transito alla lista di tutti i transiti
            if (transito != "")
            {
                orarioTreno.impostaDato(TrenoVT::dtOrarioTransito, transito.section(' ', -2, -2));
                orarioTreno.impostaDato(TrenoVT::dtUltimoRilevamento, transito.section(' ', 3, -5));
            }

            if (ritardo.contains("orario"))
                orarioTreno.impostaDato(TrenoVT::dtRitardoTransito, QString::fromUtf8("in orario"));
            else
            {
                temp = ritardo.section(' ', 5, 5);
                if (temp == "1")
                    orarioTreno.impostaDato(TrenoVT::dtRitardoTransito, QString::fromUtf8("1 minuto in %1").arg(ritardo.section(' ', 8, 8)));
                else
                    orarioTreno.impostaDato(TrenoVT::dtRitardoTransito, QString::fromUtf8("%1 minuti in %2").arg(temp).arg(ritardo.section(' ', 8, 8)));
            }
        }
    }

    return true;
}

//analizza il testo della scheda con i dettagli del treno
bool ParserTrenoViaggiaTreno::analizzaDettagli(TrenoVT::DatiTreno& orarioTreno)
{
    QList<QDomNode> listaNodi;
    QDomDocument documentoDOM;

    m_rispostaVTAnalizzata = m_rispostaVTDettagli;

    if (!documentoDOM.setContent(m_rispostaVTAnalizzata, &m_err, &m_riga, &m_col))
        return false;

    QDomElement body = documentoDOM.documentElement().firstChildElement("body");
    //ottiene la lista degli elementi div
    QDomNodeList elementiDiv = body.elementsByTagName("div");
    //i dati delle fermate si trovano dal secondo al penultimo elemento div
    //ma siccome abbiamo già scritto i dati per la stazione di partenza e di arrivo
    //ci interessano solo gli elementi dal terzo al terz'ultimo
    for (int i = 2; i < elementiDiv.count()-2; i++)
    {
        TrenoVT::Fermata *fermata = new TrenoVT::Fermata();
        QDomElement div = elementiDiv.at(i).toElement();
        //verifica se la fermata è già stata effettuata o no, sfruttando l'attributo class
        if (div.attribute("class") == "corpocentrale")
            fermata->impostaEffetuata(false);
        else
            fermata->impostaEffetuata(true);
        //verifica se per caso la fermata non sia stata soppressa
        if (div.text().contains("soppressa"))
            fermata->impostaSoppressa(true);
        //imposta il nome della fermata
        fermata->impostaNomeFermata(div.firstChildElement("h2").text());
        //imposta l'ora di arriva programmata ed effettiva
        fermata->impostaOraArrivoProgrammata(div.firstChildElement("p").firstChildElement("strong").text());
        if (fermata->effettuata())
            fermata->impostaOraArrivoReale(div.firstChildElement("p").nextSiblingElement("p").firstChildElement("strong").text());
        else
            fermata->impostaOraArrivoStimata(div.firstChildElement("p").nextSiblingElement("p").firstChildElement("strong").text());
        listaNodi.clear();
        for(QDomNode n = div.firstChild(); !n.isNull(); n = n.nextSibling())
        {
            QDomText t = n.toText();
            if (!t.isNull())
                listaNodi.append(n);

        }
        //se ci sono 4 elementi nella lista allora né binario previsto né reale sono scritti in grassetto
        if ( listaNodi.count() == 4)
        {
            fermata->impostaBinarioProgrammato(listaNodi.at(1).toText().data().simplified());
            fermata->impostaBinarioReale(listaNodi.at(3).toText().data().simplified());
        }

        //ce ne sono 3 allora le possibilità sono varie
        //se la fermata è stata effettuata allora il binario reale è in grassetto
        else
            if (fermata->effettuata())
            {
                fermata->impostaBinarioProgrammato(listaNodi.at(1).toText().data().simplified());
                fermata->impostaBinarioReale(div.firstChildElement("strong").text().simplified());
            }
        //oppure la fermata non è stata effettuata
        else
            //è indicato in grassetto il binario reale (evento raro ma succede)
            if (listaNodi.at(2).toText().data().contains("Binario"))
            {
                fermata->impostaBinarioProgrammato(listaNodi.at(1).toText().data().simplified());
                fermata->impostaBinarioReale(div.firstChildElement("strong").text().simplified());
            }
            //è indicato in grassetto il binario programmato
            else
            {
                fermata->impostaBinarioProgrammato(div.firstChildElement("strong").text().simplified());
                fermata->impostaBinarioReale(listaNodi.at(2).toText().data().simplified());
            }

    orarioTreno.aggiungiFermata(fermata);
    }

    return true;
}

void TrenoVT::DatiTreno::inizializza()
{
        m_statoTreno = DatiSconosciuti;

        m_dati.clear();
        m_fermate.clear();

}

TrenoVT::Fermata::Fermata()
{
        m_effettuata = false;
        m_soppressa = false;

        m_fermata = "";
        m_binProgrammato = "";
        m_binReale = "";
        m_oraArrivoProgrammata = "";
        m_oraArrivoReale = "";
        m_oraArrivoStimata = "";
}

TrenoVT::Transito::Transito()
{
        m_ritardo = 0;
        m_localita = "";
        m_orarioTransito = "";
}

ListaVT::DatiTreno::DatiTreno(const QString& numero)
{
    m_numero = numero;
    m_codice = "";

    m_stato = TrenoVT::DatiSconosciuti;
}

//cancella tutti i dati del treno tranne numero
void ListaVT::DatiTreno::cancella()
{
    m_dati.clear();
    m_stato = TrenoVT::DatiSconosciuti;
}

//restituisce una stringa con lo stato del treno
QString ListaVT::DatiTreno::stringaStatoTreno() const
{
    QString stringa;
    switch(m_stato)
    {
    case TrenoVT::DatiSconosciuti: stringa = QString::fromUtf8("Stato sconosciuto"); break;
    case TrenoVT::TrenoNonPrevisto: stringa = QString::fromUtf8("Non previsto oggi"); break;
    case TrenoVT::TrenoNonPartito: stringa = QString::fromUtf8("Non ancora partito"); break;
    case TrenoVT::TrenoInViaggio: stringa = QString::fromUtf8("In viaggio"); break;
    case TrenoVT::TrenoArrivato: stringa = QString::fromUtf8("Arrivato"); break;
    case TrenoVT::TrenoCancellato: stringa = QString::fromUtf8("Soppresso"); break;
    }

    return stringa;
}

bool ParserTrenoViaggiaTreno::analizzaRiepilogoPerLista(ListaVT::DatiTreno &treno)
{

    QDomDocument documentoDOM;
    int idx;
    QString temp;

    treno.cancella();

    //analizza il testo della risposta di viaggiatreno
    m_rispostaVTAnalizzata = m_rispostaVTRiepilogo;
    if (!documentoDOM.setContent(m_rispostaVTAnalizzata, &m_err, &m_riga, &m_col ))
        return false;

    //inizia l'analisi dell'albero DOM
    //rintraccia il tag body
    QDomElement body = documentoDOM.documentElement().firstChildElement("body");
    //rintraccia il tag H1 che contiene categoria e numero treno
    temp = body.firstChildElement("h1").text();
    treno.impostaDato(ListaVT::dtCategoria, temp.section(' ', 0, 0));
    //ottiene una lista di tutti gli elementi div
    QDomNodeList elementiDiv = body.elementsByTagName("div");

    QDomElement div;
    //imposta lo stato del treno
    if (m_rispostaVTAnalizzata.contains("ancora partito"))
        treno.impostaStatoTreno(TrenoVT::TrenoNonPartito);

    if (m_rispostaVTAnalizzata.contains("viaggia "))
        treno.impostaStatoTreno(TrenoVT::TrenoInViaggio);

    if (m_rispostaVTAnalizzata.contains("arrivato"))
        treno.impostaStatoTreno(TrenoVT::TrenoArrivato);


    //rintraccia l'elemento DIV con i dati della stazione di partenza
        for (int i = 0; i < elementiDiv.count(); i++)
            if (elementiDiv.at(i).toElement().text().contains("Partenza"))
                div = elementiDiv.at(i).toElement();

        //estrai i dati sulla stazione di partenza dall'elemento appena ottenuto
        treno.impostaDato(ListaVT::dtOrigine, div.firstChildElement("h2").text());
        treno.impostaDato(ListaVT::dtPartenzaProgrammata, div.firstChildElement("p").firstChildElement("strong").text());
        //non cercare l'orario di partenza effettivo se il treno risulta non ancora partito
        if (!m_rispostaVTAnalizzata.contains("ancora partito"))
            treno.impostaDato(ListaVT::dtPartenzaEffettiva, div.firstChildElement("p").nextSiblingElement("p")
                               .firstChildElement("strong").text());

        //rintraccia l'elemento DIV con i dati della stazione di arrivo
        for (int i = 0; i < elementiDiv.count(); i++)
            if (elementiDiv.at(i).toElement().text().contains("Arrivo"))
                div = elementiDiv.at(i).toElement();

        //estrai i dati sulla stazione di arrivo dall'elemento appena ottenuto
        treno.impostaDato(ListaVT::dtDestinazione, div.firstChildElement("h2").text());
        treno.impostaDato(ListaVT::dtArrivoProgrammato, div.firstChildElement("p").firstChildElement("strong").text());
        //cerca l'orario di arrivo effettivo solo se il treno risulta già arrivato
        if (m_rispostaVTAnalizzata.contains("arrivato"))
            treno.impostaDato(ListaVT::dtArrivoEffettivo, div.firstChildElement("p").nextSiblingElement("p")
                               .firstChildElement("strong").text());

        //cerca solo per i treni ancora in viaggio il dato sull'ultima fermata effettuata
        if (m_rispostaVTAnalizzata.contains("viaggia "))
        {
            idx = -1;
            for (int i = 0; i < elementiDiv.count(); i++)
                if (elementiDiv.at(i).toElement().text().contains("Ultima fermata effettuata"))
                    idx = i+1;
            //è stata trovato? allora estrai i dati
            if (idx != -1)
            {
                div = elementiDiv.at(idx).toElement();

                treno.impostaDato(ListaVT::dtUltimaFermata, div.firstChildElement("h2").text());
                treno.impostaDato(ListaVT::dtOrarioFermataProgrammato, div.firstChildElement("p").firstChildElement("strong").text());
                treno.impostaDato(ListaVT::dtOrarioFermataEffettivo, div.firstChildElement("p").nextSiblingElement("p")
                                   .firstChildElement("strong").text());
            }
        }

        //cerca il ritardo in arrivo per un treno già arrivato
        if (m_rispostaVTAnalizzata.contains("arrivato"))
        {
            idx = -1;
            for (int i = 0; i < elementiDiv.count(); i++)
                if (elementiDiv.at(i).toElement().text().contains("Il treno e' arrivato"))
                    idx = i;
            if (idx != -1)
            {
                div = elementiDiv.at(idx).toElement();
                //il treno è arrivato in orario, non c'è ragione di estrarre il ritardo
                if (div.text().contains("orario"))
                    treno.impostaDato(ListaVT::dtRitardoTransito, QString::fromUtf8("In orario"));
                else
                {
                    QString minuti = div.text().section(' ', 6, 6);
                    QString ritOAnticipo = div.text().section(' ', 9, 9);
                    if (minuti == "1")
                        temp = QString::fromUtf8("1 minuto in %1").arg(ritOAnticipo);
                    else
                        temp = QString::fromUtf8("%1 minuti in %2").arg(minuti).arg(ritOAnticipo);

                    treno.impostaDato(ListaVT::dtRitardoTransito, temp);
                }
            }
        }

        //cerca il ritardo per i treni ancora in viaggio
        if (m_rispostaVTAnalizzata.contains("viaggia "))
        {
            idx = -1;
            for (int i = 0; i < elementiDiv.count(); i++)
                if (elementiDiv.at(i).toElement().text().contains("Il treno viaggia"))
                    idx = i;


            if (idx != -1)
            {
                QString ritardo, transito;
                div = elementiDiv.at(idx).toElement();
                temp = div.firstChildElement("strong").text();
                idx = temp.indexOf("Ultimo");
                if (idx == -1)
                {
                    ritardo = temp;
                    transito = "";
                }
                else
                {
                    ritardo = temp.left(idx);
                    transito = temp.mid(idx);
                }

                //esistono i dati sull'ora e la località dell'ultimo transito
                if (transito != "")
                {
                    treno.impostaDato(ListaVT::dtOrarioTransito, transito.section(' ', -2, -2));
                    treno.impostaDato(ListaVT::dtUltimoRilevamento, transito.section(' ', 3, -5));
                }

                if (ritardo.contains("orario"))
                    treno.impostaDato(ListaVT::dtRitardoTransito, QString::fromUtf8("in orario"));
                else
                {
                    temp = ritardo.section(' ', 5, 5);
                    if (temp == "1")
                        treno.impostaDato(ListaVT::dtRitardoTransito, QString::fromUtf8("1 minuto in %1").arg(ritardo.section(' ', 8, 8)));
                    else
                        treno.impostaDato(ListaVT::dtRitardoTransito, QString::fromUtf8("%1 minuti in %2").arg(temp).arg(ritardo.section(' ', 8, 8)));
                }
            }
        }
        return true;
}
