TEMPLATE = app
TARGET = binarycreator
INCLUDEPATH += . .. rcc ../common

include(../../installerfw.pri)

QT -= gui
QT += script

CONFIG += console
DESTDIR = $$IFW_APP_PATH

SOURCES = binarycreator.cpp \
          rcc/rcc.cpp \
          rcc/rccmain.cpp \
          ../common/repositorygen.cpp
HEADERS = rcc/rcc.h
RESOURCES += binarycreator.qrc
