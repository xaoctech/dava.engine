#include "PackageControlsNode.h"

#include "ControlNode.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(UIPackage *package, const String &name, bool editable) : package(SafeRetain(package)), name(name), editable(editable)
{
    for (int i = 0; i < package->GetControlsCount(); i++)
    {
        Add(new ControlNode(package->GetControl(i), editable));
    }
}

PackageControlsNode::~PackageControlsNode()
{
    SafeRelease(package);
}

String PackageControlsNode::GetName() const
{
    return name;
}

bool PackageControlsNode::IsInstancedFromPrototype() const
{
    return false;
}

bool PackageControlsNode::IsCloned() const
{
    return false;
}

