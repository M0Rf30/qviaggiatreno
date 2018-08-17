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

#include "schedalistatreni.h"
#include "download_viaggiatreno.h"
#include "qviaggiatreno.h"
#include "schedatreno.h"
#include "utils.h"

int SchedaListaTreni::s_count = 0;

SchedaListaTreni::SchedaListaTreni(QViaggiaTreno* parent, const unsigned int intervalloStandard):
    SchedaQViaggiaTreno(parent, tsListaTreni, intervalloStandard)
{
    s_count++;

    m_stato = statoNuovaScheda;
    m_modificata = false;
    m_titoloLista = "";
    m_idTabella = m_idNumeroAmbiguo = -1;

    //imposta il widget
    m_widget = new WidgetListaTreni(this);
    m_idTabella = addWidget(m_widget);
    setCurrentIndex(m_idTabella);
    m_widget->impostaTitolo(titolo(true));
    m_widget->ridimensionaColonne();

    //imposta il parser
    m_parser = new ParserTrenoViaggiaTreno(this);

    //imposta connessioni
    connect(this, SIGNAL(statoCambiato(quint32)), parent, SLOT(aggiornaStatoScheda(quint32)));
    connect(this, SIGNAL(nomeSchedaCambiato(quint32)), parent, SLOT(aggiornaNomeScheda(quint32)));
    connect(this, SIGNAL(apriSchedaStazione(const QString&, bool)), parent, SLOT(nuovaStazione(const QString&, bool)));
    connect(this, SIGNAL(apriSchedaTreno(const QString&)), parent, SLOT(nuovoTreno(const QString&)));
    connect(this, SIGNAL(GuiNonSincronizzata(quint32)), parent, SLOT(sincronizzaGUI(quint32)));
    connect(this, SIGNAL(messaggioStatus(const QString&)), parent, SLOT(mostraMessaggioStatusBar(const QString&)));
    connect(this, SIGNAL(downloadRiepilogoTreno(quint32, QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadRiepilogoTreno(quint32, QString)));
    connect(this, SIGNAL(downloadRiepilogoTreno(quint32,QString,QString)), qViaggiaTreno()->downloadViaggiaTreno(), SLOT(downloadRiepilogoTreno(quint32, QString, QString)));
}

//restituisce una stringa con il "titolo" di questa scheda, che viene ad esempio usato come etichetta della TabBar
// se titoloItem == true restituisce un titolo per l'item dell'albero
QString SchedaListaTreni::titolo(bool titoloBreve) const
{
    if (m_titoloLista.isEmpty())
    {
        if (titoloBreve)
            return(QString::fromUtf8("Lista di treni senza titolo #%1").arg(s_count));
        else
            return(QString::fromUtf8("Lista di treni: senza titolo #%1").arg(s_count));
    }
    else
    {
        if (titoloBreve)
            return m_titoloLista;
        else
            return(QString::fromUtf8("Lista di treni: %1").arg(m_titoloLista));
    }
}

void SchedaListaTreni::impostaTitolo()
{
    bool ok;
    QString titolo = QInputDialog::getText(this, QString::fromUtf8("Impostare titolo della lista"),
                                           QString::fromUtf8("Inserire il nuovo titolo della lista di treni"),
                                           QLineEdit::Normal, m_titoloLista, &ok);
    if (ok)
    {
        m_titoloLista = titolo;
        m_widget->impostaTitolo(titolo);
        m_modificata = true;

        emit GuiNonSincronizzata(idScheda());
        emit nomeSchedaCambiato(idScheda());
    }
}


void SchedaListaTreni::avvia()
{
    SchedaQViaggiaTreno::avvia();
}

void SchedaListaTreni::ferma()
{
    SchedaQViaggiaTreno::ferma();

    m_codatreni.clear();
    m_trenoAttuale = "";

}

void SchedaListaTreni::aggiorna()
{
    //cancella dallo StackedWidget i widget creati per gestire gli errori

    if (m_idNumeroAmbiguo != -1)
    {
        QWidget *widgetDaRimuovere = widget(m_idNumeroAmbiguo);
        widgetDaRimuovere->deleteLater();
        m_idNumeroAmbiguo = -1;
    }

    //ottiene una lista di tutti i numeri dei treni che si stanno attualmente controllando
    QStringList listaNumeri = m_listatreni.keys();

    //e li aggiunge alla coda
    QStringListIterator it(listaNumeri);
    while (it.hasNext())
        m_codatreni.enqueue(it.next());

    //preleva il primo elemento dalla coda e procedi...
    prossimoTreno();
}

void SchedaListaTreni::downloadFinito(const QString &rispostaVT)
{
    ListaVT::DatiTreno* treno;
    //procedi solo se nel frattempo non è stato cancellato il numero del treno attuale
    //questo succede ad esempio se nel frattempo il treno è stato rimosso dalla lista dei treni
    if (m_trenoAttuale != "")
    {
        treno = m_listatreni.value(m_trenoAttuale);
        m_parser->impostaRispostaVTRiepilogo(rispostaVT);

        //sono possibili 6 risposte
        //1) il numero del treno non è valido
        //2) il treno non è partito ancora
        //3) il treno è in viaggio
        //4) il treno è già arrivato
        //5) il treno è stato soppresso totalmente
        //6) il numero treno è ambiguo
        //nei casi 1)  5) e 6) non è necessario fare il parsing della risposta...
        //verifichiamo quindi se siamo in questa situazione e nel caso aggiorniamo immediatamente il treno
        //o lasciamo risolvere all'utente l'ambiguità sul numero treno

        if (m_parser->trenoNonPrevisto())
        {
            treno->cancella();
            treno->impostaStatoTreno(TrenoVT::TrenoNonPrevisto);
        }
        else if (m_parser->trenoSoppressoTotalmente())
        {
            treno->cancella();
            treno->impostaStatoTreno(TrenoVT::TrenoCancellato);
        }
        else if (m_parser->numeroTrenoAmbiguo())
        {
            m_trenoAmbiguo = m_trenoAttuale;
            ferma();
            m_idNumeroAmbiguo = addWidget(new WidgetDisambiguaNumeroTreno(this, treno->numero(), m_parser->listaCodiciTreno(rispostaVT)));
            setCurrentIndex(m_idNumeroAmbiguo);
            cambiaStato(statoErrore);
            return;
        }
        else
        {
            if (!m_parser->analizzaRiepilogoPerLista(*treno))
            {
                cambiaStato(statoErrore);
                 treno->impostaStatoTreno(TrenoVT::DatiSconosciuti);
                return;
            }
        }

        //aggiorna la riga del treno attuale nel widget
        m_widget->aggiornaTreno(treno);

        emit messaggioStatus(QString::fromUtf8("Aggiornati dati treno %1 della lista '%2' Rimangono %3 su %4 treni da analizzare")
                             .arg(treno->numero()).arg(titolo(true)).arg(m_codatreni.count()).arg(m_listatreni.count()));

        //non ci sono altri treni in coda?
        if (m_codatreni.isEmpty())
            //no, cambia stato e memorizza l'ora di fine aggiornamento
        {
            m_trenoAttuale = "";
            m_ultimoAgg = QDateTime::currentDateTime();
            m_widget->impostaAggiornamento(m_ultimoAgg.toString("dd/MM/yyyy hh:mm"));
            cambiaStato(statoMonitoraggioAttivo);
        }
        else
            //si, preleva il treno successivo
            prossimoTreno();
    }
}

//slot
//questo metodo risponde all'attivazione dell'azione per l'apertura di un file
void SchedaListaTreni::apri()
{
    QString filename = QFileDialog::getOpenFileName(this, QString::fromUtf8("Apri lista treni"),
                                                    QDir::homePath(), QString::fromUtf8("File QViaggiaTreno (*.qvt);;Tutti i file (*.*)"));

    if (filename != "")
        apriFile(filename);
}

void SchedaListaTreni::apriFile(const QString& filename)
{
    QFile fileaperto(filename);
    if (!fileaperto.open(QIODevice::ReadOnly))
    {
        //errore durante l'apertura del file
        QMessageBox::critical(this, QString::fromUtf8("Errore apertura file"),
                              QString::fromUtf8("Impossibile aprire il file '%1'").arg(filename));
        return;
    }

    QString errore;
    int riga, col;
    if (!m_dom.setContent(&fileaperto, &errore, &riga, &col))
    {
        //errore nel parsing
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString::fromUtf8("Errore durante la lettura della lista di treni."));
        msgBox.setDetailedText(QString::fromUtf8("Ricevuto messaggio di errore %1 alla riga %2, colonna %3.").arg(errore).arg(riga).arg(col));
        msgBox.exec();
        return;
    }

    //legge il file xml
    QDomElement radice = m_dom.documentElement();
    //verifica che il tag dell'elemento radice sia "qviaggiatreno"
    if (radice.tagName() != "qviaggiatreno")
    {
        QMessageBox::warning(this, QString::fromUtf8("Errore nel file"),
                             QString::fromUtf8("Il file non sembra essere un file dati di QViaggiaTreno"));
        return;
    }

    //in futuro si dovrà verificare la versione del file, ora non è necessario
    if (radice.attribute("tipo") != "lista_treni")
    {
        QMessageBox::warning(this, QString::fromUtf8("Errore nel file"),
                             QString::fromUtf8("Il file è un file di dati di QViaggiaTreno,"
                                               " ma non sembra contenere una lista treni"));
        return;
    }

    // il file contiene una lista di treni...
    //recupera il titolo
    m_titoloLista = radice.firstChildElement("titolo").text();
    m_widget->impostaTitolo(titolo(true));
    emit nomeSchedaCambiato(idScheda());
    //ottiene una lista di tutti gli elementi con tag name "treno"
    QDomNodeList listaTreni = m_dom.elementsByTagName("treno");
    //scorre la lista ed aggiunge i singoli treni
    for (int i = 0; i < listaTreni.count(); i++)
    {
        aggiungiTreno(listaTreni.at(i).toElement().attribute("numero"),
                      listaTreni.at(i).toElement().attribute("codice", ""));
    }
    fileaperto.close();
    //forza l'aggiornamento
    aggiorna();
    m_modificata = false;
    m_nomefile = filename;
    //sincronizza la GUI
    emit GuiNonSincronizzata(idScheda());
}

