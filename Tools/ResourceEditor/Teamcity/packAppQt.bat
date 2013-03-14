cd ..\_ReleaseQt

if exist app rmdir /s /q app
if exist app.zip del /q app.zip

mkdir app\ResourceEditor\dava.framework\Tools\ResourceEditor\Data
mkdir app\ResourceEditor\dava.resourceeditor.beast
call git log --since=3.days --branches="development" --pretty=format:"%%%%s (%%%%an, %%%%ar) " >> app/ResourceEditor/changes.txt
echo cd .\dava.framework\Tools\ResourceEditor > app/ResourceEditor/start.cmd
echo start ResourceEditorQtVS2010.exe >> app/ResourceEditor/start.cmd

xcopy /e ..\Data\*.* app\ResourceEditor\dava.framework\Tools\ResourceEditor\Data 
xcopy *.exe app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\glew32.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\Teamcity\imagesplitter\*.bat app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT_HOME%\lib\QtCore4.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT_HOME%\lib\QtGui4.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\..\..\..\dava.resourceeditor.beast\beast\bin\beast32.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy /e ..\..\..\..\dava.resourceeditor.beast\*.* app\ResourceEditor\dava.resourceeditor.beast\

wzzip -p -r ResourceEditor_win_%1.zip app\*.*	