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

#include "parser_viaggiatreno_stazione.h"
#include "utils.h"
#include <QApplication>
#include <QPalette>



ParserStazioneViaggiaTreno::ParserStazioneViaggiaTreno(SchedaQViaggiaTreno* scheda)
{
    m_scheda = scheda;
    m_err = "";
    m_riga = m_col = -1;

}

//imposta il testo della risposta di viaggiatreno che dovrà essere analizzata dal parser
//reimposta anche tutte le variabili utilizzate
void ParserStazioneViaggiaTreno::impostaRispostaVT(const QString& rispostaVT)
{
    m_rispostaVT = rispostaVT;
    m_err = "";
    m_riga = m_col = -1;
    m_docDOM.clear();
}

//cerca nel testo la stringa "non trovata", visto che quando VT non trova una stazione visualizza
//una casella di testo con di fianco la scritta "località non trovata"
//oppure cerca la stringa "inserire almeno" visualizzata quando si inserisce un nome troppo breve
bool ParserStazioneViaggiaTreno::stazioneNonTrovata() const
{
    return (m_rispostaVT.contains("non trovata", Qt::CaseInsensitive) ||
            (m_rispostaVT.contains("Inserire almeno", Qt::CaseInsensitive)));
}

//cerca nel testo la stringa "option", se è presente vuol dire che viaggiatreno ha preparato un form
//per la selezione del nome della stazione da una lista, in quanto il nome introdotto è ambiguo
bool ParserStazioneViaggiaTreno::nomeStazioneAmbiguo() const
{
    return (m_rispostaVT.contains("option", Qt::CaseInsensitive));
}

