#!/bin/sh

echo "*****************   START   ****************"

echo "\$0=$0"
cd `dirname $0`

echo " "

echo "PWD=`pwd`"

#export NDK_MODULE_PATH=`pwd`/jni:$SDK_ROOT/External:$SDK_ROOT/External/Yaml:$SDK_ROOT/External/xmllib:$SDK_ROOT/External/pnglib:$SDK_ROOT/External/Box2D:$SDK_ROOT/External/Freetype:$SDK_ROOT/External/zip:$SDK_ROOT/Internal:$SDK_ROOT

export NDK_MODULE_PATH=`pwd`/jni

echo ""
#echo "NDK_MODULE_PATH=$NDK_MODULE_PATH"
echo ""

ndk-build NDK_DEBUG=0
if [ $? != 0 ]; then
    echo "ERROR: Can't build test program!"
    exit 1
fi

cp $NDK_MODULE_PATH/../obj/local/armeabi/libfreetype_android.a $NDK_MODULE_PATH/../../../libs/

echo "*****************   FINISH   ****************"
