QT       += core gui
QT += widgets
QT += quick quickwidgets qml

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
    main.cpp \
#     mainwindow.cpp

# HEADERS += \
#     mainwindow.h

# FORMS += \
#     mainwindow.ui

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
# INCLUDEPATH += $$PWD/virtualkey
# include($$PWD/virtualkey/virtualkey.pri)


INCLUDEPATH += $$PWD/fullscreen
include($$PWD/fullscreen/fullscreen.pri)


INCLUDEPATH += $$PWD/datasrc
include($$PWD/datasrc/datasrc.pri)

RESOURCES += resources/resources.qrc

# 配置远程部署路径
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# DISTFILES += \
#     Keyboard.qml

