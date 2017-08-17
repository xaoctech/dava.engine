#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class BeastModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    DAVA::TArc::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(BeastModule, DAVA::TArc::ClientModule);
};

#endif //#if defined (__DAVAENGINE_BEAST__)
