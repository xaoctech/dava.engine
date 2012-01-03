#include "CreateLandscapeDialog.h"
#include "ControlsFactory.h"

#include "LandscapePropertyControl.h"

CreateLandscapeDialog::CreateLandscapeDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new LandscapePropertyControl(propertyRect);
    propertyList->InitProperties();
    AddControl(propertyList);

    
    KeyedArchive *keyedArchieve = new KeyedArchive();
    keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
    String path = keyedArchieve->GetString("LastSavedPath", "/");
    ((LandscapePropertyControl *)propertyList)->SetProjectPath(path);
    SafeRelease(keyedArchieve);

    
    SetHeader(L"Create Landscape");
}
    
void CreateLandscapeDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new LandscapeNode(scene);

    CreateNodeDialog::CreateNode();
}
