#include "VisualScript/VisualScriptNode.h"
#include "Engine/Engine.h"
#include "FileSystem/YamlNode.h"
#include "Job/JobManager.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"

#include <locale>

namespace DAVA
{
FastName VisualScriptNode::typeNames[] =
{
  FastName("None"),
  FastName("GetVar"),
  FastName("SetVar"),
  FastName("Function"),
  FastName("Branch"),
  FastName("While"),
  FastName("DoN"),
  FastName("For"),
  FastName("Wait"),
  FastName("Event"),
  FastName("CustomEvent"),
  FastName("AnotherScript"),
  FastName("GetMember"),
  FastName("SetMember"),
};

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptNode)
{
    ReflectionRegistrator<VisualScriptNode>::Begin()[M::Serialize()]
    .ConstructorByPointer()
    .Field("type", &VisualScriptNode::type)[M::Serialize()]
    .Field("name", &VisualScriptNode::name)[M::Serialize()]
    .Field("position", &VisualScriptNode::position)[M::Serialize()]
    .Field("allPinsMap", &VisualScriptNode::allPinsMap)
    .End();
}

VisualScriptNode::VisualScriptNode()
{
}

VisualScriptNode::~VisualScriptNode()
{
    auto connections = GetAllConnections();
    for (auto connection : connections)
    {
        connection.first->Disconnect(connection.second);
    }

    for (auto it : allPinsMap)
    {
        VisualScriptPin* pin = it.second;
        /*
            Check that this is our own pin. Some nodes like AnotherScriptNode register pins from it's internals.
            We do not need to release them.
         */
        if (pin->GetExecutionOwner() == this && pin->GetSerializationOwner() == this)
        {
            delete pin;
        }
    }
}

void VisualScriptNode::SetType(eType type_)
{
    type = type_;
}

void VisualScriptNode::SetName(const FastName& name_)
{
    name = name_;
}

VisualScriptNode::eType VisualScriptNode::GetType() const
{
    return type;
}

const FastName& VisualScriptNode::GetName() const
{
    return name;
}

const FastName& VisualScriptNode::GetTypeName()
{
    return typeNames[type];
}

VisualScriptPin* VisualScriptNode::RegisterPin(VisualScriptPin* pin)
{
    allPinsMap.emplace(pin->GetName(), pin);

    if (pin->GetAttribute() == VisualScriptPin::ATTR_IN)
    {
        allInputPins.emplace_back(pin);
        dataInputPins.emplace_back(pin);
    }
    if (pin->GetAttribute() == VisualScriptPin::ATTR_OUT)
    {
        allOutputPins.emplace_back(pin);
        dataOutputPins.emplace_back(pin);
    }

    if (pin->GetAttribute() == VisualScriptPin::EXEC_IN)
    {
        allInputPins.emplace_back(pin);
        execInPins.emplace_back(pin);
    }
    if (pin->GetAttribute() == VisualScriptPin::EXEC_OUT)
    {
        allOutputPins.emplace_back(pin);
        execOutPins.emplace_back(pin);
    }
    return pin;
}

void VisualScriptNode::UnregisterPin(VisualScriptPin* pin)
{
    for (auto it = allPinsMap.begin(); it != allPinsMap.end(); ++it)
    {
        if (it->second == pin)
        {
            allPinsMap.erase(it);
            break;
        }
    }

    allInputPins.erase(std::remove(allInputPins.begin(), allInputPins.end(), pin), allInputPins.end());
    allOutputPins.erase(std::remove(allOutputPins.begin(), allOutputPins.end(), pin), allOutputPins.end());

    dataInputPins.erase(std::remove(dataInputPins.begin(), dataInputPins.end(), pin), dataInputPins.end());
    dataOutputPins.erase(std::remove(dataOutputPins.begin(), dataOutputPins.end(), pin), dataOutputPins.end());

    execInPins.erase(std::remove(execInPins.begin(), execInPins.end(), pin), execInPins.end());
    execOutPins.erase(std::remove(execOutPins.begin(), execOutPins.end(), pin), execOutPins.end());
}

VisualScriptPin* VisualScriptNode::GetPinByName(const FastName& pinName)
{
    auto it = allPinsMap.find(pinName);
    if (it != allPinsMap.end())
    {
        return it->second;
    }
    return nullptr;
}

const Vector<VisualScriptPin*>& VisualScriptNode::GetAllInputPins() const
{
    return allInputPins;
}
const Vector<VisualScriptPin*>& VisualScriptNode::GetAllOutputPins() const
{
    return allOutputPins;
}

