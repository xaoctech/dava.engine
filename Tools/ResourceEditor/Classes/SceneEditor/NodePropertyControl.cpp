#include "NodePropertyControl.h"
#include "ControlsFactory.h"


NodePropertyControl::NodePropertyControl(const Rect & rect, bool _showMatrix)
    :   UIControl(rect)
{
    showMatrix = _showMatrix;
    
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
    propertyList->AddStringProperty("Retain Count", "1", PropertyList::PROPERTY_IS_READ_ONLY);
    propertyList->AddStringProperty("Class Name", "Unknown", PropertyList::PROPERTY_IS_READ_ONLY);
    propertyList->AddStringProperty("C++ Class Name", "Unknown", PropertyList::PROPERTY_IS_READ_ONLY);
    
    if(showMatrix)
    {
        Matrix4 matrix;
        propertyList->AddMatrix4Property("Local Matrix", matrix, PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddMatrix4Property("World Matrix", matrix, PropertyList::PROPERTY_IS_READ_ONLY);
    }
}

void NodePropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "SceneNode");
    propertyList->SetStringPropertyValue("Retain Count", "1");
    propertyList->SetStringPropertyValue("Class Name", "Unknown");
    propertyList->SetStringPropertyValue("C++ Class Name", "Unknown");
    
    if(showMatrix)
    {
        Matrix4 matrix;
        propertyList->SetMatrix4PropertyValue("Local Matrix", matrix);
        propertyList->SetMatrix4PropertyValue("World Matrix", matrix);
    }
}

void NodePropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    propertyList->SetStringPropertyValue("Name", sceneNode->GetName());
    propertyList->SetStringPropertyValue("Retain Count", Format("%d", sceneNode->GetRetainCount()));
    propertyList->SetStringPropertyValue("Class Name", sceneNode->GetClassName());
    propertyList->SetStringPropertyValue("C++ Class Name", typeid(*sceneNode).name());
    
    if(showMatrix)
    {
        propertyList->SetMatrix4PropertyValue("Local Matrix", sceneNode->GetLocalTransform());
        propertyList->SetMatrix4PropertyValue("World Matrix", sceneNode->GetWorldTransform());
    }
}

void NodePropertyControl::ReadToNode(SceneNode *sceneNode)
{
    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
 
    if(showMatrix)
    {
        sceneNode->SetLocalTransform(propertyList->GetMatrix4PropertyValue("Local Matrix"));
        //    sceneNode->SetWordTransform(propertyList->GetMatrix4PropertyValue("World Matrix"));
    }
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

void NodePropertyControl::OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4)
{
    if(nodeDelegate)
    {
        nodeDelegate->NodePropertyChanged();
    }
}



