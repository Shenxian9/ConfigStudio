QT += core testlib
CONFIG += c++17 testcase console
TEMPLATE = app
TARGET = variablemodel_test

SOURCES += \
    variablemodel_test.cpp \
    ../model/variablemodel.cpp

HEADERS += \
    ../model/variablemodel.h \
    ../model/Variable.h
