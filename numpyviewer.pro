#-------------------------------------------------
#
# Project created by QtCreator 2017-10-28T13:53:52
#
#-------------------------------------------------

QT       += core gui network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = numpyviewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
    src/cnpy.cpp \
    src/keyeventhandler.cpp \
    src/histogram.cpp \
    src/envi_parser.cpp \
    src/convertwindow.cpp \
    src/INIReader.cpp \
    src/ini.c \
    src/graphics_view_zoom.cpp \
    src/filedownloader.cpp

HEADERS += \
        src/mainwindow.h \
    src/cnpy.h \
    src/keyeventhandler.h \
    src/histogram.h \
    src/envi_parser.h \
    src/convertwindow.h \
    src/ini.h \
    src/INIReader.h \
    src/colormap.h \
    src/graphics_view_zoom.h \
    src/filedownloader.h

FORMS += \
        mainwindow.ui \
    histogram.ui \
    convertwindow.ui

DISTFILES += \
    ../../../../64.png \
    appicon.rc \
    artwork/Screenshot mainwindow.png \
    artwork/Screenshot spectogram.png

RESOURCES += \
    icons.qrc

QT += charts

ICON = artwork/icon.icns

# macOS / Linux
#INCLUDEPATH += "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/"

# AppImage / Linux support
!win32 {
    LIBS += -lz
}
CONFIG += c++11

# Windows
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

RC_FILE = appicon.rc

target.path = /usr/local/bin/
INSTALLS += target
