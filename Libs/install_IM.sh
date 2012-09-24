#!/bin/bash

installPath="ImageMagick-6.7.4"
imPath="ImageMagick-6.7.4-10"

rm -rf $imPath
rm -rf $installPath

tar -xf $imPath.tar.gz

currPath=`pwd`

mkdir -p $currPath/$installPath/delegates
mkdir $currPath/$installPath/delegates/lib
mkdir $currPath/$installPath/delegates/include

cp $currPath/libs/libpng_macos.a $currPath/$installPath/delegates/lib/libpng-static.a
cp $currPath/include/libpng/*.h $currPath/$installPath/delegates/include/

cd $imPath

./configure --prefix=$currPath/$installPath --disable-shared --without-dps --without-djvu --without-tiff --without-jpeg --without-fontconfig --without-gslib --without-gvc --without-lcms --without-lcms2 --without-lqr --without-lzma --without-openexr --without-rsvg --without-webp --without-wmf --without-xml --disable-openmp --disable-opencl --with-x=no --with-png=yes CPPFLAGS=-I$currPath/$installPath/delegates/include LDFLAGS=-L$currPath/$installPath/delegates/lib
make
make install
cd ..

rm -rf $imPath

