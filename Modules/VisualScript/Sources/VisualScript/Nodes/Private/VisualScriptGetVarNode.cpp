#include "VisualScript/Nodes/VisualScriptGetVarNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>

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
    outValuePin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("value"), nullptr));
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

    String newName = String("Get ") + varPath.c_str();
    SetName(FastName(newName));
}

const FastName& VisualScriptGetVarNode::GetVarPath() const
{
    return varPath;
}

void VisualScriptGetVarNode::BindReflection(const Reflection& ref)
{
    Reflection refField = ref.GetField(varPath);
    const Type* type = refField.GetValue().GetType();
    outValuePin->SetType(type);
    SetReflection(refField);
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