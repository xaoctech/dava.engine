#include "NodePropertyControl.h"
#include "ControlsFactory.h"


NodePropertyControl::NodePropertyControl(const Rect & rect)
    :   UIControl(rect)
{
    nodeDelegate = NULL;
    
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


void NodePropertyControl::SetDelegate(NodePropertyDelegate *delegate)
{
    nodeDelegate = delegate;
}

void NodePropertyControl::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}
void NodePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}
void NodePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}
void NodePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}
void NodePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}
void NodePropertyControl::OnItemIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}



