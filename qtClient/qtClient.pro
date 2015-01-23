TEMPLATE = app
CONFIG += console
CONFIG += app_bundle
CONFIG += qt
CONFIG += c++11

QT += network
QT += widgets

SOURCES += main.cpp \
    Client.cpp \
    Server.cpp \
    inputthread.cpp \
    ../crypto_crt/crypto.cpp \
    polarSSL/aes.c \
    polarSSL/aesni.c \
    polarSSL/padlock.c \
    polarssl/ctr_drbg.c \
    polarSSL/entropy.c \
    polarssl/entropy_poll.c \
    polarssl/sha256.c \
    polarssl/sha512.c \
    polarssl/timing.c \
    tableone.cpp
include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    Client.h \
    Server.h \
    inputthread.h \
    ../ProtocolMsg.h \
    ../crypto_crt/crypto.hpp \
    polarSSL/aes.h \
    polarSSL/aesni.h \
    polarSSL/check_config.h \
    polarSSL/config.h \
    polarSSL/padlock.h \
    polarSSL/platform.h \
    polarssl/ctr_drbg.h \
    polarssl/entropy_poll.h \
    polarSSL/entropy.h \
    polarssl/havege.h \
    polarssl/sha256.h \
    polarssl/sha512.h \
    polarssl/timing.c \
    tableone.h
