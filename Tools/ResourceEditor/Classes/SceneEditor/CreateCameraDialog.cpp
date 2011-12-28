#include "CreateCameraDialog.h"
#include "ControlsFactory.h"

#include "CameraPropertyControl.h"

CreateCameraDialog::CreateCameraDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    propertyList = new CameraPropertyControl(propertyRect);
    propertyList->InitProperties();
    AddControl(propertyList);

    SetHeader(L"Create Camera");
}
    
void CreateCameraDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new Camera(scene);
    
    CreateNodeDialog::CreateNode();
}
