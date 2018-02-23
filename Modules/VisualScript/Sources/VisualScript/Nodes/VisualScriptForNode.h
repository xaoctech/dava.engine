#pragma once

#include "VisualScript/VisualScriptNode.h"

namespace DAVA
{
class VisualScriptForNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptForNode, VisualScriptNode);

public:
    VisualScriptForNode();
    ~VisualScriptForNode() override = default;

    VisualScriptPin* execPin = nullptr;
    VisualScriptPin* breakPin = nullptr;
    VisualScriptPin* cyclePin = nullptr;
    VisualScriptPin* loopBodyPin = nullptr;
    VisualScriptPin* loopCompletedPin = nullptr;
    VisualScriptPin* firstIndexPin = nullptr;
    VisualScriptPin* lastIndexPin = nullptr;
    VisualScriptPin* indexPin = nullptr;
    int32 index = 0;
};
}
