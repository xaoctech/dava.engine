#include "Modules/DocumentsModule/EditorData.h"
#include "EditorSystems/EditorSystemsManager.h"

DAVA_VIRTUAL_REFLECTION_IMPL(EditorData)
{
    DAVA::ReflectionRegistrator<EditorData>::Begin()
    .ConstructorByPointer()
    .Field(emulationModePropertyName.c_str(), &EditorData::emulationMode)
    .End();
}

EditorData::EditorData()
{
}

EditorData::~EditorData() = default;

const EditorSystemsManager* EditorData::GetSystemsManager() const
{
    return systemsManager.get();
}

DAVA::FastName EditorData::emulationModePropertyName{ "emulation mode" };
