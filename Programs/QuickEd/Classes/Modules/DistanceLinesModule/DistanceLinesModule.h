#pragma once

#include <TArc/Core/ClientModule.h>

class DistanceLinesModule : public DAVA::TArc::ClientModule
{
public:
    DistanceLinesModule();

private:
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(DistanceLinesModule, DAVA::TArc::ClientModule);
};