//slot
//viene richiamato quando viene scelto da menu o toolbar l'azione epr salvare il file
void SchedaListaTreni::salva()
{
    if (m_nomefile != "")
        salvaFile(m_nomefile);
    else
        salvaConNome();
}

void SchedaListaTreni::salvaConNome()
{
    QString filename = QFileDialog::getSaveFileName(this, QString::fromUtf8("Salva file con nome"),
                                                    QDir::homePath(), QString::fromUtf8("File QViaggiaTreno (*.qvt)"));
    if (filename != "")
    {
        // se il file non ha una estensione, aggiungi l'estensione .qvt
        if (QFileInfo(filename).suffix() == "")
            filename.append(".qvt");
        salvaFile(filename);
    }
}

void  SchedaListaTreni::salvaFile(const QString& filename)
{
    //prova ad aprire il file
    QFile fileDaSalvare(filename);
    //l'apertura non è andata a buon fine
    if (!fileDaSalvare.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, QString::fromUtf8("Errore salvataggio file"),
                              QString::fromUtf8("Errore nell'apertura in scrittura del file %1").arg(filename));
        return;
    }

    //cancella il documento dom
    m_dom = QDomDocument("qviaggiatreno");
    //crea l'elemento radice
    QDomElement radice = m_dom.createElement("qviaggiatreno");
    m_dom.appendChild(radice);
    //imposta versione e tipo file
    radice.setAttribute("versione", "2");
    radice.setAttribute("tipo", "lista_treni");
    //aggiunge la radice al documento

    //crea l'elemento per il titolo
    QDomElement titolo = m_dom.createElement("titolo");
    QDomText testo = m_dom.createTextNode(m_titoloLista);
    titolo.appendChild(testo);
    radice.appendChild(titolo);
    //scorre la lista dei treni monitorati
    QDomElement lista = m_dom.createElement("listatreni");
    radice.appendChild(lista);
    QStringListIterator it(m_listatreni.keys());
    while (it.hasNext())
    {
        QString numero = it.next();
        QDomElement elementoTreno = m_dom.createElement("treno");
        elementoTreno.setAttribute("numero", numero);
        if (!m_listatreni[numero]->codiceOrigine().isEmpty())
            elementoTreno.setAttribute("codice", m_listatreni[numero]->codiceOrigine());
        lista.appendChild(elementoTreno);
    }

    //salva l'albero DOM
    QTextStream stream(&fileDaSalvare);
    m_dom.save(stream, 4);
    fileDaSalvare.close();

    //cambia nome file
    m_nomefile = filename;
    m_modificata = false;
    emit GuiNonSincronizzata(idScheda());
}

