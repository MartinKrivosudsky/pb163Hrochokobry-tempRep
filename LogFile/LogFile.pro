TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    logfile.cpp \
    tests.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    logfile.h \
    catch.hpp

