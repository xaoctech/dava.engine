#pragma once

#include <TArc/Core/ClientModule.h>

class DistanceModule : public DAVA::TArc::ClientModule
{
    DistanceModule();

    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(DistanceModule, DAVA::TArc::ClientModule);
};
