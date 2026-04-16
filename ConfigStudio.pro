QT       += core gui
QT += widgets
QT += quick quickwidgets qml
QT += serialport

TEMPLATE = app
TARGET = ConfigStudio
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


# INCLUDEPATH += /usr/include/qwt
# LIBS += -lqwt-qt5

# INCLUDEPATH += /usr/include/qwt
LIBS += -lqwt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp

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


INCLUDEPATH += $$PWD/fullscreen
include($$PWD/fullscreen/fullscreen.pri)


INCLUDEPATH += $$PWD/datasrc
include($$PWD/datasrc/datasrc.pri)

RESOURCES += resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /userdata
!isEmpty(target.path): INSTALLS += target


