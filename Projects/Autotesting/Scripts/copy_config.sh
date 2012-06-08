#!/bin/bash
#  copy_config.sh

echo "copy_config.sh"
cp -Rf ../../../Sources/Internal/Autotesting/Config.h ../Data/Config_backup.h
cp -Rf ../Data/Config.h ../../../Sources/Internal/Autotesting/Config.h
echo "copy_config.sh done"