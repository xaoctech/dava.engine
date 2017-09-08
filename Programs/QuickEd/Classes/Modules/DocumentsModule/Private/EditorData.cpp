#include "Modules/DocumentsModule/EditorData.h"
#include "EditorSystems/EditorSystemsManager.h"

DAVA_VIRTUAL_REFLECTION_IMPL(EditorData)
{
    DAVA::ReflectionRegistrator<EditorData>::Begin()
    .ConstructorByPointer()
    .Field(emulationModePropertyName.c_str(), &EditorData::emulationMode)
    .Field(systemsManagerPropertyName.c_str(), &EditorData::GetSystemsManager, nullptr)
    .End();
}

EditorData::EditorData()
{
}

EditorData::~EditorData() = default;

EditorSystemsManager* EditorData::GetSystemsManager()
{
    return systemsManager.get();
}

DAVA::FastName EditorData::emulationModePropertyName{ "emulation mode" };
DAVA::FastName EditorData::systemsManagerPropertyName{ "editor systems manager" };