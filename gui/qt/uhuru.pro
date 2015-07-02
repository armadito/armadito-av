#-------------------------------------------------
#
# Project created by QtCreator 2014-10-07T16:24:41
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uhuru
TEMPLATE = app


SOURCES += main.cpp\
    ui/mainwindow.cpp \
    model/scanmodel.cpp \
    ui/scanwidget.cpp \
    ui/systray.cpp \
    model/counter.cpp \
    model/scanreportmodel.cpp \
    utils/umwsu.cpp \
    ui/aboutdialog.cpp

HEADERS  += ui/mainwindow.h \
    model/scanmodel.h \ 
    ui/scanwidget.h \
    ui/systray.h \
    model/counter.h \
    utils/umwsu.h \
    utils/stdpaths.h \
    model/scanreportmodel.h \
    ui/aboutdialog.h

FORMS    += \  
    ui/scanwidget.ui \
    ui/aboutdialog.ui

RESOURCES = uhuru.qrc

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libumwsu

target.path =    $$PREFIX/bin
INSTALLS += target

customdist.commands = $$_PRO_FILE_PWD_/../scripts/makedist.sh uhuruihm-1.0.0 $$_PRO_FILE_PWD_ $$OUT_PWD $$SOURCES $$HEADERS $$RESOURCES $$FORMS $$RC_FILE uhuru.pro icons/uhuru.svg
customdist.target = customdist
QMAKE_EXTRA_TARGETS += customdist
