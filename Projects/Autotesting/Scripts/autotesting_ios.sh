# !/bin/bash 
#  autotesting_ios.sh

# $1 - project folder name
# $2 - project name
# $3 - project configuration
# $4 - project target name
# $5 - certificate id
# $6 - device id

echo "autotesting_ios.sh"
sh autotesting_init.sh

echo "rm -rf ../../../../$1/Autotesting"
rm -rf ../../../../$1/Autotesting

echo "mkdir ../../../../$1/Autotesting"
mkdir -p ../../../../$1/Autotesting

echo "copy scripts for iOS deployment to Autotesting"
cp -Rf deployRunApp.sh ../../../../$1/Autotesting/deployRunApp.sh
cp -Rf floatsign.sh ../../../../$1/Autotesting/floatsign.sh
cp -Rf transporter_chief.rb ../../../../$1/Autotesting/transporter_chief.rb
cp -Rf testRun.js ../../../../$1/Autotesting/testRun.js

echo "cd ../../../../$1"
cd ../../../../$1

# for each test: 
for filename in ./Tests/*
do
   echo "copy $filename"
   # copy test scenario from Tests folder into autotesting.yaml in Data/Tests
   cp -Rf $filename ./Data/Tests/autotesting.yaml
   echo "xcodebuild -project $2.xcodeproj -configuration $3 -target $4"
   xcodebuild -project $2.xcodeproj -configuration $3 -target $4 
   
   echo "run and wait until finishes"

   echo "cd Autotesting"
   cd Autotesting
   echo "sh deployRunApp.sh $3 $4 $5 $6"
   sh deployRunApp.sh $3 $4 $5 $6
   echo "cd .."
   cd ..
done
cd ../dava.framework/Projects/Autotesting/Scripts

sh autotesting_finish.sh
echo "autotesting_macos.sh finished"