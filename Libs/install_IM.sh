#!/bin/bash

echo on
rm -rf ImageMagick-6.6.9-0/
rm -rf libpng-1.2.46/

tar -xf ImageMagick-6.6.9-0.tar.gz
tar -xf libpng-1.2.46.tar.gz

currPath=`pwd`
installPath="ImageMagick-6.6.9"
echo $currPath
cd libpng-1.2.46
./configure --prefix=$currPath/$installPath/delegates --enable-static --disable-shared
make
make install
cd ../ImageMagick-6.6.9-0

./configure --prefix=$currPath/$installPath --disable-shared --without-dps --without-djvu --without-fontconfig --without-freetype --without-gslib --without-gvc --without-lcms --without-lcms2 --without-lqr --without-lzma --without-openexr --without-rsvg --without-webp --without-wmf --without-xml --disable-openmp --disable-opencl --with-x=no CPPFLAGS=-I$currPath/$installPath/delegates/include LDFLAGS=-L$currPath/$installPath/delegates/lib
make
make install
cd ..

rm -rf libpng-1.2.46/
rm -rf ImageMagick-6.6.9-0/
