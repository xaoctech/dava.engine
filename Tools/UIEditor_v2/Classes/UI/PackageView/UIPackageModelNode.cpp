//
//  UIPackageModelNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 3.10.14.
//
//

#include "UIPackageModelNode.h"
#include "UIControls/UIEditorComponent.h"

using namespace DAVA;

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelNode
////////////////////////////////////////////////////////////////////////////////

UIPackageModelNode::UIPackageModelNode() : parent(NULL)
{
    
}

UIPackageModelNode::~UIPackageModelNode()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->parent == this);
        (*it)->parent = NULL;
        SafeRelease(*it);
    }
    children.clear();
}

int UIPackageModelNode::GetCount() const
{
    return (int) children.size();
}

UIPackageModelNode *UIPackageModelNode::Get(int index) const
{
    return children[index];
}

int UIPackageModelNode::GetIndex(UIPackageModelNode *node) const
{
    return find(children.begin(), children.end(), node) - children.begin();
}

void UIPackageModelNode::Add(UIPackageModelNode *node)
{
    DVASSERT(node->parent == NULL);
    node->parent = this;
    children.push_back(node);
}

UIPackageModelNode *UIPackageModelNode::GetParent() const
{
    return parent;
}

String UIPackageModelNode::GetName() const
{
    return "Unknown";
}

UIControl *UIPackageModelNode::GetControl() const
{
    return NULL;
}

bool UIPackageModelNode::IsHeader() const
{
    return false;
}

bool UIPackageModelNode::IsInstancedFromPrototype() const
{
    return false;
}

bool UIPackageModelNode::IsCloned() const
{
    return false;
}

bool UIPackageModelNode::IsEditable() const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelControlNode
////////////////////////////////////////////////////////////////////////////////

UIPackageModelControlNode::UIPackageModelControlNode(UIControl *control, bool editable)
    : control(SafeRetain(control)), editable(editable)
{
    const List<UIControl*> &children = control->GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it)
        Add(new UIPackageModelControlNode(*it, editable));
}

UIPackageModelControlNode::~UIPackageModelControlNode()
{
    SafeRelease(control);
}

String UIPackageModelControlNode::GetName() const
{
    return control->GetName();
}


UIControl *UIPackageModelControlNode::GetControl() const
{
    return control;
}

bool UIPackageModelControlNode::IsInstancedFromPrototype() const
{
    UIEditorComponent *component = static_cast<UIEditorComponent*>(control->GetCustomData());
    return component->GetPrototype() != NULL;
}

bool UIPackageModelControlNode::IsCloned() const
{
    UIEditorComponent *component = static_cast<UIEditorComponent*>(control->GetCustomData());
    return component->IsClonedFromPrototype();
}

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelRootNode
////////////////////////////////////////////////////////////////////////////////

UIPackageModelRootNode::UIPackageModelRootNode(DAVA::UIPackage *package, const String &name, eMode mode)
: package(SafeRetain(package))
, name(name)
{
    switch (mode)
    {
        case MODE_CONTROLS_AND_IMPORTED_PACKAGED:
            Add(new UIPackageModelImportedPackagesNode(package));
//            for (int i = 0; i < package->GetPackagesCount(); i++)
//            {
//                String name = package->GetPackage(i)->GetName() + " [imported]";
//                Add(new UIPackageModelRootNode(package->GetPackage(i), name, UIPackageModelRootNode::MODE_ONLY_CONTROLS));
//            }
//
            Add(new UIPackageModelControlsHeaderNode(package));
            break;
            
        case MODE_ONLY_CONTROLS:
            for (int i = 0; i < package->GetControlsCount(); i++)
            {
                Add(new UIPackageModelControlNode(package->GetControl(i), false));
            }
            break;
    }
}

UIPackageModelRootNode::~UIPackageModelRootNode()
{
    SafeRelease(package);
}

String UIPackageModelRootNode::GetName() const
{
    return name;
}

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelControlsHeaderNode
////////////////////////////////////////////////////////////////////////////////

UIPackageModelControlsHeaderNode::UIPackageModelControlsHeaderNode(DAVA::UIPackage *package)
: package(SafeRetain(package))
{
    for (int i = 0; i < package->GetControlsCount(); i++)
    {
        Add(new UIPackageModelControlNode(package->GetControl(i), true));
    }
}

UIPackageModelControlsHeaderNode::~UIPackageModelControlsHeaderNode()
{
    SafeRelease(package);
}

String UIPackageModelControlsHeaderNode::GetName() const
{
    return "Controls";
}

bool UIPackageModelControlsHeaderNode::IsInstancedFromPrototype() const
{
    return false;
}

bool UIPackageModelControlsHeaderNode::IsCloned() const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelImportedPackagesNode
////////////////////////////////////////////////////////////////////////////////

UIPackageModelImportedPackagesNode::UIPackageModelImportedPackagesNode(DAVA::UIPackage *package)
{
    for (int i = 0; i < package->GetPackagesCount(); i++)
    {
        UIPackage *pack = package->GetPackage(i);
        Add(new UIPackageModelRootNode(pack, pack->GetName(), UIPackageModelRootNode::MODE_ONLY_CONTROLS));
    }
}

UIPackageModelImportedPackagesNode::~UIPackageModelImportedPackagesNode()
{
    
}
    
String UIPackageModelImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}
    
