#include "AggregatorMetadata.h"

// Getters/setters.
QString AggregatorMetadata::GetName() const
{
    const HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetName();
    }

    return QString();
}

void AggregatorMetadata::SetName(const QString& name)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetName(name);
    }
}

float AggregatorMetadata::GetHeight() const
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetRect().dy;
    }

    return -1.0f;
}

void AggregatorMetadata::SetHeight(float value)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetRect(Rect(0, 0, node->GetRect().dx, value));
    }
}

float AggregatorMetadata::GetWidth() const
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetRect().dx;
    }
    
    return -1.0f;
}

void AggregatorMetadata::SetWidth(float value)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetRect(Rect(0, 0, value, node->GetRect().dy));
    }
}

HierarchyTreeAggregatorNode* AggregatorMetadata::GetNode() const
{
    // Platform Node is one and only.
    return dynamic_cast<HierarchyTreeAggregatorNode*>(GetTreeNode(0));
}