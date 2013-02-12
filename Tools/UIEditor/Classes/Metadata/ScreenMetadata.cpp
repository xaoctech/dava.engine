//
//  ScreenMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#include "ScreenMetadata.h"
#include "HierarchyTreeController.h"

QString ScreenMetadata::GetName() const
{
    const HierarchyTreeScreenNode* screenNode = GetScreenNode();
    if (screenNode)
    {
        return screenNode->GetName();
    }
    
    return QString();
}

void ScreenMetadata::SetName(const QString& name)
{
    HierarchyTreeScreenNode* screenNode = GetScreenNode();
    if (screenNode)
    {
        screenNode->SetName(name);
    }
}

HierarchyTreeScreenNode* ScreenMetadata::GetScreenNode() const
{
    // Screen Node is one and only.
    return dynamic_cast<HierarchyTreeScreenNode*>(GetTreeNode(0));
}

