#pragma once

#include <TArc/Core/ClientModule.h>
#include <Reflection/Reflection.h>

class TerrainConverterGUIModule : public DAVA::TArc::ClientModule
{
public:
    TerrainConverterGUIModule() = default;

    void PostInit() override;

private:
    DAVA_VIRTUAL_REFLECTION(TerrainConverterGUIModule, DAVA::TArc::ClientModule);
};