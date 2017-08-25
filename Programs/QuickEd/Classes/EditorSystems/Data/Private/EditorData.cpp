#include "EditorSystems/Data/EditorData.h"

DAVA::FastName EditorData::highlightedNodePropertyName{ "highlighted node" };

DAVA_VIRTUAL_REFLECTION_IMPL(EditorData)
{
    DAVA::ReflectionRegistrator<EditorData>::Begin()
    .Field(highlightedNodePropertyName.c_str(), &EditorData::GetHighlightedNode, &EditorData::SetHighlightedNode)
    .End();
}

ControlNode* EditorData::GetHighlightedNode() const
{
    return highlightedNode;
}

void EditorData::SetHighlightedNode(ControlNode* node)
{
    highlightedNode = node;
}
