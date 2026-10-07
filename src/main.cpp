/***************************************************************************
 *   Copyright (C) 2008-2010 by fra74   *
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


#include <QApplication>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "qviaggiatreno.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);

	//le impostazioni vanno configurate prima di creare qualsiasi finestra
	QCoreApplication::setOrganizationName("fra74");
	QCoreApplication::setApplicationName("QViaggiaTreno");
	QCoreApplication::setApplicationVersion(QString::fromUtf8(QVT_VERSION));
	QSettings::setDefaultFormat(QSettings::IniFormat);

	//traduzioni di Qt per la lingua di sistema, con ripiego sulla traduzione italiana inclusa
	QTranslator qTranslator;
	if (!qTranslator.load(QLocale(), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
	{
		if (!qTranslator.load(":/traduzioni/qt_it.qm"))
			qWarning("Impossibile caricare le traduzioni di Qt");
	}
	app.installTranslator(&qTranslator);

	QViaggiaTreno mw;
	mw.show();
	return app.exec();
}
