#include "ControlNode.h"

#include "UIControls/UIEditorComponent.h"

using namespace DAVA;

class ControlPropertiesRoot : public BaseProperty
{
public:
    virtual String GetName() const {
        return "ROOT";
    }
    
    virtual ePropertyType GetType() const {
        return TYPE_HEADER;
    }
    
};

ControlNode::ControlNode(UIControl *control) : PackageBaseNode(NULL), control(SafeRetain(control)), propertiesRoot(NULL), editable(true)
{
    propertiesRoot = new ControlPropertiesRoot();
//    const List<UIControl*> &children = control->GetChildren();
//    for (auto it = children.begin(); it != children.end(); ++it)
//    {
//        ControlNode *node = new ControlNode(*it, editable);
//        Add(node);
//        SafeRelease(node);
//    }
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(propertiesRoot);
}

void ControlNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
}

int ControlNode::GetCount() const
{
    return (int) nodes.size();
}

PackageBaseNode *ControlNode::Get(int index) const
{
    return nodes[index];
}

ControlNode *ControlNode::FindByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return NULL;
}

ControlNode *ControlNode::Clone() const
{
    return CloneNode(this);
}

String ControlNode::GetName() const
{
    return control->GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

bool ControlNode::IsInstancedFromPrototype() const
{
    UIEditorComponent *component = static_cast<UIEditorComponent*>(control->GetCustomData());
    return component && component->GetPrototype() != NULL;
}

bool ControlNode::IsCloned() const
{
    UIEditorComponent *component = static_cast<UIEditorComponent*>(control->GetCustomData());
    return component && component->IsClonedFromPrototype();
}

ControlNode *ControlNode::CloneNode(const ControlNode *node) const
{
    UIControl *sourceControl = node->GetControl();
    UIControl *clonedControl = ObjectFactory::Instance()->New<UIControl>(sourceControl->GetControlClassName());
    clonedControl->SetCustomControlClassName(sourceControl->GetCustomControlClassName());
    
    ControlNode *clonedNode = new ControlNode(clonedControl);
    SafeRelease(clonedControl);
    
    // TODO: Apply properties
    
    for (auto it = node->nodes.begin(); it != node->nodes.end(); ++it)
    {
        ControlNode *child = CloneNode(*it);
        clonedNode->Add(child);
        SafeRelease(child);
    }
    
    return clonedNode;
}
