TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    polarSSL/aes.c \
    polarSSL/padlock.c \
    polarSSL/aesni.c \
    polarSSL/platform.c \
    polarSSL/ctr_drbg.c \
    polarSSL/sha512.c \
    crypto.cpp \
    tests.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    aes.h \
    polarSSL/aes.h \
    polarSSL/config.h \
    polarSSL/check_config.h \
    polarSSL/padlock.h \
    polarSSL/aesni.h \
    polarSSL/platform.h \
    polarSSL/ctr_drbg.h \
    polarSSL/entrophy.h \
    polarSSL/sha512.h \
    catch.hpp \
    crypto.hpp

