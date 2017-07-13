#include "TerainConverterModule.h"

#include <TArc/PluginsManager/TArcPlugin.h>
#include <Base/BaseTypes.h>

void TerrainConverterGUIModule::PostInit()
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(TerrainConverterGUIModule)
{
    DAVA::ReflectionRegistrator<TerrainConverterGUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

START_PLUGINS_DECLARATION()
DECLARE_PLUGIN(TerrainConverterGUIModule, "ResourceEditor", "TerrainConverter", "Terrain Converter for Blitz", "Terrain Converter for Blitz", 0, 1)
END_PLUGINS_DECLARATION()
