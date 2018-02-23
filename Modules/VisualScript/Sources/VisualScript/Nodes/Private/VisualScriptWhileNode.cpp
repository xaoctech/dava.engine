#include "VisualScript/Nodes/VisualScriptWhileNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptWhileNode)
{
    ReflectionRegistrator<VisualScriptWhileNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptWhileNode::VisualScriptWhileNode()
{
    SetType(WHILE);
    SetName(GetTypeName());

    execPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    conditionPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("condition"), Type::Instance<bool>()));
    loopBodyPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("loop body"), nullptr));
    loopCompletedPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("completed"), nullptr));

    cyclePin = new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("cycle"), nullptr);
}
}
