TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += qt

QT += network
QT += widgets
QT += sql

SOURCES += main.cpp \
    Server.cpp \
    ClientThread.cpp \
    ../LogFile/logfile.cpp \
    Console.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    Server.h \
    ClientThread.h \
    ../LogFile/logfile.h \
    ../ProtocolMsg.h \
    Console.h

