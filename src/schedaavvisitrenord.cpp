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

#include "download_trenord.h"
#include "schedaavvisitrenord.h"
#include "qviaggiatreno.h"
#include "parser_trenord.h"

SchedaAvvisiTrenord::SchedaAvvisiTrenord(QViaggiaTreno *parent, const unsigned int intervalloStandard) :
    SchedaQViaggiaTreno(parent, tsAvvisiTrenord, intervalloStandard)
{
    m_stato = statoNuovaScheda;
    //crea il widget con le tabelle contenti gli avvisi

    m_parser = new ParserTrenord(this);
    m_avvisi = new ModelloAvvisiTrenord(this);

    m_widgetAvvisi = new WidgetAvvisiTrenord(this, m_avvisi);
    addWidget(m_widgetAvvisi);

    //connessioni
    connect(this, SIGNAL(statoCambiato(quint32)), parent, SLOT(aggiornaStatoScheda(quint32)));
    //TODO: una volta terminato codice scheda verificare se questa connessione sia realmente necessaria
    connect(this, SIGNAL(messaggioStatus(const QString&)), parent, SLOT(mostraMessaggioStatusBar(const QString&)));
    connect(this, SIGNAL(aggiornaListaDirettrici()), qViaggiaTreno()->downloadTrenord(), SLOT(aggiornaListaDirettrici()));
}

SchedaAvvisiTrenord::~SchedaAvvisiTrenord()
{
    if (m_parser)
        delete m_parser;
    if (m_avvisi)
        delete m_avvisi;
}

void SchedaAvvisiTrenord::avvia()
{
      SchedaQViaggiaTreno::avvia();

}

void SchedaAvvisiTrenord::ferma()
{
    SchedaQViaggiaTreno::ferma();
}

void SchedaAvvisiTrenord::aggiorna()
{
    cambiaStato(statoInAggiornamento);

    emit aggiornaListaDirettrici();
}

void SchedaAvvisiTrenord::downloadFinito(const QString &rispostaTN)
{
    if (m_parser->analizzaListaDirettrici(rispostaTN))
        qViaggiaTreno()->downloadTrenord()->scaricaAvvisi(m_parser);




}
