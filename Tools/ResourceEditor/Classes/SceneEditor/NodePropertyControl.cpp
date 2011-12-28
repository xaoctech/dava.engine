#include "NodePropertyControl.h"
#include "ControlsFactory.h"


NodePropertyControl::NodePropertyControl(const Rect & rect)
    :   UIControl(rect)
{
    propertyList = new PropertyList(Rect(0, 0, rect.dx, rect.dy), this);
    AddControl(propertyList);
}
    
NodePropertyControl::~NodePropertyControl()
{
    SafeRelease(propertyList);
}

void NodePropertyControl::WillAppear()
{
}

void NodePropertyControl::InitProperties()
{
    propertyList->AddStringProperty("Name", "SceneNode", PropertyList::PROPERTY_IS_EDITABLE);
}

void NodePropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "SceneNode");
}

void NodePropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    propertyList->SetStringPropertyValue("Name", sceneNode->GetName());
}

void NodePropertyControl::ReadToNode(SceneNode *sceneNode)
{
    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
}


