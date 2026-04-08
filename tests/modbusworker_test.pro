QT += core testlib serialport
CONFIG += c++17 testcase console
TEMPLATE = app
TARGET = modbusworker_test

INCLUDEPATH += ../datasrc

SOURCES += \
    modbusworker_test.cpp \
    ../datasrc/modbusworker.cpp

HEADERS += \
    ../datasrc/modbusworker.h
