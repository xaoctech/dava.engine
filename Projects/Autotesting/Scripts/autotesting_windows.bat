echo off
echo "autotesting_windows.bat"
echo "go to %1"
cd ..\..\..\..\%1
echo "copy %3\%2.exe to current folder"
copy /Y %3\%2.exe %2.exe
for /f %%f in ('dir /b Tests\') do (
echo "copy Tests\%%f to Data\Tests\autotesting.yaml"
copy /Y Tests\%%f Data\Tests\autotesting.yaml
echo "run %2.exe"
start /WAIT %2.exe
)
del /F %2.exe
del /F Data\Tests\autotesting.yaml

echo "go to dava.framework"
cd ..\dava.framework\Projects\Autotesting\Scripts

echo "TODO: create report"
echo "autotesting_windows.bat finished"