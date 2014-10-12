#include "ControlNode.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control) : PackageBaseNode(NULL), control(SafeRetain(control)), propertiesRoot(NULL), editable(true)
{
    propertiesRoot = new PropertiesRoot();
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
    // TODO: FIXME:
    return false;
}

bool ControlNode::IsCloned() const
{
    // TODO: FIXME:
    return false;
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
