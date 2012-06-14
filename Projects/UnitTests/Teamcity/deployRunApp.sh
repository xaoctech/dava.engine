# !/bin/bash
echo "copy UnitTests.app"
cp -Rf "../build/Release-iphoneos/UnitTests.app" "./"

echo "sign and create PerfomanceTest.ipa"
sh floatsign.sh UnitTests.app 4L7VSNH4R3 UnitTests.ipa

echo "deploy ipa on device"
./transporter_chief.rb UnitTests.ipa

echo "run app on device"
instruments -w 3a80111143a4bf963e27ea94197a85d33039cec9 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "UnitTests" -e UIASCRIPT testRun.js

#del temporary files
rm -rf ./UnitTests.*
rm -rf ./fruitstrap
rm -rf ./instrumentscli*
