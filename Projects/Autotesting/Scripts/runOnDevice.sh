# !/bin/bash

# $1 - target product name

echo "get udid device" 
UDID_device=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' -e '/iPod/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
echo "udid is ${UDID_device}"

echo "run $1 on device"
instruments -w ${UDID_device} -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$1" -e UIASCRIPT testRun.js