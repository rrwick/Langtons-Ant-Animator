#-------------------------------------------------
#
# Project created by QtCreator 2012-06-14T19:38:49
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Langtons_Ant
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    statewidget.cpp \
    antsettings.cpp \
    grid.cpp \
    antgrid.cpp \
    imageblender.cpp \
    antcounter.cpp \
    searchdialog.cpp

HEADERS  += mainwindow.h \
    statewidget.h \
    antsettings.h \
    antdirection.h \
    grid.h \
    antgrid.h \
    imageblender.h \
    antcounter.h \
    searchdialog.h

FORMS    += mainwindow.ui \
    statewidget.ui \
    searchdialog.ui

RESOURCES += \
    images.qrc


unix:QMAKE_CXXFLAGS += -std=c++11

win32:RC_FILE = myapp.rc
macx:ICON = application.icns
