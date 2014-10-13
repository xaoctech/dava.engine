#include "ControlNode.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control) : PackageBaseNode(NULL), control(SafeRetain(control)), propertiesRoot(NULL), editable(true), cloned(false)
{
    this->propertiesRoot = new PropertiesRoot(control);
}

ControlNode::ControlNode(const ControlNode *node) : PackageBaseNode(NULL), control(NULL), propertiesRoot(NULL), editable(true), cloned(true)
{
    UIControl *sourceControl = node->GetControl();
    control = ObjectFactory::Instance()->New<UIControl>(sourceControl->GetControlClassName());
    control->SetCustomControlClassName(sourceControl->GetCustomControlClassName());
    
    propertiesRoot = new PropertiesRoot(control, node->GetPropertiesRoot());
    
    for (auto it = node->nodes.begin(); it != node->nodes.end(); ++it)
    {
        ControlNode *childNode = new ControlNode(*it);
        childNode->SetParent(this);
        nodes.push_back(childNode);
        control->AddControl(childNode->GetControl());
    }
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
    control->AddControl(node->GetControl());
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
    return new ControlNode(this);
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
    return cloned;
}

