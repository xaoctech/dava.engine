#include "CreateLightDialog.h"
#include "ControlsFactory.h"

#include "LightPropertyControl.h"

CreateLightDialog::CreateLightDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new LightPropertyControl(propertyRect);
    propertyList->InitProperties();
    AddControl(propertyList);

    SetHeader(L"Create Light");
}
    
void CreateLightDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new LightNode(scene);

    CreateNodeDialog::CreateNode();
}
