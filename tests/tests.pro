QT += core gui widgets testlib quick quickwidgets qml serialport
CONFIG += c++17 testcase console
TEMPLATE = app
TARGET = tst_phase1_datasource

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ..
INCLUDEPATH += ../app
include(../app/app.pri)
INCLUDEPATH += ../ui
include(../ui/ui.pri)
INCLUDEPATH += ../canvas
include(../canvas/canvas.pri)
INCLUDEPATH += ../model
include(../model/model.pri)
INCLUDEPATH += ../runtime
include(../runtime/runtime.pri)
INCLUDEPATH += ../commands
include(../commands/commands.pri)
INCLUDEPATH += ../utils
include(../utils/utils.pri)
INCLUDEPATH += ../virtualkey
include(../virtualkey/virtualkey.pri)
INCLUDEPATH += ../fullscreen
include(../fullscreen/fullscreen.pri)
INCLUDEPATH += ../datasrc
include(../datasrc/datasrc.pri)

SOURCES += \
    componentfactory_stub.cpp \
    tst_phase1_datasource.cpp

RESOURCES += ../resources/resources.qrc
