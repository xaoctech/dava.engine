#include "Modules/DocumentsModule/EditorData.h"
#include "EditorSystems/EditorSystemsManager.h"

DAVA_VIRTUAL_REFLECTION_IMPL(EditorSystemsData)
{
    DAVA::ReflectionRegistrator<EditorSystemsData>::Begin()
    .ConstructorByPointer()
    .Field(emulationModePropertyName.c_str(), &EditorSystemsData::emulationMode)
    .End();
}

EditorSystemsData::EditorSystemsData()
{
}

EditorSystemsData::~EditorSystemsData() = default;

const EditorSystemsManager* EditorSystemsData::GetSystemsManager() const
{
    return systemsManager.get();
}

DAVA::FastName EditorSystemsData::emulationModePropertyName{ "emulation mode" };
