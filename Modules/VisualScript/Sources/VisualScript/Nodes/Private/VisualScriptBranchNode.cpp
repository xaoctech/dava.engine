#include "VisualScript/Nodes/VisualScriptBranchNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptBranchNode)
{
    ReflectionRegistrator<VisualScriptBranchNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptBranchNode::VisualScriptBranchNode()
    : VisualScriptNode()
{
    SetType(BRANCH);
    SetName(GetTypeName());

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    conditionIn = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("condition"), Type::Instance<bool>()));
    execOutTrue = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("true"), nullptr));
    execOutFalse = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("false"), nullptr));
}
}
