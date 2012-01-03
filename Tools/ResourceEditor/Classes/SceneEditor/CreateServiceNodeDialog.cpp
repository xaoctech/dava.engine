#include "CreateServiceNodeDialog.h"
#include "ControlsFactory.h"

#include "ServicenodePropertyControl.h"

CreateServiceNodeDialog::CreateServiceNodeDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new ServicenodePropertyControl(propertyRect, false);
    propertyList->InitProperties();
    AddControl(propertyList);

    SetHeader(L"Create Service Node");
}
    
void CreateServiceNodeDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new SceneNode(scene);
    
    CreateNodeDialog::CreateNode();
}
