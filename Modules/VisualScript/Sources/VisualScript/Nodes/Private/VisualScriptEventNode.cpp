#include "VisualScript/Nodes/VisualScriptEventNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptEventNode)
{
    ReflectionRegistrator<VisualScriptEventNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptEventNode::VisualScriptEventNode()
{
    SetType(EVENT);
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("event"), nullptr));
}

void VisualScriptEventNode::SetEventName(const FastName& eventName_)
{
    const ReflectedType* rType = ReflectedTypeDB::GetByPermanentName(String(eventName_.c_str()));

    for (const auto& field : rType->GetStructure()->fields)
    {
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, field->name, field->valueWrapper->GetType(ReflectedObject())));
    }

    eventName = eventName_;
    SetName(eventName);
}

const FastName& VisualScriptEventNode::GetEventName() const
{
    return eventName;
}

void VisualScriptEventNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);

    node->Add("eventName", eventName.c_str());
}

void VisualScriptEventNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);

    eventName = FastName(node->Get("eventName")->AsString());
    SetEventName(eventName);
}

void VisualScriptEventNode::BindReflection(const Reflection& reflection)
{
    for (size_t i = 0; i < dataOutputPins.size(); ++i)
    {
        Reflection refField = reflection.GetField(dataOutputPins[i]->GetName());
        DVASSERT(refField.IsValid());
        dataOutputPins[i]->SetType(refField.GetValueType());
        dataOutputPins[i]->SetValue(refField.GetValue());
    }
}
}
