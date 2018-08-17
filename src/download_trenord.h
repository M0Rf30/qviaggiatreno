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

#include <QtNetwork>

class QViaggiaTreno;
class ParserTrenord;

//Questa classe è la classe in cui viene centralizzato il download dal sito Trenord
class DownloadTrenord :public QObject
{
    Q_OBJECT
public:
    DownloadTrenord(QViaggiaTreno* qvt, QNetworkAccessManager* nam);

public slots:
    void aggiornaListaDirettrici();
    void scaricaAvvisi(ParserTrenord *parser);
    void downloadFinito();
    void scaricaNuovaDirettrice();

    private:
    QViaggiaTreno *m_qvt;
    QNetworkAccessManager* m_nam;
    ParserTrenord *m_parser;
    QQueue<QString> m_coda;

    QTimer* m_timerAvvisi;
};

#endif // DOWNLOAD_TRENORD_H
