#-------------------------------------------------
#
# Project created by QtCreator 2012-06-30T09:38:03
#
#-------------------------------------------------

QT       -= gui

TARGET = Box2D_Qt
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$PWD

INCLUDEPATH += $$PWD/../../../../Sources/External


SOURCES += \
    ../../../../Sources/External/Box2D/Collision/*.cpp \
    ../../../../Sources/External/Box2D/Collision/Shapes/*.cpp \
    ../../../../Sources/External/Box2D/Common/*.cpp \
    ../../../../Sources/External/Box2D/Dynamics/*.cpp \
    ../../../../Sources/External/Box2D/Dynamics/Contacts/*.cpp \
    ../../../../Sources/External/Box2D/Dynamics/Joints/*.cpp \
    ../../../../Sources/External/Box2D/Rope/*.cpp \

HEADERS += \
    ../../../../Sources/External/Box2D/Collision/*.h \
    ../../../../Sources/External/Box2D/Collision/Shapes/*.h \
    ../../../../Sources/External/Box2D/Common/*.h \
    ../../../../Sources/External/Box2D/Dynamics/*.h \
    ../../../../Sources/External/Box2D/Dynamics/Contacts/*.h \
    ../../../../Sources/External/Box2D/Dynamics/Joints/*.h \
    ../../../../Sources/External/Box2D/Rope/*.h \



win32 {

}


macx {

CONFIG += x86 x86_64

QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.7.sdk

LIBS += -framework Cocoa
}


