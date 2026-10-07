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
#ifndef DOWNLOAD_TRENORD_H
#define DOWNLOAD_TRENORD_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QTimer>

class QViaggiaTreno;
class SchedaAvvisiTrenord;

//Questa classe è la classe in cui viene centralizzato il download dal sito Trenord
class DownloadTrenord :public QObject
{
    Q_OBJECT
public:
    DownloadTrenord(QViaggiaTreno* qvt, QNetworkAccessManager* nam);

    //mette in coda il download delle schede delle linee indicate per la scheda avvisi;
    //le schede vengono scaricate una alla volta ad intervalli regolari
    void scaricaAvvisi(SchedaAvvisiTrenord* scheda, const QStringList& codiciLinee);

public slots:
    //scarica la lista delle linee con il loro stato; la risposta viene consegnata alla
    //scheda che ha emesso il segnale tramite downloadFinito()/downloadFallito()
    void aggiornaListaDirettrici();

private slots:
    void downloadFinito();
    void scaricaNuovaDirettrice();
    void dettagliLineaScaricati();

private:
    QNetworkRequest creaRichiesta(const QString& percorso) const;

    struct RichiestaLinea
    {
        QPointer<SchedaAvvisiTrenord> scheda;
        QString codice;
    };

    QViaggiaTreno *m_qvt;
    QNetworkAccessManager* m_nam;
    QQueue<RichiestaLinea> m_coda;

    QTimer* m_timerAvvisi;
};

#endif // DOWNLOAD_TRENORD_H
