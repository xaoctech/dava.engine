cd Launcher-OSX-Release

rm -r Launcher.app/Contents/Frameworks/
mkdir Launcher.app/Contents/Frameworks

cp -R ../Libs/* Launcher.app/Contents/Frameworks

install_name_tool -change /Users/adebt/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/QtCore \
 @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
 Launcher.app/Contents/MacOS/Launcher

install_name_tool -change /Users/adebt/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtGui.framework/Versions/4/QtGui \
 @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
 Launcher.app/Contents/MacOS/Launcher

install_name_tool -change /Users/adebt/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtNetwork.framework/Versions/4/QtNetwork \
 @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork \
 Launcher.app/Contents/MacOS/Launcher

install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui \
 Launcher.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui 

install_name_tool -change /Users/adebt/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/QtCore \
 @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
 Launcher.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui 

install_name_tool -id @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork \
 Launcher.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork 

install_name_tool -change /Users/adebt/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/QtCore \
 @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
 Launcher.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork 

install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
 Launcher.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore

otool -L Launcher.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
otool -L Launcher.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui 
otool -L Launcher.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork 
otool -L Launcher.app/Contents/MacOS/Launcher