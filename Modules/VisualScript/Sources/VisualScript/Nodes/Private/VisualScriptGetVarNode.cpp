#include "VisualScript/Nodes/VisualScriptGetVarNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Utils/StringFormat.h>
#include <Utils/StringUtils.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptGetVarNode)
{
    ReflectionRegistrator<VisualScriptGetVarNode>::Begin()
    .ConstructorByPointer<>()
    .ConstructorByPointer<const Reflection&, const FastName&>()
    .Field("varPath", &VisualScriptGetVarNode::varPath)
    .End();
}

VisualScriptGetVarNode::VisualScriptGetVarNode()
{
    SetType(GET_VAR);
    outValuePin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::Attribute::ATTR_OUT, FastName("value"), nullptr));
}

VisualScriptGetVarNode::VisualScriptGetVarNode(const Reflection& ref, const FastName& varPath)
    : VisualScriptGetVarNode()
{
    SetVarPath(varPath);
    BindReflection(ref);
}

void VisualScriptGetVarNode::SetVarPath(const DAVA::FastName& varPath_)
{
    varPath = varPath_;

    String newName = Format("Get%s", StringUtils::CapitalizeFirst(varPath.c_str()).c_str());
    SetName(FastName(newName));
}

const FastName& VisualScriptGetVarNode::GetVarPath() const
{
    return varPath;
}

void VisualScriptGetVarNode::BindReflection(const Reflection& ref)
{
    reflection = ref.GetField(varPath);
    DVASSERT(reflection.IsValid());

    const Type* type = reflection.GetValue().GetType();
    outValuePin->SetType(type);
}

const Reflection& VisualScriptGetVarNode::GetReflection() const
{
    return reflection;
}

void VisualScriptGetVarNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("varPath", varPath.c_str());
}

void VisualScriptGetVarNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetVarPath(node->Get("varPath")->AsFastName());
}
}