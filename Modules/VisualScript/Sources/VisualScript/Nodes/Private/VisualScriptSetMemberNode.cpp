#include "VisualScript/Nodes/VisualScriptSetMemberNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

#include <locale>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptSetMemberNode)
{
    ReflectionRegistrator<VisualScriptSetMemberNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptSetMemberNode::VisualScriptSetMemberNode()
{
    SetType(SET_MEMBER);
}

VisualScriptSetMemberNode::VisualScriptSetMemberNode(const FastName& className_, const FastName& fieldName_)
    : VisualScriptSetMemberNode()
{
    SetClassName(className_);
    SetFieldName(fieldName_);
    InitPins();
}

void VisualScriptSetMemberNode::SetClassName(const FastName& className_)
{
    className = className_;
}

const FastName& VisualScriptSetMemberNode::GetClassName() const
{
    return className;
}

void VisualScriptSetMemberNode::SetFieldName(const FastName& fieldName_)
{
    fieldName = fieldName_;
}

const FastName& VisualScriptSetMemberNode::GetFieldName() const
{
    return fieldName;
}

void VisualScriptSetMemberNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("className", className.c_str());
    node->Add("fieldName", fieldName.c_str());
    SaveDefaults(node);
}

void VisualScriptSetMemberNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetClassName(node->Get("className")->AsFastName());
    SetFieldName(node->Get("fieldName")->AsFastName());
    InitPins();
    LoadDefaults(node);
}

const ValueWrapper* VisualScriptSetMemberNode::GetValueWrapper() const
{
    return valueWrapper;
}

void VisualScriptSetMemberNode::InitPins()
{
    if (className.IsValid() && fieldName.IsValid())
    {
        String fName = fieldName.c_str();
        {
            fName[0] = std::toupper(fName[0], std::locale());
        }
        String nodeName = className.c_str() + String("::Set") + fName;
        name = FastName(nodeName);

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
        if (!vw)
        {
            Logger::Error("Failed to find value wrapper for %s in class %s", fieldName.c_str(), className.c_str());
        }
        InitNodeWithValueWrapper(vw);
    }
}

void VisualScriptSetMemberNode::InitNodeWithValueWrapper(const ValueWrapper* wrapper)
{
    valueWrapper = wrapper;

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("object"), ReflectedTypeDB::GetByPermanentName(className.c_str())->GetType()));

    const Type* mType = valueWrapper->GetType(ReflectedObject());
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("set"), mType, VisualScriptPin::DEFAULT_PARAM));
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("get"), mType));
}
}