QMap<QString, QString> ParserStazioneViaggiaTreno::listaCodiciStazioni(const QString& rispostaVT)
{
    QDomDocument docDom;
    QString msg;
    int riga, col;
    QMap<QString, QString> lista;

    //ottiene la lista di tutti gli elementi figli del tag "select"
    //ogni nodo è un elemento "option" che contiene il nome della stazione e nell'attributo "value" il codice della stazione
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

bool ParserStazioneViaggiaTreno::analizza()
{
    if (!m_docDOM.setContent(m_rispostaVT, &m_err, &m_riga, &m_col))
        return false;

    //recupera l'elemento body
    QDomElement body = m_docDOM.documentElement().firstChildElement("body");

    //visto che il parser non ha rilevato problemi si fa l'assunzione che la pagina inviata da viaggiatreno sia corretta
    //quindi si suppone che l'elemento body esista
    //in future versioni si implementerà probabilmente un algoritmo più robusto

    //ottiene la lista di tutti i nodi figli
    QDomNodeList nodi = body.childNodes();


    //recupera nome della stazione dal documento DOM
    QDomElement h1 = body.firstChildElement("h1");
    m_stazione = h1.text().section(' ', 2);

    int inizioArrivi, fineArrivi, inizioPartenze, finePartenze;
    inizioArrivi = fineArrivi = inizioPartenze = finePartenze = -1;
    //scorre la lista alla ricerca di tutti i commenti
    for (int idx = 0; idx < nodi.length(); idx++)
    {
        if (nodi.at(idx).isComment())
        {
            if (nodi.at(idx).toComment().data().contains("partenze", Qt::CaseInsensitive))
            {
                inizioPartenze = idx+1;
                //individua il commento successivo che identifica la fine del blocco di arrivi o partenze
                for (int j = idx +1; j < nodi.length(); j++)
                {
                    if (nodi.at(j).isComment())
                    {

                        finePartenze = j-1;
                        idx = j;
                        break;
                    }
                }
            }

            if (nodi.at(idx).toComment().data().contains("arrivi", Qt::CaseInsensitive))
            {
                inizioArrivi = idx+1;
                //individua il commento successivo che identifica la fine del blocco di arrivi o partenze
                for (int j = idx +1; j < nodi.length(); j++)
                {
                    if (nodi.at(j).isComment())
                    {
                        fineArrivi = j-1;
                        idx = j;
                        break;
                    }
                }
            }
        }
    }

    //pulisci le liste
    m_arrivi.clear();
    m_partenze.clear();

    //itera lungo la lista dei nodi, partendo dal primo elemento che rappresenta un treno in partenza
    //e finendo con l'ultimo elemento. I rispettivi indici sono memorizzati in idxInizioPartenze, idxFinePartenze
    for (int j=inizioPartenze, i=0 ; j <= finePartenze; j++,i++)
    {
        StazioneVT::DatiTreno treno;
        //recupera l'elemento dom corrente
        QDomElement elemento = nodi.at(j).toElement();

        //categoria e numero treno sono memorizzati in un elemento "h2", recuperiamolo
        QDomElement elementoh2 = elemento.firstChildElement("h2").toElement();

        //recupera dall'elemento h2 il testo, e lo separa in due stringhe... la prima conterrà la categoria
        //...la seconda il numero
        QStringList catNumeroTreno = elementoh2.text().split(" ");

        //copia le rispettive stringhe nella variabile che memorizza i dati del treno
        treno.impostaCategoria(catNumeroTreno[0]);
        treno.impostaNumero(catNumeroTreno[1]);

        //ricava dai due elementi "<strong>" nome della stazione di origine/destinazione
        //e l'orario teorico di arrivo/partenza
        QDomElement bloccotreno = elemento.firstChildElement("div");
        QDomElement stazione = bloccotreno.firstChildElement("strong");

        QDomElement orario = stazione.nextSiblingElement("strong");
        //TODO: test
        //treno.impostaStazione(sostituisciNomeStazione(stazione.text()));
        treno.impostaStazione(stazione.text());
        treno.impostaOrario(orario.text());

        QString datitreno = bloccotreno.text();

        QStringList lista = datitreno.split(" ", QString::SkipEmptyParts);

        QStringListIterator it(lista);

        //cerca la stringa "Previsto:"
        it.findNext("Previsto:");
        QString binario = it.next();
        if (binario == "--")
            binario = QString::fromUtf8("Sconosciuto");
        else
        {
            //la string successiva o è "Binario Reale:" o fa ancora parte del numero binario
            while (true)
            {
                QString next = it.next();
                if (next != "Binario")
                    binario += QString(" %1").arg(next);
                else
                    break;
            }

        }

        treno.impostaBinarioProgrammato(binario);

        //riporta l'iteratore in cima alla lista
        it.toFront();
        //cerca la stringa "Reale:"
        it.findNext("Reale:");
        binario = it.next();
        if (binario == "--")
            binario = QString::fromUtf8("Sconosciuto");
        else
        {
            //la stringa successiva o è "in" ("in orario") o è "ritardo" (ritardo x minuti)
            //oppure fa ancora parte del nome del binario
            while (true)
            {
                QString next = it.next();
                if ((next != "in") && (next != "ritardo"))
                    binario += QString(" %1").arg(next);
                else
                    break;
            }
        }

        treno.impostaBinarioReale(binario);

        //individua se il treno è in orario o in ritardo
        it.toFront();

        if (it.findNext("orario"))
            treno.impostaRitardo("0");
        else
        {
            //il treno non è indicato in orario (l'indicazione vale anche per treni in anticipo
            //quindi è in ritardo
            //il numero di minuti di ritardo è nel penultimo elemento della lista
            it.toBack();
            it.previous();
            treno.impostaRitardo(QString::fromUtf8("+%1").arg(it.previous()));
        }

        //rintraccia dal link per i dettagli del treno il codice della stazione di origine
        QString href = elemento.firstChildElement("a").attribute("href", "");
        QString cod = href.section("&",1,1).section("=",1,1);

        if (!cod.isEmpty())
        {
            treno.impostaCodiceOrigine(cod);
            //aggiunge il treno solo se ha un codice stazione di origine, questo per evitare duplicati che capitano con le S LeNord
            m_partenze.append(treno);
        }

    }

    //ripete la procedura anche per gli arrivi
    for (int j=inizioArrivi, i=0 ; j <= fineArrivi; j++,i++)
    {
        StazioneVT::DatiTreno treno;
        //recupera l'elemento dom corrente
        QDomElement elemento = nodi.at(j).toElement();

        //categoria e numero treno sono memorizzati in un elemento "h2", recuperiamolo
        QDomElement elementoh2 = elemento.firstChildElement("h2").toElement();

        //recupera dall'elemento h2 il testo, e lo separa in due stringhe... la prima conterrà la categoria
        //...la seconda il numero
        QStringList catNumeroTreno = elementoh2.text().split(" ");

        //copia le rispettive stringhe nella variabile che memorizza i dati del treno
        treno.impostaCategoria(catNumeroTreno[0]);
        treno.impostaNumero(catNumeroTreno[1]);

        //ricava dai due elementi "<strong>" nome della stazione di origine/destinazione
        //e l'orario teorico di arrivo/partenza
        QDomElement bloccotreno = elemento.firstChildElement("div");
        QDomElement stazione = bloccotreno.firstChildElement("strong");

        QDomElement orario = stazione.nextSiblingElement("strong");
        //TODO: test
        //treno.impostaStazione(sostituisciNomeStazione(stazione.text()));
        treno.impostaStazione(stazione.text());
         treno.impostaOrario(orario.text());

        QString datitreno = bloccotreno.text();

        QStringList lista = datitreno.split(" ", QString::SkipEmptyParts);

        QStringListIterator it(lista);

        //cerca la stringa "Previsto:"
        it.findNext("Previsto:");
        QString binario = it.next();
        if (binario == "--")
            binario = QString::fromUtf8("Sconosciuto");
        else
        {
            //la string successiva o è "Binario Reale:" o fa ancora parte del numero binario
            while (true)
            {
                QString next = it.next();
                if (next != "Binario")
                    binario += QString(" %1").arg(next);
                else
                    break;
            }

        }

        treno.impostaBinarioProgrammato(binario);

        //riporta l'iteratore in cima alla lista
        it.toFront();
        //cerca la stringa "Reale:"
        it.findNext("Reale:");
        binario = it.next();
        if (binario == "--")
            binario = QString::fromUtf8("Sconosciuto");
        else
        {
            //la stringa successiva o è "in" ("in orario") o è "ritardo" (ritardo x minuti)
            //oppure fa ancora parte del nome del binario
            while (true)
            {
                QString next = it.next();
                if ((next != "in") && (next != "ritardo"))
                    binario += QString(" %1").arg(next);
                else
                    break;
            }
        }

        treno.impostaBinarioReale(binario);

        //individua se il treno è in orario o in ritardo
        it.toFront();

        if (it.findNext("orario"))
            treno.impostaRitardo("0");
        else
        {
            //il treno non è indicato in orario (l'indicazione vale anche per treni in anticipo
            //quindi è in ritardo
            //il numero di minuti di ritardo è nell'ultimo elemento della lista
            it.toBack();
            treno.impostaRitardo(QString::fromUtf8("+%1").arg(it.previous()));
        }
        //rintraccia dal link per i dettagli del treno il codice della stazione di origine
        QString href = elemento.firstChildElement("a").attribute("href", "");
        QString cod = href.section("&",1,1).section("=",1,1);

        if (!cod.isEmpty())
        {
            treno.impostaCodiceOrigine(cod);
            //aggiunge il treno solo se ha un codice stazione di origine, questo per evitare duplicati che capitano con le S LeNord
            m_arrivi.append(treno);
        }

    }

    return true;
}

//costruisce un'istanza della classe che implementa il modello di dati per le
//tabelle della scheda stazione
//partenze == true --> il modello riguarda la scheda delle partenze,
//altrimenti quella degli arrivi
ModelloStazione::ModelloStazione(QWidget *parent, bool partenze) :
    QAbstractTableModel(parent), m_partenze(partenze)
{

}

//restituisce il numero di righe nel modello
//cioè il numero di treni elencati nel prospetto
int ModelloStazione::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_treni.count();
}

