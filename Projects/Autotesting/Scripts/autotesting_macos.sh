# !/bin/bash 
#  autotesting_macos.sh

echo "autotesting_macos.sh"
sh autotesting_init.sh
# for each test: 
cd ../../../../$1
for filename in ./Tests/*
do
   # copy test scenario from Tests folder into autotesting.yaml in Data/Tests
   cp -Rf filename ./Data/Tests/autotesting.yaml
   # build project
   xcodebuild -project $2.xcodeproj -configuration $3 -alltargets
   # run and wait until finishes
   open -W ./build/$3/$2.app 
done
sh autotesting_finish.sh
echo "autotesting_macos.sh finished"