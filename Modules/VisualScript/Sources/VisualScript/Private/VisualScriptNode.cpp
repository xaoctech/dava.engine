#include "VisualScript/VisualScriptNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"

#include "Engine/Engine.h"
#include "FileSystem/YamlNode.h"
#include "Job/JobManager.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"

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
                if (type->IsReference())
                {
                    type = type->Deref();
                }
                if (type->IsConst())
                {
                    type = type->Decay();
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
        uint32 defsCount = defsNode->GetCount();
        for (uint32 i = 0; i < defsCount; ++i)
        {
            const String pinName = defsNode->GetItemKeyName(i);
            const YamlNode* valNode = defsNode->Get(i);
            const Type* checkType = nullptr;

            DVASSERT(valNode);

            if (valNode->GetType() == YamlNode::eType::TYPE_ARRAY) // Variant then we store type of default value
            {
                DVASSERT(valNode->GetCount() == 2);
                const String typeName = valNode->Get(0)->AsString();
                const ReflectedType* refType = ReflectedTypeDB::GetByPermanentName(typeName);
                DVASSERT(refType);
                checkType = refType->GetType();
                valNode = valNode->Get(1); // valNode now contains value
                DVASSERT(valNode);
            }

            VisualScriptPin* inPin = GetPinByName(FastName(pinName));

            if (inPin == nullptr) // Check legacy support (pins with names like `argN`)
            {
                if (StringUtils::StartsWith(pinName, "arg"))
                {
                    int32 index = atoi(pinName.c_str() + 3);
                    inPin = GetDataInputPin(index);
                }
            }

            if (inPin)
            {
                const Type* inPinType = inPin->GetType();
                if (inPinType && checkType)
                {
                    if (inPinType->IsReference())
                    {
                        inPinType = inPinType->Deref();
                    }
                    if (inPinType->IsConst())
                    {
                        inPinType = inPinType->Decay();
                    }

                    // TODO: check type here or check cast between types
                    DVASSERT(inPinType == checkType);
                }

                if (inPinType || checkType)
                {
                    inPin->SetDefaultValue(valNode->AsAny(checkType ? checkType : inPinType));
                }
                else
                {
                    Logger::Error("Pin type error: %s", pinName.c_str());
                }
            }
            else
            {
                Logger::Error("Pin `%s.%s` not found while setting default value `%s`!",
                              GetName().c_str(),
                              pinName.c_str(),
                              valNode->AsString().c_str());
            }
        }
    }
}

} //DAVA