//restituisce il numero di colonne nel modello
int ModelloStazione::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    //il numero di colonne è pari all'indice dell'ultima colonna +1
    return (StazioneVT::colUltima+1);
}

//restituisce i titoli delle colonne del modello
QVariant ModelloStazione::headerData(int section, Qt::Orientation orientation, int role) const
{
    // usa l'implementazione di default per restituire i dati dell'header verticale
    if (orientation == Qt::Vertical)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (role == Qt::DisplayRole)
    {
        switch(section)
        {
        case StazioneVT::colCategoria: return QString::fromUtf8("Categoria"); break;
        case StazioneVT::colNumero: return QString::fromUtf8("Numero"); break;
            //è necessario distinguere se il modello riguarda il prospetto degli arrivi
            //o quello delle partenze perché il nome della colonna è differente nei due casi
        case StazioneVT::colStazione: {
            if (m_partenze)
                return QString::fromUtf8("Destinazione");
            else
                return QString::fromUtf8("Origine");
        }; break;
        case StazioneVT::colBinProgrammato: return QString::fromUtf8("Bin. previsto"); break;
        case StazioneVT::colBinReale: return QString::fromUtf8("Bin. effettivo"); break;
        case StazioneVT::colOrario: return QString::fromUtf8("Orario"); break;
        case StazioneVT::colRitardo: return QString::fromUtf8("Ritardo"); break;
        default: return QVariant();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

//passa al modello la lista dei treni
void ModelloStazione::aggiornaModello(const QList<StazioneVT::DatiTreno> &treni)
{
beginResetModel();
    m_treni = treni;
    //pulisce le liste di stazioni e categorie
    m_stazioni.clear();
    m_categorie.clear();
    //scorre la tabella e memorizza categoria e stazione di origine e o destinazione
    QListIterator<StazioneVT::DatiTreno> it(treni);
    while (it.hasNext())
    {
        StazioneVT::DatiTreno treno = it.next();
        m_categorie.append(treno.categoria());
        m_stazioni.append(treno.stazione());
    }
    //rimuove duplicati
    m_categorie.removeDuplicates();
    m_stazioni.removeDuplicates();
    //ed ordina le liste
    m_categorie.sort();
    m_stazioni.sort();
endResetModel();

}

QVariant ModelloStazione::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int riga = index.row();
    int col = index.column();

    //Imposta l'allineamento delle colonne
    if (role == Qt::TextAlignmentRole)
    {
        switch(col)
        {
        case StazioneVT::colCategoria:
        case StazioneVT::colNumero:
        case StazioneVT::colStazione: return int(Qt::AlignLeft|Qt::AlignVCenter); break;
        case StazioneVT::colBinProgrammato:
        case StazioneVT::colBinReale:
        case StazioneVT::colOrario:
        case StazioneVT::colRitardo: return Qt::AlignCenter; break;
        }
    }

    //imposta la sottolineatura delle voci (per simulare un link)
    if (role == Qt::FontRole)
    {
        QFont font;
        if ((col == StazioneVT::colNumero) || (col == StazioneVT::colStazione))
            font.setUnderline(true);
        else
            font.setUnderline(false);
        return font;
    }

    //imposta il colore del testo
    // per le colonne con il numero del treno e il nome della stazione viene impostato come
    // colore il colore di default per i link, in modo insieme alla sottolineatura da simulare un link
    // per le altre colonne invece il colore viene calcolato in funzione del ritardo
    if (role == Qt::ForegroundRole)
    {
        //restituisce un colore che codifica l'entità del ritardo per rendere meglio visibili nella tabella i treni in ritardo
        // se il ritardo è > 10 minuti restituisce un colore rosso
        // se è tra 3 e 10 minuti restituisce un colore giallo scuro
        //se è meno di 3 minuti restituisce il colore di default
        bool ok;
        QColor colore;
        int rit = m_treni.at(riga).ritardo().toInt(&ok);

        //selezione un colore opportuno per la riga di testo
        if (rit > 2 && rit<= 10)
            colore = Qt::darkYellow;
        else if (rit > 10)
            colore = Qt::red;
        //nessun colore speciale usa il colore di default
        else
            colore = QApplication::palette().text().color();

        // le colonne sono quelle di numero treno e stazione?
        if ((col == StazioneVT::colNumero) || (col == StazioneVT::colStazione))
            //se si resituisce il colore del link
            return (QApplication::palette().link());
        else
            //altrimenti restituisce il colore che codifica il ritardo
            return colore;
    }

    //restituisce il codice della stazione di origine treno
    if (role == Qt::UserRole)
    {
        if (col == StazioneVT::colNumero)
            return m_treni.at(riga).codiceOrigine();
        else
            return QVariant();
    }

    //restituisce i dati veri e propri
    if (role == Qt::DisplayRole)
        switch(col)
        {
        case StazioneVT::colCategoria: return m_treni.at(riga).categoria(); break;
        case StazioneVT::colNumero: return m_treni.at(riga).numero(); break;
        case StazioneVT::colStazione: return m_treni.at(riga).stazione(); break;
        case StazioneVT::colBinProgrammato: return m_treni.at(riga).binarioProgrammato(); break;
        case StazioneVT::colBinReale: return m_treni.at(riga).binarioReale(); break;
        case StazioneVT::colOrario: return m_treni.at(riga).orario(); break;
        case StazioneVT::colRitardo: {
            QString rit = m_treni.at(riga).ritardo();
            if (rit == "0")
                return QString::fromUtf8("In orario");
            else
            {
                bool ok;
                int ritardo = rit.toInt(&ok);
                if (ritardo==1)
                    return QString::fromUtf8("1 minuto");
                else
                    return QString::fromUtf8("%1 minuti").arg(ritardo);
            }
        } break;
        default: return QVariant();
        }

    //non previsto, ritorna un valore vuoto
        return QVariant();
}


