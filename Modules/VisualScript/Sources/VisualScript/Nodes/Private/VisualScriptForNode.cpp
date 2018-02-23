#include "VisualScript/Nodes/VisualScriptForNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptForNode)
{
    ReflectionRegistrator<VisualScriptForNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptForNode::VisualScriptForNode()
{
    SetType(FOR);
    SetName(GetTypeName());

    execPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    firstIndexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("first index"), Type::Instance<int32>()));
    lastIndexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("last index"), Type::Instance<int32>()));
    breakPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("break"), nullptr));

    // hidden pin
    cyclePin = new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("cycle"), nullptr);

    loopBodyPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("loop body"), nullptr));
    indexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("index"), Type::Instance<int32>()));
    loopCompletedPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("completed"), nullptr));
}
}
