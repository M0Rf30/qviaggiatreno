/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74                                      *
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


#include "schedaviaggiatreno.h"
#include "qviaggiatreno.h"

unsigned long SchedaQViaggiaTreno::s_id = 0;
QString SchedaQViaggiaTreno::statoNuovaScheda = QString::fromUtf8("Nuova scheda");
QString SchedaQViaggiaTreno::statoInAggiornamento = QString::fromUtf8("In aggiornamento");
QString SchedaQViaggiaTreno::statoMonitoraggioAttivo = QString::fromUtf8("Attivo");
QString SchedaQViaggiaTreno::statoMonitoraggioFermato = QString::fromUtf8("Fermo");
QString SchedaQViaggiaTreno::statoErrore = QString::fromUtf8("Errore");

//
SchedaQViaggiaTreno::SchedaQViaggiaTreno(QViaggiaTreno* parent, TipoScheda ts, const unsigned int intervalloStandard)
	: QStackedWidget(parent), m_tipo(ts)
{
	m_id = s_id++;
	m_qviaggiatreno = parent;
	
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(aggiorna()));
	m_timer->setInterval(intervalloStandard*60000);

    //per default la scheda non è né stampabile né esportabile
    m_stampabile = m_esportabile = false;
}

void SchedaQViaggiaTreno::avvia()
{
	if (m_timer)
		m_timer->start();
		
	cambiaStato(statoMonitoraggioAttivo);
	
}

void SchedaQViaggiaTreno::ferma()
{
	if (m_timer)
		m_timer->stop();
		
	cambiaStato(statoMonitoraggioFermato);	
}

int SchedaQViaggiaTreno::intervallo() const
{
	if (!m_timer)
		return 0;
	else
		return m_timer->interval()/60000;
}

void SchedaQViaggiaTreno::impostaIntervallo(int nuovoInt)
{
	if (m_timer)
    {
        //cambia intervallo solo se è differente dal precedente
        if (!(nuovoInt == m_timer->interval()/60000))
            m_timer->setInterval(nuovoInt*60000);
    }
}

void SchedaQViaggiaTreno::aggiorna()
{
	
}

void SchedaQViaggiaTreno::cambiaStato(QString nuovoStato)
{
	m_stato = nuovoStato;
	
	emit statoCambiato(m_id);
}


//la funzione restituisce la larghezza in punti di un item
int larghezzaItem(const QTableWidgetItem* item)
{
	if (!item)
		return -1;

	QFontMetrics fm(item->font());
	int larghezza = fm.width(item->text());
	return larghezza;
}

//la funzione cambia il font di un item in modo che sia sottolineato
// se sottolinato = true, altrimenti rimuove la sottolineatura
void itemSottolineato(QTableWidgetItem* item, bool sottolineato)
{
	QFont font = item->font();
	font.setUnderline(sottolineato);
	item->setFont(font);
	item->setForeground(QApplication::palette().link());
}
