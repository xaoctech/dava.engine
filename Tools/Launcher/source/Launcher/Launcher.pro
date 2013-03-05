#-------------------------------------------------
#
# Project created by QtCreator 2012-09-25T11:44:21
#
#-------------------------------------------------


DEFINES += LAUNCER_VER=\\\"0.7\\\"

QT       += core gui network

TARGET = Launcher
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    selfupdater.cpp \
    settings.cpp \
    configDownload.cpp \
    logger.cpp \
    directorymanager.cpp \
    ziphelper.cpp \
    installer.cpp \
    processhelper.cpp

HEADERS  += mainwindow.h \
    selfupdater.h \
    settings.h \
    configDownload.h \
    logger.h \
    directorymanager.h \
    ziphelper.h \
    installer.h \
    AppType.h \
    processhelper.h

FORMS    += mainwindow.ui

macx: LIBS += -L$$PWD/yaml-cpp/libs/ -lyaml-cpp_osx
macx: PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cpp_osx.a
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/yaml-cpp/libs/ -llibyaml-cppmd -luser32 -lshell32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/yaml-cpp/libs/ -llibyaml-cppmdd -luser32 -lshell32
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cppmd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cppmdd.lib
INCLUDEPATH += $$PWD/yaml-cpp/include
DEPENDPATH += $$PWD/yaml-cpp/include

#win32: LIBS += -L$$WINDOWSSDKDI/Lib -lPsapi
win32: LIBS += -L$$WINDOWSSDKDI/Lib -lUser32
mac: LIBS += -F//System/Library/Frameworks/ -framework ApplicationServices
mac: INCLUDEPATH += /System/Library/Frameworks/ApplicationServices.framework/Frameworks/HIServices.framework/Headers/
mac: DEPENDPATH += /System/Library/Frameworks/ApplicationServices.framework/Frameworks/HIServices.framework/Headers/


win32: INCLUDEPATH += $$PWD/../../../../Libs/include/libpng/
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/quazip-msvc2010/release/ -lquazip1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/quazip-msvc2010/debug/ -lquazip1
mac: LIBS += -lz
mac: HEADERS += \
    quazip/JlCompress.h \
    quazip/zip.h \
    quazip/unzip.h \
    quazip/quazipnewinfo.h \
    quazip/quazipfileinfo.h \
    quazip/quazipfile.h \
    quazip/quazipdir.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quaziodevice.h \
    quazip/quagzipfile.h \
    quazip/quacrc32.h \
    quazip/quachecksum32.h \
    quazip/quaadler32.h \
    quazip/ioapi.h
mac: SOURCES += \
    quazip/quazipfile.cpp \
    quazip/quazip.cpp \
    quazip/qioapi.cpp \
    quazip/JlCompress.cpp \
    quazip/zip.c \
    quazip/unzip.c \
    quazip/quazipnewinfo.cpp \
    quazip/quazipdir.cpp \
    quazip/quaziodevice.cpp \
    quazip/quagzipfile.cpp \
    quazip/quacrc32.cpp \
    quazip/quaadler32.cpp
