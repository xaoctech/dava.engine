# !/bin/bash

# $1 - target product name

echo "get udid device" 
UDID_device=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' -e '/iPod/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
echo "udid is ${UDID_device}"

echo "run $1 on device"
instruments -w ${UDID_device} -t $PATH_TO_AUTO_TEMPL/Automation.tracetemplate "$1" -e UIASCRIPT testRun.js