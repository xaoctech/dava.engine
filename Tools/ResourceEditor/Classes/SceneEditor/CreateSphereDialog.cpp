#include "CreateSphereDialog.h"
#include "ControlsFactory.h"

#include "SpherePropertyControl.h"

CreateSphereDialog::CreateSphereDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new SpherePropertyControl(propertyRect, false);
    propertyList->InitProperties();
    AddControl(propertyList);

    SetHeader(L"Create Sphere");
}
    
void CreateSphereDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new SphereNode(scene);
    
    CreateNodeDialog::CreateNode();
}
