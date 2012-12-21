//
//  PlatformMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#include "PlatformMetadata.h"
#include "HierarchyTreeController.h"

// Getters/setters.
QString PlatformMetadata::GetName() const
{
    const HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        return platformNode->GetName();
    }

    return QString();
}

void PlatformMetadata::SetName(const QString& name)
{
    HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        platformNode->SetName(name);
    }
}

float PlatformMetadata::GetHeight() const
{
    HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        return platformNode->GetHeight();
    }

    return -1.0f;
}

void PlatformMetadata::SetHeight(float value)
{
    HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        platformNode->SetSize(platformNode->GetWidth(), value);
    }
}

float PlatformMetadata::GetWidth() const
{
    HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        return platformNode->GetWidth();
    }
    
    return -1.0f;
}

void PlatformMetadata::SetWidth(float value)
{
    HierarchyTreePlatformNode* platformNode = GetPlatformNode();
    if (platformNode)
    {
        platformNode->SetSize(value, platformNode->GetHeight());
    }
}

HierarchyTreePlatformNode* PlatformMetadata::GetPlatformNode() const
{
    // Platform Node is one and only.
    return dynamic_cast<HierarchyTreePlatformNode*>(GetTreeNode(0));
}
