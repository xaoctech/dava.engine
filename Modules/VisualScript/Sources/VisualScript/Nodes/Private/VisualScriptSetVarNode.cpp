#include "VisualScript/Nodes/VisualScriptSetVarNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Utils/StringFormat.h>
#include <Utils/StringUtils.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptSetVarNode)
{
    ReflectionRegistrator<VisualScriptSetVarNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const Reflection&, const FastName&>()
    .Field("varPath", &VisualScriptSetVarNode::varPath)
    .End();
}

VisualScriptSetVarNode::VisualScriptSetVarNode()
{
    SetType(SET_VAR);

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    varInPin = new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("set"), nullptr, VisualScriptPin::DEFAULT_PARAM);
    RegisterPin(varInPin);

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));
    varOutPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("get"), nullptr));
}

VisualScriptSetVarNode::VisualScriptSetVarNode(const Reflection& ref, const FastName& varPath)
    : VisualScriptSetVarNode()
{
    SetVarPath(varPath);
    BindReflection(ref);
}

void VisualScriptSetVarNode::SetVarPath(const DAVA::FastName& varPath_)
{
    varPath = varPath_;

    String newName = Format("Set%s", StringUtils::CapitalizeFirst(varPath.c_str()).c_str());
    SetName(FastName(newName));
}

const FastName& VisualScriptSetVarNode::GetVarPath() const
{
    return varPath;
}

void VisualScriptSetVarNode::BindReflection(const Reflection& ref)
{
    reflection = ref.GetField(varPath);
    DVASSERT(reflection.IsValid());

    const Type* type = reflection.GetValue().GetType();
    varInPin->SetType(type);
    varOutPin->SetType(type);
}

const Reflection& VisualScriptSetVarNode::GetReflection() const
{
    return reflection;
}

void VisualScriptSetVarNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("varPath", varPath.c_str());
    SaveDefaults(node);
}

void VisualScriptSetVarNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetVarPath(node->Get("varPath")->AsFastName());
    LoadDefaults(node);
}
}
