#-------------------------------------------------
#
# Project created by QtCreator 2014-10-07T16:24:41
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uhuru-qt
TEMPLATE = app

SOURCES += main.cpp\
    model/scanmodel.cpp \
    ui/scanwidget.cpp \
    ui/systray.cpp \
    model/counter.cpp \
    model/scanreportmodel.cpp \
    ui/aboutdialog.cpp \
    utils/uhuru.cpp \
    ui/scanwindow.cpp \
    ui/updatedialog.cpp \
    model/updateinfomodel.cpp

HEADERS  +=  \
    model/scanmodel.h \ 
    ui/scanwidget.h \
    ui/systray.h \
    model/counter.h \
    utils/stdpaths.h \
    model/scanreportmodel.h \
    ui/aboutdialog.h \
    utils/uhuru.h \
    ui/scanwindow.h \
    ui/updatedialog.h \
    model/updateinfomodel.h

FORMS    += \  
    ui/scanwidget.ui \
    ui/aboutdialog.ui \
    ui/updatedialog.ui

RESOURCES = uhuru.qrc

TRANSLATIONS = translations/uhuru-qt_fr.ts \
                translations/uhuru-qt_en.ts

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libuhuru

target.path =    $$PREFIX/bin
INSTALLS += target
