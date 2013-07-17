# !/bin/bash

# $1 - target product name

echo "get udid device" 

if [[ -n $2 ]]
then
	UDID_device=`~/AIRSDK_Compiler/bin/adt -devices -platform iOS | grep "$2" | awk '{print $3}'`
else
	UDID_device=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' -e '/iPod/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
fi

echo "udid is ${UDID_device}"

echo "run $1 on device"
instruments -w ${UDID_device} -t $PATH_TO_AUTO_TEMPL/Automation.tracetemplate "$1" -e UIASCRIPT testRun.js