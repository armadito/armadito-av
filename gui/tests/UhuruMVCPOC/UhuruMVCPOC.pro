#-------------------------------------------------
#
# Project created by QtCreator 2014-10-07T16:24:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UhuruMVCPOC
TEMPLATE = app


SOURCES += main.cpp\
    ui/mainwindow.cpp \
    model/scanmodel.cpp \
    ui/scanwidget.cpp \
    model/counter.cpp

HEADERS  += ui/mainwindow.h \
    model/scanmodel.h \ 
    ui/scanwidget.h \
    model/counter.h \
    utils/umwsu.h

FORMS    += \  
    ui/scanwidget.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libumwsu
