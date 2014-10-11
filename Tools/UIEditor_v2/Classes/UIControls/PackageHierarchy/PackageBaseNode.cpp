//
//  UIPackageModelNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 3.10.14.
//
//

#include "PackageBaseNode.h"
#include "UIControls/UIEditorComponent.h"

using namespace DAVA;

PackageBaseNode::PackageBaseNode() : parent(NULL)
{
    
}

PackageBaseNode::~PackageBaseNode()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->parent == this);
        (*it)->parent = NULL;
        SafeRelease(*it);
    }
    children.clear();
}

int PackageBaseNode::GetCount() const
{
    return (int) children.size();
}

PackageBaseNode *PackageBaseNode::Get(int index) const
{
    return children[index];
}

int PackageBaseNode::GetIndex(PackageBaseNode *node) const
{
    return find(children.begin(), children.end(), node) - children.begin();
}

void PackageBaseNode::Add(PackageBaseNode *node)
{
    DVASSERT(node->parent == NULL);
    node->parent = this;
    children.push_back(node);
}

PackageBaseNode *PackageBaseNode::GetParent() const
{
    return parent;
}

String PackageBaseNode::GetName() const
{
    return "Unknown";
}

UIControl *PackageBaseNode::GetControl() const
{
    return NULL;
}

bool PackageBaseNode::IsHeader() const
{
    return false;
}

bool PackageBaseNode::IsInstancedFromPrototype() const
{
    return false;
}

bool PackageBaseNode::IsCloned() const
{
    return false;
}

bool PackageBaseNode::IsEditable() const
{
    return false;
}
