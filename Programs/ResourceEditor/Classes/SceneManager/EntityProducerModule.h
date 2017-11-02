#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"
#include "Reflection/Reflection.h"

class EntityProducerModule : public DAVA::ClientModule
{
protected:
    void PostInit() override;

    // actions
    void InstantiateCurrentCamera();

private:
    DAVA::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(EntityProducerModule, DAVA::ClientModule);
};
