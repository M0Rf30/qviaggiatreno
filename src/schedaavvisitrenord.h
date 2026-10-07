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


#ifndef SCHEDAAVVISITRENORD_H
#define SCHEDAAVVISITRENORD_H

#include "parser_trenord.h"
#include "schedaviaggiatreno.h"
#include "ui_wgtavvisitrenord.h"

#include <QHash>
#include <QSet>

class QViaggiaTreno;

class WidgetAvvisiTrenord: public QWidget, private Ui::wgtAvvisiTrenord
{
    Q_OBJECT

public:
    WidgetAvvisiTrenord(QWidget *parent, ModelloAvvisiTrenord* modello);

    //mostra la data dell'ultimo aggiornamento o un messaggio di errore
    void impostaAggiornamento(const QString& messaggio, bool errore = false);
    //mostra il riepilogo dello stato delle linee
    void impostaRiepilogo(const QString& riepilogo);

private:
    void mostraAvvisoSelezionato();

    ModelloAvvisiTrenord* m_modello;
};

class SchedaAvvisiTrenord: public SchedaQViaggiaTreno
{
    Q_OBJECT

public:
    SchedaAvvisiTrenord(QViaggiaTreno* parent, const unsigned int intervalloStandard = 5);

    QString titolo(bool = false) const override {return QString::fromUtf8("Avvisi Trenord");}

    void avvia() override;
    void aggiorna() override;
    void salvaScheda(QSettings& settings) override;

    void downloadFinito(const QString &) override;
    void downloadFallito(const QString& errore) override;

    //chiamati dal downloader quando la scheda di una linea è stata scaricata (o il download è fallito)
    void dettagliLineaScaricati(const QString& codice, const QString& risposta);
    void dettagliLineaNonScaricati(const QString& codice, const QString& errore);

    ~SchedaAvvisiTrenord() override;

private:
    void lineaCompletata(const QString& codice);
    void completaAggiornamento();

    WidgetAvvisiTrenord* m_widgetAvvisi;
    ParserTrenord* m_parser;
    ModelloAvvisiTrenord *m_avvisi;

    //stato dell'aggiornamento in corso
    bool m_inAggiornamento = false;
    QSet<QString> m_lineeInAttesa;
    QHash<QString, QString> m_nomiLinee;
    QList<AvvisoTrenord> m_avvisiInArrivo;
    QStringList m_lineeNonScaricate;

Q_SIGNALS:
    void aggiornaListaDirettrici();
};

#endif // SCHEDAAVVISITRENORD_H
