/***************************************************************************
 *   Copyright (C) 2012 by fra74                                           *
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
#ifndef PARSER_TRENORD_H
#define PARSER_TRENORD_H

#include <QtWebKit>

class SchedaQViaggiaTreno;

class AvvisoTrenord
{
public:
    void impostaOrario(QDateTime orario) {m_orario = orario;}
    QDateTime orario() const {return m_orario;}

    void impostaDirettrice(const QString& direttrice) {m_direttrice = direttrice;}
    QString direttrice() const {return m_direttrice;}

    void impostaTesto(const QString& testo) {m_testo = testo;}
    QString testo() const {return m_testo;}



private:
    QDateTime m_orario;
    QString m_direttrice, m_testo;

};

class ParserTrenord : public QObject
{
    Q_OBJECT
public:
    ParserTrenord(SchedaQViaggiaTreno* scheda);
    bool analizzaListaDirettrici(const QString& rispostaTN);
    QQueue<QString> listaDirettrici() const {return m_direttrici;}


private:
    QQueue<QString> m_direttrici;
    SchedaQViaggiaTreno* m_scheda;

};

class ModelloAvvisiTrenord : public QAbstractTableModel
{
    Q_OBJECT

public:
    ModelloAvvisiTrenord(QWidget* parent);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

 private:
    QList<AvvisoTrenord> m_avvisi;
};

#endif // PARSER_TRENORD_H
