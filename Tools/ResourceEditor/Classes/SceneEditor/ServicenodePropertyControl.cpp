#include "ServicenodePropertyControl.h"

ServicenodePropertyControl::ServicenodePropertyControl(const Rect & rect, bool showMatrix)
:   NodePropertyControl(rect, showMatrix)
{
    
}

void ServicenodePropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    propertyList->SetStringPropertyValue("Name", "Service node");
}

void ServicenodePropertyControl::ReadFrom(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFrom(sceneNode);
}

void ServicenodePropertyControl::WriteTo(SceneNode *sceneNode)
{
    NodePropertyControl::WriteTo(sceneNode);
}


