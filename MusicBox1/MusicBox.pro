#-------------------------------------------------
#
# Project created by QtCreator 2020-10-17T18:45:17
#
#-------------------------------------------------

QT       += core gui
QT      +=  multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(./sqlapi/sqlapi.pri)
INCLUDEPATH += $$PWD/sqlapi

include(./uiapi/uiapi.pri)
INCLUDEPATH += $$PWD/uiapi

include(./netapi/netapi.pri)
INCLUDEPATH += $$PWD/netapi

TARGET = MusicBox
TEMPLATE = app


SOURCES += main.cpp\
        musicwidget.cpp \
    downloadwidget.cpp

HEADERS  += musicwidget.h \
    downloadwidget.h

FORMS    += musicwidget.ui \
    downloadwidget.ui

RESOURCES += \
    resource.qrc
