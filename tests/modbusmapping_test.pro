QT += core testlib serialport
CONFIG += c++17 testcase console
TEMPLATE = app
TARGET = modbusmapping_test

INCLUDEPATH += ../datasrc

SOURCES += \
    modbusmapping_test.cpp \
    ../datasrc/modbusmappingdefs.cpp

HEADERS += \
    ../datasrc/modbusmappingdefs.h
