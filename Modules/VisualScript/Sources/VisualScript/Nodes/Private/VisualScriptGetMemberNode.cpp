#include "VisualScript/Nodes/VisualScriptGetMemberNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Utils/StringFormat.h>
#include <Utils/StringUtils.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptGetMemberNode)
{
    ReflectionRegistrator<VisualScriptGetMemberNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptGetMemberNode::VisualScriptGetMemberNode()
{
    SetType(GET_MEMBER);
}

VisualScriptGetMemberNode::VisualScriptGetMemberNode(const FastName& className_, const FastName& fieldName_)
    : VisualScriptGetMemberNode()
{
    SetClassName(className_);
    SetFieldName(fieldName_);
    InitPins();
}

void VisualScriptGetMemberNode::SetClassName(const FastName& className_)
{
    className = className_;
}

const FastName& VisualScriptGetMemberNode::GetClassName() const
{
    return className;
}

void VisualScriptGetMemberNode::SetFieldName(const FastName& fieldName_)
{
    fieldName = fieldName_;
}

const FastName& VisualScriptGetMemberNode::GetFieldName() const
{
    return fieldName;
}

void VisualScriptGetMemberNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("className", className.c_str());
    node->Add("fieldName", fieldName.c_str());
}

void VisualScriptGetMemberNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetClassName(node->Get("className")->AsFastName());
    SetFieldName(node->Get("fieldName")->AsFastName());
    InitPins();
}

const ValueWrapper* VisualScriptGetMemberNode::GetValueWrapper() const
{
    return valueWrapper;
}

void VisualScriptGetMemberNode::InitPins()
{
    if (className.IsValid() && fieldName.IsValid())
    {
        String nodeName = Format("%s::Get%s", className.c_str(), StringUtils::CapitalizeFirst(fieldName.c_str()).c_str());
        SetName(FastName(nodeName));

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        const ReflectedStructure* rs = type->GetStructure();
        const ValueWrapper* vw = nullptr;
        for (auto& field : rs->fields)
        {
            if (field->name == fieldName)
            {
                vw = field->valueWrapper.get();
                break;
            }
        }
        if (vw)
        {
            valueWrapper = vw;

            const Type* classType = ReflectedTypeDB::GetByPermanentName(className.c_str())->GetType();
            RegisterPin(new VisualScriptPin(this, VisualScriptPin::Attribute::ATTR_IN, FastName("object"), classType));

            const Type* memberType = valueWrapper->GetType(ReflectedObject());
            RegisterPin(new VisualScriptPin(this, VisualScriptPin::Attribute::ATTR_OUT, FastName("get"), memberType));
        }
        else
        {
            VSLogger_Error("Failed to find value wrapper for %s in class %s", fieldName.c_str(), className.c_str());
        }
    }
}
}
