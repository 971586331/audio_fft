#-------------------------------------------------
#
# Project created by QtCreator 2020-02-05T20:48:36
#
#-------------------------------------------------

QT       += core gui charts multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = audio_fft
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    xyseriesiodevice.cpp \
    qcustomplot.cpp

HEADERS += \
        mainwindow.h \
    fftw-3.3.5-dll32/fftw3.h \
    xyseriesiodevice.h \
    qcustomplot.h

FORMS += \
        mainwindow.ui

LIBS += $$PWD\fftw-3.3.5-dll32\libfftw3-3.lib
