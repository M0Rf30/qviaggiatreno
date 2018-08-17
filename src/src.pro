TEMPLATE = app
CONFIG += warn_on
QT += xml webkitwidgets
TARGET = qviaggiatreno
DESTDIR = ../bin
RESOURCES = application.qrc
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac

FORMS += dlgnomestazione.ui \
    dlgconfigurazione.ui \
    wgtstazione.ui \
    wgtavvisitrenord.ui

SOURCES += qviaggiatreno.cpp \
    main.cpp \
    schedaviaggiatreno.cpp \
    schedastazione.cpp \
    qviaggiatrenoslots.cpp \
    schedatreno.cpp \
    schedalistatreni.cpp \
    qviaggiatrenoslots_schede.cpp \
    schedastazione_widget.cpp \
    schedalistatreni_widget.cpp \
    items.cpp \
    schedatreno_widget.cpp \
    utils.cpp \
    download_viaggiatreno.cpp \
    config.cpp \
    dialogo_nomestazione.cpp \
    dialogo_configurazione.cpp \
    qledindicator.cpp \
    parser_viaggiatreno_stazione.cpp \
    parser_viaggiatreno_treno.cpp \
    schedaavvisitrenord.cpp \
    schedaavvisitrenord_widget.cpp \
    download_trenord.cpp \
    parser_trenord.cpp \
    parser_viaggiatreno_base.cpp
HEADERS += qviaggiatreno.h \
    schedaviaggiatreno.h \
    schedastazione.h \
    schedatreno.h \
    schedalistatreni.h \
    items.h \
    utils.h \
    download_viaggiatreno.h \
    config.h \
    dialogo_nomestazione.h \
    dialogo_configurazione.h \
    qledindicator.h \
    parser_viaggiatreno_stazione.h \
    parser_viaggiatreno_treno.h \
    parser_viaggiatreno_lista.h \
    parser_viaggiatreno_base.h \
    schedaavvisitrenord.h \
    download_trenord.h \
    parser_trenord.h























