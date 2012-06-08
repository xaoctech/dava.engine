#!/bin/bash
#  copy_test.sh

echo "copy_test.sh"
echo $1
cp -Rf ../../../../$1/Tests/autotesting1.yaml ../../../../$1/Data/Tests/autotesting.yaml
echo "copied ../../../../$1/Tests/autotesting1.yaml to ../../../../$1/Data/Tests/autotesting.yaml"
echo "copy_test.sh done"