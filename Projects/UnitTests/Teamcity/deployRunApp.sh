# !/bin/bash
echo "copy UnitTests.app"
APP=UnitTests
DIR_APP="../DerivedData/TemplateProjectiPhone/Build/Products/Release-iphoneos"

if [ ! -d $DIR_APP/$APP.app ]; then
 echo "directory $DIR_APP/$APP.app doesn't exist"
 exit 1
fi
cp -Rf $DIR_APP/$APP.app "./"

echo "sign and create $APP.ipa"
sh floatsign.sh $APP.app 4L7VSNH4R3 $APP.ipa
sh floatsign.sh $APP.app 4L7VSNH4R2 $DIR_APP/$APP.ipa

if [ ! -f $APP.ipa ]; then
  echo "$APP.ipa wasn't created"
  exit 1
fi


echo "deploy ipa on device"
./transporter_chief.rb $APP.ipa

echo "run app on device"
instruments -w 3a80111143a4bf963e27ea94197a85d33039cec9 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$APP" -e UIASCRIPT testRun.js

#del temporary files
rm -rf ./$APP.*
#rm -rf ./fruitstrap
rm -rf ./instrumentscli*
