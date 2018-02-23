#pragma once

#include "VisualScript/VisualScriptNode.h"

namespace DAVA
{
class VisualScriptWhileNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptWhileNode, VisualScriptNode);

public:
    VisualScriptWhileNode();
    ~VisualScriptWhileNode() override = default;

    VisualScriptPin* execPin = nullptr;
    VisualScriptPin* conditionPin = nullptr;
    VisualScriptPin* loopBodyPin = nullptr;
    VisualScriptPin* cyclePin = nullptr;
    VisualScriptPin* loopCompletedPin = nullptr;
};
}
