#include "VisualScript/Nodes/VisualScriptFunctionNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptFunctionNode)
{
    ReflectionRegistrator<VisualScriptFunctionNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptFunctionNode::VisualScriptFunctionNode()
{
    SetType(FUNCTION);
}

VisualScriptFunctionNode::VisualScriptFunctionNode(const FastName& className_, const FastName& functionName_)
    : VisualScriptFunctionNode()
{
    SetClassName(className_);
    SetFunctionName(functionName_);
    InitPins();
}

void VisualScriptFunctionNode::SetClassName(const FastName& className_)
{
    className = className_;
}
void VisualScriptFunctionNode::SetFunctionName(const FastName& functionName_)
{
    functionName = functionName_;
}

const FastName& VisualScriptFunctionNode::GetClassName() const
{
    return className;
}

const FastName& VisualScriptFunctionNode::GetFunctionName() const
{
    return functionName;
}

void VisualScriptFunctionNode::InitPins()
{
    if (className.IsValid() && functionName.IsValid())
    {
        String nodeName = className.c_str();
        nodeName += String("::") + functionName.c_str();
        name = FastName(nodeName);

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        AnyFn function = type->GetStructure()->GetMethod(functionName);
        if (function.IsValid() == false)
        {
            Logger::Error("Failed to find function %s in class %s", functionName.c_str(), className.c_str());
        }
        InitNodeWithAnyFn(function);
    }
}

void VisualScriptFunctionNode::InitNodeWithAnyFn(const AnyFn& function)
{
    if (!function.IsConst() && !function.IsStatic())
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));

    auto& functionTypes = function.GetInvokeParams().argsType;
    uint32 index = 0;

    for (auto type : functionTypes)
    {
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName(Format("arg%d", index++)), type));
        //Logger::Debug("arg%d - %s", index - 1, type->GetDemangledName().c_str());
    }

    if (!function.IsConst() && !function.IsStatic())
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));

    const Type* returnType = function.GetInvokeParams().retType;
    if (returnType != Type::Instance<void>())
    {
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("result"), returnType));
    }

    SetFunction(function);
}

void VisualScriptFunctionNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);

    node->Add("className", className.c_str());
    node->Add("functionName", functionName.c_str());
}

void VisualScriptFunctionNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);

    className = FastName(node->Get("className")->AsString());
    functionName = FastName(node->Get("functionName")->AsString());
    InitPins();
}
}
