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

#ifndef DOWNLOAD_VIAGGIATRENO_H
#define DOWNLOAD_VIAGGIATRENO_H

#include <QtNetwork>

class QViaggiaTreno;
class DownloadViaggiaTreno;
class DownloadViaggiaTrenoItem;
class SchedaQViaggiaTreno;

//questa classe è la classe in cui viene centralizzato il download delle schede
//da viaggiatreno
class DownloadViaggiaTreno : public QObject
{
	Q_OBJECT
public:
	DownloadViaggiaTreno(QViaggiaTreno* qvt, QNetworkAccessManager* nam);

signals:
    void statoViaggiaTreno(bool);

public slots:
	//mette in coda la richiesta di scaricare da Viaggiatreno la scheda di una stazione, fornendo il nome della stazione
	void downloadStazione(quint32 idScheda, const QString& nomeStazione);

	//mette in coda la richiesta di scaricare da ViaggiaTreno la scheda di una stazione, fornendone il codice
	void downloadStazioneCodice(quint32 idScheda, const QString& codiceStazione);

	//mette in coda la richiesta di scaricare da ViaggiaTreno la scheda di riepilogo di un treno, fornendone il numero
	void downloadRiepilogoTreno(quint32 idScheda, const QString& numero);

	//mette in coda la richiesta di scaricare da ViaggiaTreno la scheda di riepilogo di un treno, fornendo il numero
	//e il codice della stazione di origine
	void downloadRiepilogoTreno(quint32 idScheda, const QString &numero, const QString& codiceStazOrigine);

	//mette in coda la richiesta di scaricare da ViaggiaTreno la scheda con i dettagli di un treno, fornendone il numero
	void downloadDettagliTreno(quint32 idScheda, const QString& numero);

	//mette in coda la richiesta di scaricare da ViaggiaTreno la scheda con i dettagli di un treno
	//fornendo il numero e il codice della stazione di origine
	void downloadDettagliTreno(quint32 idScheda, const QString &numero, const QString& codiceStazOrigine);

    //avvia il download
    void avvia();


private:
	//corregge alcuni errori nel codice XHTML generato da viaggiatreno che ne impediscono
	//il corretto parsing da parte della classe QDom
	QString correggiOutputVT(QString testoVT);

	void richiestaHTTPStazioneConNome(DownloadViaggiaTrenoItem *item);
	void richiestaHTTPStazioneConCodice(DownloadViaggiaTrenoItem *item);
	void richiestaHTTPRiepilogoTreno(DownloadViaggiaTrenoItem* item);
	void richiestaHTTPRiepilogoTrenoConOrigine(DownloadViaggiaTrenoItem *item);
	void richiestaHTTPDettagliTreno(DownloadViaggiaTrenoItem *item);
	void richiestaHTTPDettagliTrenoConOrigine(DownloadViaggiaTrenoItem * item);

private slots:
	void download();
    void downloadEffettuato();
    bool controllaViaggiaTreno();

private:
	QViaggiaTreno* m_qvt;
	QNetworkAccessManager* m_nam;
	//intervallo tra due download successivi dal server di viaggiatreno
	int m_intervalloDownload;
	//intervallo di aggiornamento delle cache
	int m_intervalloAggiornamentoCache;
	//tempo in cui una scheda rimane in cache
	int m_tempoCache;
	QQueue<DownloadViaggiaTrenoItem*> m_codaDownload;
    QTimer *m_timerDownload, *m_timerControlloVT;

    //costanti
    //valore di default dell'intervallo di download (in ms)
    static const int s_intervalloDownload;

    //valore di default dell'intervallo di aggiornamento della cache (in ms)
    static const int s_intervalloAggiornamentoCache;

    //valore di default del tempo di permanenza in cache (in s)
    static const int s_tempoCache;

    //valore di default dell'intervallo di controllo del corretto funzionamento di Viaggiatreno (in m)
    static const int s_itervalloCheckViaggiaTreno;
};

//l'enum elenca i tipi diversi di dati possibili scaricabili da viaggiatreno
enum TipoSchedaViaggiaTreno {StazioneConNome, StazioneConCodice, RiepilogoTreno, RiepilogoTrenoConOrigine,
							 DettagliTreno, DettagliTrenoConOrigine};

//questa classe memorizza i dati di ogni singolo download
class DownloadViaggiaTrenoItem : public QObject
{
	Q_OBJECT
public:
	DownloadViaggiaTrenoItem(quint32 idScheda, TipoSchedaViaggiaTreno tipoScheda);

	//restituisce il tipo di scheda
	TipoSchedaViaggiaTreno tipoScheda() const {return m_tipoSchedaVT;}

	//restituisce data e ora della richiesta di download
	QDateTime dataEOra() const {return m_dataEOra;}

	//restituisce puntatore alla scheda che ha effettuato la richiesta di download
	quint32 idScheda() const {return m_scheda;}

	//restituisce uno dei dati impostati
	QString dato(const QString& nome) const {return m_dati[nome];}

	//imposta un dato
	void impostaDato(const QString& nome, const QString& valore)
		{
		m_dati[nome] = valore;
		}

private:
	quint32 m_scheda;
	TipoSchedaViaggiaTreno m_tipoSchedaVT;
	QMap<QString, QString> m_dati;
	QDateTime m_dataEOra;
};

#endif // DOWNLOAD_VIAGGIATRENO_H
