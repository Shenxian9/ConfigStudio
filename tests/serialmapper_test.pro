QT += core testlib serialport
CONFIG += c++17 testcase console
TEMPLATE = app
TARGET = serialmapper_test

INCLUDEPATH += ../model ../datasrc

SOURCES += \
    serialmapper_test.cpp \
    ../datasrc/serialdatasource.cpp \
    ../model/variablemodel.cpp

HEADERS += \
    ../datasrc/serialdatasource.h \
    ../model/variablemodel.h \
    ../model/Variable.h
