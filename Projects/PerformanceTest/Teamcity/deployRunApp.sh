# !/bin/bash
echo "copy PerfomanceTest.app"
cp -Rf "../build/Release-iphoneos/PerfomanceTest.app" "./"

echo "sign and create PerfomanceTest.ipa"
sh floatsign.sh PerfomanceTest.app 4L7VSNH4R3 PerfomanceTest.ipa

echo "deploy ipa on device"
./transporter_chief.rb PerfomanceTest.ipa

echo "run app on device"
instruments -w 3a80111143a4bf963e27ea94197a85d33039cec9 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "PerfomanceTest" -e UIASCRIPT testRun.js

#del temporary files
rm -rf ./PerfomanceTest.*
rm -rf ./fruitstrap
rm -rf ./instrumentscli*
