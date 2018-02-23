#pragma once

#include "VisualScript/VisualScriptNode.h"

namespace DAVA
{
class VisualScriptBranchNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptBranchNode, VisualScriptNode);

public:
    VisualScriptBranchNode();
    ~VisualScriptBranchNode() override = default;

    VisualScriptPin* execOutTrue = nullptr;
    VisualScriptPin* execOutFalse = nullptr;
    VisualScriptPin* conditionIn = nullptr;
};
}