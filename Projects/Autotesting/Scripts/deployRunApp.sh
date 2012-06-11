# !/bin/bash

# $1 - configuration
# $2 - target product name
# $3 - certificate id
# $4 - device id

echo "cd .."
cd ..

echo "rm -rf Autotesting/$2.app"
rm -rf Autotesting/$2.app

echo "copy build/$3-iphoneos/$2.app to Autotesting/$2.app"
cp -Rf build/$1-iphoneos/$2.app Autotesting/$2.app

echo "cd Autotesting"
cd Autotesting

echo "sign with $3 and create $2"
sh floatsign.sh $2.app $3 $2.ipa

echo "deploy $2.ipa on device"
./transporter_chief.rb $2.ipa

echo "run app on device $4"
instruments -w $4 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$2" -e UIASCRIPT testRun.js

echo "cd .."
cd ..

#del temporary files
#rm -rf ./AutoTest.*
#rm -rf ./fruitstrap
#rm -rf ./instrumentscli*