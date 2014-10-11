#include "ControlNode.h"

#include "UIControls/UIEditorComponent.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control, bool editable) : control(SafeRetain(control)), editable(editable)
{
    const List<UIControl*> &children = control->GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it)
        Add(new ControlNode(*it, editable));
}

ControlNode::~ControlNode()
{
    SafeRelease(control);
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

