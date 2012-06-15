cd ..\Release

if exist app rmdir /s /q app
if exist app.zip del /q app.zip

mkdir app\Data\
xcopy /e ..\Data\*.* app\Data\ 
xcopy PerformanceTestVS2010.exe app\
xcopy ..\glew32.dll app\

wzzip -p -r app.zip app\*.*