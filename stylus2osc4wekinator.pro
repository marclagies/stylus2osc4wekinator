QT += core
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

TARGET = stylus2osc4wekinator

SOURCES += stylus2osc4wekinator.cpp

HEADERS += stylus2osc4wekinator.h

LIBS += /usr/lib/x86_64-linux-gnu/libX11.so.6 \
        /usr/lib/x86_64-linux-gnu/libXext.so \
        /usr/lib/x86_64-linux-gnu/libXi.so \
        /usr/lib/liblo.so