VisualScriptPin* VisualScriptNode::GetDataInputPin(uint32 index) const
{
    return dataInputPins[index];
}

VisualScriptPin* VisualScriptNode::GetDataOutputPin(uint32 index) const
{
    return dataOutputPins[index];
}

VisualScriptPin* VisualScriptNode::GetExecInputPin(uint32 index) const
{
    return execInPins[index];
}

VisualScriptPin* VisualScriptNode::GetExecOutputPin(uint32 index) const
{
    return execOutPins[index];
}

const Vector<VisualScriptPin*>& VisualScriptNode::GetDataInputPins() const
{
    return dataInputPins;
}

const Vector<VisualScriptPin*>& VisualScriptNode::GetDataOutputPins() const
{
    return dataOutputPins;
}

const Vector<VisualScriptPin*>& VisualScriptNode::GetExecInputPins() const
{
    return execInPins;
}

const Vector<VisualScriptPin*>& VisualScriptNode::GetExecOutputPins() const
{
    return execOutPins;
}

void VisualScriptNode::SetScript(VisualScript* script_)
{
    script = script_;
}

VisualScript* VisualScriptNode::GetScript() const
{
    return script;
}

Set<std::pair<VisualScriptPin*, VisualScriptPin*>> VisualScriptNode::GetAllConnections() const
{
    Set<std::pair<VisualScriptPin*, VisualScriptPin*>> connections;

    for (const auto& it : allPinsMap)
    {
        VisualScriptPin* inPin = it.second;
        const Set<VisualScriptPin*>& connectedTo = inPin->GetConnectedSet();
        for (const auto& outPin : connectedTo)
        {
            std::pair<VisualScriptPin*, VisualScriptPin*> pair = std::make_pair(inPin, outPin);
            connections.insert(pair);
        }
    }
    return connections;
}

void VisualScriptNode::Save(YamlNode* node) const
{
    node->Add("position", position);
}

void VisualScriptNode::Load(const YamlNode* node)
{
    position = node->Get("position")->AsVector2();
}

void VisualScriptNode::SaveDefaults(YamlNode* node) const
{
    if (!GetDataInputPins().empty())
    {
        YamlNode* defsNode = YamlNode::CreateMapNode();
        for (VisualScriptPin* inPin : GetDataInputPins())
        {
            if (!inPin->GetDefaultValue().IsEmpty())
            {
                const Any& value = inPin->GetDefaultValue();

                const Type* type = inPin->GetType(); // First get type from pin
                if (type == nullptr) // Otherwise get type from default value
                {
                    type = value.GetType();
                }

                const ReflectedType* refType = ReflectedTypeDB::GetByType(type);
                DVASSERT(refType->GetPermanentName() != "");

                YamlNode* valNode = YamlNode::CreateArrayNode();
                valNode->Add(refType->GetPermanentName());
                valNode->Add(YamlNode::CreateNodeFromAny(value));

                defsNode->Add(inPin->GetName().c_str(), valNode);
            }
        }
        node->Add("defaults", defsNode);
    }
}

void VisualScriptNode::LoadDefaults(const YamlNode* node)
{
    const YamlNode* defsNode = node->Get("defaults");
    if (defsNode)
    {
        for (VisualScriptPin* inPin : GetDataInputPins())
        {
            const YamlNode* valNode = defsNode->Get(inPin->GetName().c_str());
            if (valNode)
            {
                if (valNode->GetType() == YamlNode::eType::TYPE_STRING) // Legacy loader
                {
                    const Type* inPinType = inPin->GetType();
                    if (inPinType)
                    {
                        inPin->SetDefaultValue(valNode->AsAny(inPinType->Decay() ? inPinType->Decay() : inPinType));
                    }
                }
                else if (valNode->GetType() == YamlNode::eType::TYPE_ARRAY)
                {
                    DVASSERT(valNode->GetCount() == 2);

                    const String typeName = valNode->Get(0)->AsString();
                    DVASSERT(typeName != "");
                    const ReflectedType* refType = ReflectedTypeDB::GetByPermanentName(typeName);
                    DVASSERT(refType);
                    const Type* type = refType->GetType();

                    const Type* inPinType = inPin->GetType();
                    if (inPinType)
                    {
                        // TODO: check type here or check cast between types
                        DVASSERT(inPinType == type);
                    }

                    inPin->SetDefaultValue(valNode->Get(1)->AsAny(type));
                }
            }
        }
    }
}

} //DAVA
