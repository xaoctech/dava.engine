#include "Modules/DocumentsModule/EditorSystemsData.h"
#include "EditorSystems/EditorSystemsManager.h"

DAVA_VIRTUAL_REFLECTION_IMPL(EditorSystemsData)
{
    DAVA::ReflectionRegistrator<EditorSystemsData>::Begin()
    .ConstructorByPointer()
    .Field(highlightedNodePropertyName.c_str(), &EditorSystemsData::GetHighlightedNode, &EditorSystemsData::SetHighlightedNode)
    .Field(emulationModePropertyName.c_str(), &EditorSystemsData::emulationMode)
    .End();
}

EditorSystemsData::EditorSystemsData()
{
}

EditorSystemsData::~EditorSystemsData() = default;

ControlNode* EditorSystemsData::GetHighlightedNode() const
{
    return highlightedNode;
}

void EditorSystemsData::SetHighlightedNode(ControlNode* node)
{
    highlightedNode = node;
}

const EditorSystemsManager* EditorSystemsData::GetSystemsManager() const
{
    return systemsManager.get();
}

DAVA::FastName EditorSystemsData::emulationModePropertyName{ "emulation mode" };
DAVA::FastName EditorSystemsData::highlightedNodePropertyName{ "highlighted node" };
