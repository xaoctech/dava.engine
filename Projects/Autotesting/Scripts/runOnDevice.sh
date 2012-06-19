# !/bin/bash

# $1 - target product name
# $2 - device id

echo "run $1 on device $2"
instruments -w $2 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$1" -e UIASCRIPT testRun.js