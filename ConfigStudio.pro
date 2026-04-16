QT       += core gui
QT += widgets
QT += quick quickwidgets qml
QT += serialport

TEMPLATE = app
TARGET = ConfigStudio
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

LIBS += -lqwt

SOURCES += \
    main.cpp

INCLUDEPATH += $$PWD/app
include($$PWD/app/app.pri)
INCLUDEPATH += $$PWD/ui
include($$PWD/ui/ui.pri)
INCLUDEPATH += $$PWD/canvas
include($$PWD/canvas/canvas.pri)
INCLUDEPATH += $$PWD/components
include($$PWD/components/components.pri)
INCLUDEPATH += $$PWD/model
include($$PWD/model/model.pri)
INCLUDEPATH += $$PWD/runtime
include($$PWD/runtime/runtime.pri)
INCLUDEPATH += $$PWD/commands
include($$PWD/commands/commands.pri)
INCLUDEPATH += $$PWD/utils
include($$PWD/utils/utils.pri)
INCLUDEPATH += $$PWD/virtualkey
include($$PWD/virtualkey/virtualkey.pri)

INCLUDEPATH += $$PWD/fullscreen
include($$PWD/fullscreen/fullscreen.pri)

INCLUDEPATH += $$PWD/datasrc
include($$PWD/datasrc/datasrc.pri)

RESOURCES += resources/resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /userdata
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Keyboard.qml
