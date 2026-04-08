INCLUDEPATH += $$PWD/dock
include($$PWD/dock/dock.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/mainwindow.h \
    $$PWD/optioncyclebutton.h

SOURCES += \
    $$PWD/mainwindow.cpp \
    $$PWD/optioncyclebutton.cpp
FORMS += \
    $$PWD/mainwindow.ui
