cd ..\_ReleaseQt

if exist app rmdir /s /q app
if exist app.zip del /q app.zip

mkdir app\1\2\3\Data
mkdir app\dava.resourceeditor.beast
call git log --since=3.days --branches="development" --pretty=format:"%%%%s (%%%%an, %%%%ar) " >> app/changes.txt
xcopy /e ..\Data\*.* app\1\2\3\Data 
xcopy *.exe app\1\2\3
xcopy ..\glew32.dll app\1\2\3
xcopy ..\Teamcity\imagesplitter\*.bat app\1\2\3
xcopy %QT_HOME%\Desktop\Qt\4.8.1\msvc2010\lib\QtCore4.dll app\1\2\3
xcopy %QT_HOME%\Desktop\Qt\4.8.1\msvc2010\lib\QtGui4.dll app\1\2\3
xcopy ..\..\..\..\dava.resourceeditor.beast\beast\bin\beast32.dll app\1\2\3
xcopy /e ..\..\..\..\dava.resourceeditor.beast\*.* app\dava.resourceeditor.beast

wzzip -p -r ResourceEditor_win_%1.zip app\*.*	