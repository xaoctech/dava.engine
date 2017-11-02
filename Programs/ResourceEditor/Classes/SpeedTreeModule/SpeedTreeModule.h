#pragma once

#if defined(__DAVAENGINE_SPEEDTREE__)


#include "TArc/Core/ClientModule.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/Utils/QtConnections.h"

#include "Base/Any.h"
#include "Reflection/Reflection.h"

#include <memory>

class SpeedTreeModule : public DAVA::ClientModule
{
protected:
    void PostInit() override;

private:
    void OnImportSpeedTree();

    DAVA::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(SpeedTreeModule, DAVA::ClientModule);
};

#endif //#if defined (__DAVAENGINE_SPEEDTREE__)
