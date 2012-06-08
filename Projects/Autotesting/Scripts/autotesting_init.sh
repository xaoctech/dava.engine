# !/bin/bash 
#  autotesting_init.sh

echo "autotesting init"
sh copy_config.sh
sh copy_test.sh $1
