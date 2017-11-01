#pragma once

#include "Reflection/Reflection.h"
#include "UI/Flow/UIFlowService.h"

namespace DAVA
{
class UIFlowContext;

class DataUIService : public UIFlowService
{
    DAVA_VIRTUAL_REFLECTION(DataUIService, UIFlowService);

public:
    void SetDataDirty(const Reflection& ref);
};
}