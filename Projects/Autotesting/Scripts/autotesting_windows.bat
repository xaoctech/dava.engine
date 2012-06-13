echo off
echo "autotesting_windows.bat"
echo "go to %1"
cd ..\..\..\..\%1
echo "copy %3\%2.exe to current folder"
copy /Y %3\%2.exe %2.exe
echo "create Data\Autotesting"
md Data\Autotesting
md Data\Autotesting\Actions
md Data\Autotesting\Tests

cd Data\Autotesting
del /f id.txt
echo %random%>> id.txt
cd ..\..

for /f %%f in ('dir /b Autotesting\Tests\') do (
echo "copy .\Autotesting\Tests\%%f to .\Data\Autotesting\Tests\%%f"
copy /Y .\Autotesting\Tests\%%f .\Data\Autotesting\Tests\%%f
)
for /f %%f in ('dir /b Autotesting\Actions\') do (
echo "copy .\Autotesting\Actions\%%f to .\Data\Autotesting\Actions\%%f"
copy /Y .\Autotesting\Actions\%%f .\Data\Autotesting\Actions\%%f
)
for /f %%f in ('dir /b Autotesting\Tests\') do (
echo "run %2.exe"
start /WAIT %2.exe
)

echo "cleanup: delete Data\Autotesting and %2.exe"
rd /s /q Data\Autotesting
del /F %2.exe

echo "go to dava.framework"
cd ..\dava.framework\Projects\Autotesting\Scripts

echo "TODO: create report"
echo "autotesting_windows.bat finished"