//aggiunge un singolo treno alla lista dei treni da monitorare
void SchedaListaTreni::aggiungiTreno(const QString& numero, const QString& codice)
{
    //verifica se si sta già controllando questo treno,
    //in caso positivo esci dalla funzione senza fare niente
    if (m_listatreni.contains(numero))
        return;

    //crea una nuova istanza di ListaVT::DatiTreno per memorizzare i dati del treno
    ListaVT::DatiTreno *treno = new ListaVT::DatiTreno(numero);
    //imposta, se esiste, il codice della stazione di origine
    if (!codice.isEmpty())
        treno->impostaCodiceOrigine(codice);
    //aggiunge il treno alla mappa dei treni
    m_listatreni.insert(numero, treno);
    //e lo aggiunge al widget
    m_widget->aggiungiTreno(treno);

}

//rimuove un singolo treno alla lista dei treni da monitorare
void SchedaListaTreni::rimuoviTreno(const QString& numero)
{
    //rimuove il treno dalla lista dei treni
    m_listatreni.remove(numero);

    //verifica che non sia nella coda dei treni, se c'è rimuovilo
    m_codatreni.removeAll(numero);
    if (m_trenoAttuale == numero)
        m_trenoAttuale = "";

    m_widget->rimuoviTreno(numero);
}

