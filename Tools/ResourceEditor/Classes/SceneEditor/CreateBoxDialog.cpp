#include "CreateBoxDialog.h"
#include "ControlsFactory.h"

#include "BoxPropertyControl.h"

CreateBoxDialog::CreateBoxDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new BoxPropertyControl(propertyRect);
    propertyList->InitProperties();
    AddControl(propertyList);
    
    SetHeader(L"Create Box");
}
    
void CreateBoxDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new CubeNode(scene);

    CreateNodeDialog::CreateNode();
}