#-------------------------------------------------
#
# Project created by QtCreator 2012-06-29T17:09:03
#
#-------------------------------------------------

QT += network opengl multimedia
QT -= gui

TARGET = FrameworkQt
TEMPLATE = lib
CONFIG += staticlib

#DESTDIR = $$PWD

INCLUDEPATH += $$PWD/../../Sources/Internal
INCLUDEPATH += $$PWD/../../Sources/External
INCLUDEPATH += $$PWD/../../Sources/External/Freetype
INCLUDEPATH += $$PWD/../../Libs/oggvorbis/include
INCLUDEPATH += $$PWD/../../Libs/include

SOURCES += \
    ../../Sources/Internal/*.cpp  \
    ../../Sources/Internal/Animation/*.cpp  \
    ../../Sources/Internal/Base/*.cpp  \
    ../../Sources/Internal/Collision/*.cpp  \
    ../../Sources/Internal/Core/*.cpp  \
    ../../Sources/Internal/Database/*.cpp  \
    ../../Sources/Internal/Debug/*.cpp  \
    ../../Sources/Internal/DLC/*.cpp  \
    ../../Sources/Internal/Entity/*.cpp  \
    ../../Sources/Internal/FileSystem/*.cpp  \
    ../../Sources/Internal/Input/*.cpp  \
    ../../Sources/Internal/Math/*.cpp  \
    ../../Sources/Internal/Network/*.cpp  \
    ../../Sources/Internal/Particles/*.cpp  \
    ../../Sources/Internal/Platform/*.cpp  \
    ../../Sources/Internal/Render/*.cpp  \
    ../../Sources/Internal/Render/2D/*.cpp  \
    ../../Sources/Internal/Render/3D/*.cpp  \
    ../../Sources/Internal/Render/Effects/*.cpp  \
    ../../Sources/Internal/Scene2D/*.cpp  \
    ../../Sources/Internal/Scene3D/*.cpp  \
    ../../Sources/Internal/Sound/*.cpp  \
    ../../Sources/Internal/UI/*.cpp  \
    ../../Sources/Internal/Utils/*.cpp  \


HEADERS += \
    ../../Sources/Internal/*.h  \
    ../../Sources/Internal/Animation/*.h  \
    ../../Sources/Internal/Base/*.h  \
    ../../Sources/Internal/Collision/*.h  \
    ../../Sources/Internal/Core/*.h  \
    ../../Sources/Internal/Database/*.h  \
    ../../Sources/Internal/Debug/*.h  \
    ../../Sources/Internal/DLC/*.h  \
    ../../Sources/Internal/Entity/*.h  \
    ../../Sources/Internal/FileSystem/*.h  \
    ../../Sources/Internal/Input/*.h  \
    ../../Sources/Internal/Math/*.h  \
    ../../Sources/Internal/Network/*.h  \
    ../../Sources/Internal/Particles/*.h  \
    ../../Sources/Internal/Platform/*.h  \
    ../../Sources/Internal/Render/*.h  \
    ../../Sources/Internal/Render/2D/*.h  \
    ../../Sources/Internal/Render/3D/*.h  \
    ../../Sources/Internal/Render/Effects/*.h  \
    ../../Sources/Internal/Scene2D/*.h  \
    ../../Sources/Internal/Scene3D/*.h  \
    ../../Sources/Internal/Sound/*.h  \
    ../../Sources/Internal/UI/*.h  \
    ../../Sources/Internal/Utils/*.h  \


win32 {

SOURCES += \
    ../../Sources/Internal/Platform/TemplateWin32/*.cpp  \

HEADERS += \
    ../../Sources/Internal/Platform/TemplateWin32/*.h  \

}


macx {

SOURCES += \
#    ../../Sources/Internal/Platform/TemplateMacOS/*.cpp  \
    ../../Sources/Internal/Platform/Qt/MacOS/*.cpp  \

OBJECTIVE_SOURCES += \
    ../../Sources/Internal/FileSystem/FileSystemObjective.mm  \
    ../../Sources/Internal/FileSystem/LoggerMacOS.mm  \
    ../../Sources/Internal/Network/*.mm  \
    ../../Sources/Internal/Platform/ThreadMacOS.mm  \
#    ../../Sources/Internal/Platform/TemplateMacOS/*.mm  \
#    ../../Sources/Internal/Platform/TemplateMacOS/*.m  \
    ../../Sources/Internal/Render/CursorMacOS.mm  \
    ../../Sources/Internal/Utils/*.mm  \

HEADERS += \
#    ../../Sources/Internal/Platform/TemplateMacOS/*.h  \
    ../../Sources/Internal/Platform/Qt/MacOS/*.h  \
    ../../Sources/Internal/Platform/Qt/*.h  \


CONFIG += x86 x86_64

QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.7.sdk

LIBS += -framework Cocoa -framework OpenAL -framework ApplicationServices -framework AppKit -framework Foundation
LIBS += -L$$PWD/../../Libs/libs/ -lmongodb_macos
LIBS += -L$$PWD/../../Libs/libs/ -lpng_macos
LIBS += -L$$PWD/../../Libs/libs/ -lxml_macos
LIBS += -L$$PWD/../../Libs/libs/ -lyaml_macos
LIBS += -L$$PWD/../../Libs/libs/ -lcurl_ios_mac
LIBS += -L$$PWD/../../Libs/libs/ -ltheora_macos
LIBS += -L$$PWD/../../Libs/freetype/lib/ -lfreetype_macos
LIBS += -L$$PWD/../../Libs/oggvorbis/lib/ -lvorbisfile_macos
LIBS += -L$$PWD/../../Libs/oggvorbis/lib/ -lvorbis_macos
LIBS += -L$$PWD/../../Libs/oggvorbis/lib/ -logg_macos

INCLUDEPATH += $$PWD/../../Libs/libs
INCLUDEPATH += $$PWD/../../Libs/freetype/include
INCLUDEPATH += $$PWD/../../Libs/oggvorbis/include

DEPENDPATH += $$PWD/../../Libs/libs
DEPENDPATH += $$PWD/../../Libs/freetype/include
DEPENDPATH += $$PWD/../../Libs/oggvorbis/include

PRE_TARGETDEPS += $$PWD/../../Libs/libs/libmongodb_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libpng_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libxml_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libyaml_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libcurl_ios_mac.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libtheora_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/libs/libogg_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/freetype/lib/libfreetype_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/oggvorbis/lib/libvorbisfile_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/oggvorbis/lib/libvorbis_macos.a
PRE_TARGETDEPS += $$PWD/../../Libs/oggvorbis/lib/libogg_macos.a


DEFINES += DDARWIN_NO_CARBON FT2_BUILD_LIBRARY Q_OS_MAC
QMAKE_CXXFLAGS = -fvisibility=hidden
}


CONFIG(debug, debug|release) {
DEFINES += DAVA_DEBUG
}