//slot
//questo slot è connesso all'azione per aggiungere treni
void SchedaListaTreni::aggiungiTreni()
{
    DialogoAggiuntaTreni dialogo(this);
    if (dialogo.exec() == QDialog::Accepted)
    {
        if (!dialogo.listaVuota())
        {
            QStringListIterator it(dialogo.listaTreni());
            while (it.hasNext())
                aggiungiTreno(it.next());
        }
    }

    //sono stati aggiunti tutti i treni, aggiornare la larghezza delle colonne
    m_widget->ridimensionaColonne();

    m_modificata = true;

    //c'è stato un cambiamento nella scheda che ha influenza sull'aspetto della GUI
    //va quindi emesso il segnale per sincronizzare la gui
    emit(GuiNonSincronizzata(idScheda()));

    //forza un aggiornamento immediato dei treni
    aggiorna();
}

void SchedaListaTreni::rimuoviTreni()
{
    DialogoRimozioneTreni dialogo(m_listatreni.keys(), this);
    if (dialogo.exec() == QDialog::Accepted)
    {
        QStringListIterator it(dialogo.listaTreni());
        while (it.hasNext())
            rimuoviTreno(it.next());
    }

    m_modificata = true;

    //c'è stato un cambiamento nella scheda che ha influenza sull'aspetto della GUI
    //va quindi emesso il segnale per sincronizzare la gui
    emit(GuiNonSincronizzata(idScheda()));
}

//slot
//questo slot è connesso all'azione per rimuovere tutti i treni dalla lista
void SchedaListaTreni::rimuoviTuttiITreni()
{
    //rimuovi tutti i treni dalla tabella
    m_widget->rimuoviTuttiITreni();

    //rimuovi tutti i treni dalla coda
    m_codatreni.clear();
    m_trenoAttuale = "";

    //elimina la lista di treni
    m_listatreni.clear();

    m_modificata = true;

    //sincronizza la gui
    emit(GuiNonSincronizzata(idScheda()));
}

//questa funziona preleva il prossimo treno dalla coda dei treni
//e richiede i dati a viaggiatreno
void SchedaListaTreni::prossimoTreno()
{
    //verifica che ci siano elementi nella coda, altrimenti esci
    if (m_codatreni.isEmpty())
        return;

    m_trenoAttuale = m_codatreni.dequeue();

    QString temp = m_listatreni[m_trenoAttuale]->codiceOrigine();

    if (m_listatreni[m_trenoAttuale]->codiceOrigine().isEmpty())
        emit (downloadRiepilogoTreno(idScheda(), m_trenoAttuale));
    else
        emit (downloadRiepilogoTreno(idScheda(), m_trenoAttuale, m_listatreni[m_trenoAttuale]->codiceOrigine()));
}

void SchedaListaTreni::salvaScheda(QSettings& settings)
{
    //la scheda è modificata, proponi di salvare il contenuto
    if (modificata())
    {
        int risposta = QMessageBox::warning(this, QString::fromUtf8("La scheda è modificata"),
                                            QString::fromUtf8("La scheda <b>'%1'</b> è stata modificata, ma la lista di treni non è ancora stata salvata e non ne sarà possibile il ripristino.<br>"
                                                              "Si desidera salvare la lista?").arg(titolo(true)), QMessageBox::Yes, QMessageBox::No);

        if (risposta == QMessageBox::Yes)
            salva();
    }

    //salva semplicemente il nome del file
    settings.setValue("tipo scheda", "lista treni");
    settings.setValue("nome file", m_nomefile);
}

void SchedaListaTreni::cambiaCodiceOrigine(const QString &nuovoCodice)
{
    m_listatreni[m_trenoAmbiguo]->impostaCodiceOrigine(nuovoCodice);
    aggiorna();
    avvia();
}
