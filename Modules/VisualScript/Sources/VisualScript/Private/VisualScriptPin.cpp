#include "VisualScript/VisualScriptPin.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptNode.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_REFLECTION_IMPL(VisualScriptPin)
{
    ReflectionRegistrator<VisualScriptPin>::Begin()
    .Field("defaultValue", &VisualScriptPin::defaultValue)
    .Field("defaultValueVisible", [](const VisualScriptPin* pin) { return pin->GetConnectedSet().empty(); }, nullptr) //{ return pin->GetConnectedTo() == nullptr; }, nullptr)
    .Field("pinValue", &VisualScriptPin::GetValue, nullptr)
    .End();
}

VisualScriptPin::VisualScriptPin(VisualScriptNode* owner_, Attribute attr_, const FastName& name_, const Type* type_, DefaultParam defaultParam_)
    : owner(owner_)
    , attr(attr_)
    , name(name_)
    , type(type_)
    , defaultParam(defaultParam_)
{
}

VisualScriptPin::~VisualScriptPin()
{
}

/*
     Data pins allow only 1 input pin
*/

VisualScriptPin::CanConnectResult VisualScriptPin::CanConnect(VisualScriptPin* pin1, VisualScriptPin* pin2)
{
    if (pin1->attr == pin2->attr)
        return CanConnectResult::CANNOT_CONNECT;

    VisualScriptPin* inputPin = pin1;
    VisualScriptPin* outputPin = pin2;
    if (pin2->IsInputPin())
    {
        inputPin = pin2;
        outputPin = pin1;
    }

    if (inputPin->IsDataPin() && outputPin->IsDataPin())
    {
        return CanConnectDataPinsOutputToInput(outputPin, inputPin);
    }
    else if (inputPin->IsExecutionPin() && outputPin->IsExecutionPin())
    {
        return CanConnectResult::CAN_CONNECT;
    }

    return CanConnectResult::CANNOT_CONNECT;
}

VisualScriptPin::CanConnectResult VisualScriptPin::CanConnectDataPinsOutputToInput(VisualScriptPin* outputPin, VisualScriptPin* inputPin)
{
    if (outputPin->type == inputPin->type)
        return CanConnectResult::CAN_CONNECT;
    if (outputPin->type == inputPin->type->Decay())
        return CanConnectResult::CAN_CONNECT;
    if (outputPin->type->Pointer() == inputPin->type)
        return CanConnectResult::CAN_CONNECT;
    if (outputPin->type == inputPin->type->Pointer())
        return CanConnectResult::CAN_CONNECT;
    if (TypeInheritance::CanCast(outputPin->type, inputPin->type))
        return CanConnectResult::CAN_CONNECT_WITH_CAST;
    if (inputPin->type->Pointer() && TypeInheritance::CanCast(outputPin->type, inputPin->type->Pointer()))
        return CanConnectResult::CAN_CONNECT_WITH_CAST;
    if (outputPin->type->Pointer() && TypeInheritance::CanCast(outputPin->type->Pointer(), inputPin->type))
        return CanConnectResult::CAN_CONNECT_WITH_CAST;

#define CHECK_CAST(from, to)                                                                     \
    if ((outputPin->type == Type::Instance<from>()) && (inputPin->type == Type::Instance<to>())) \
        return CanConnectResult::CAN_CONNECT_WITH_CAST;                                                            \
    if ((outputPin->type == Type::Instance<to>()) && (inputPin->type == Type::Instance<from>())) \
        return CanConnectResult::CAN_CONNECT_WITH_CAST;

    CHECK_CAST(bool, int32);
    CHECK_CAST(bool, uint32);
    CHECK_CAST(int32, uint32);
    CHECK_CAST(int32, float32);
    CHECK_CAST(String, const char*);
    CHECK_CAST(String, FastName);
    CHECK_CAST(String, int32);
    CHECK_CAST(String, uint32);
    CHECK_CAST(String, float32);

#undef CHECK_CAST

    return CanConnectResult::CANNOT_CONNECT;
}

bool VisualScriptPin::ConnectDataPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin)
{
    // DATA PINS
    if (CanConnectDataPinsOutputToInput(outputPin, inputPin) == CanConnectResult::CANNOT_CONNECT)
    {
        return false;
    }
    /*
     If something connected to input pin, disconnect it first
     */
    DVASSERT(inputPin->connectedTo.size() <= 1);

    if (inputPin->connectedTo.size() == 1)
    {
        inputPin->Disconnect(*inputPin->connectedTo.begin());
    }

    DVASSERT(inputPin->connectedTo.size() == 0);
    // Connect pins
    if (inputPin->connectedTo.size() == 0)
    {
        //
        inputPin->connectedTo.insert(outputPin);
        outputPin->connectedTo.insert(inputPin);

        return true;
    }

    return false;
}

/*
     Exec pins allow only 1 out pin
 */
bool VisualScriptPin::ConnectExecPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin)
{
    DVASSERT(outputPin->connectedTo.size() <= 1);
    if (outputPin->connectedTo.size() == 1)
    {
        outputPin->Disconnect(*outputPin->connectedTo.begin());
    }
    DVASSERT(outputPin->connectedTo.size() == 0);
    if (outputPin->connectedTo.size() == 0)
    {
        inputPin->connectedTo.insert(outputPin);
        outputPin->connectedTo.insert(inputPin);
        return true;
    }
    return false;
}

bool VisualScriptPin::Connect(VisualScriptPin* otherPin)
{
    DVASSERT(otherPin != nullptr);

    if (this->attr == otherPin->attr)
        return false;

    VisualScriptPin* inputPin = otherPin;
    VisualScriptPin* outputPin = this;
    if ((this->attr == Attribute::ATTR_IN) || (this->attr == Attribute::EXEC_IN))
    {
        inputPin = this;
        outputPin = otherPin;
    }
    if (inputPin->IsDataPin() && outputPin->IsDataPin())
        return ConnectDataPins(inputPin, outputPin);
    else
        return ConnectExecPins(inputPin, outputPin);
}

bool VisualScriptPin::IsInputPin() const
{
    return (attr == Attribute::EXEC_IN) || (attr == Attribute::ATTR_IN);
}

bool VisualScriptPin::IsOutputPin() const
{
    return (attr == Attribute::EXEC_OUT) || (attr == Attribute::ATTR_OUT);
}

bool VisualScriptPin::IsExecutionPin() const
{
    return (attr == Attribute::EXEC_IN) || (attr == Attribute::EXEC_OUT);
}

bool VisualScriptPin::IsDataPin() const
{
    return (attr == Attribute::ATTR_IN) || (attr == Attribute::ATTR_OUT);
}

bool VisualScriptPin::IsConnected(VisualScriptPin* pin1, VisualScriptPin* pin2)
{
    size_t c1 = pin1->connectedTo.count(pin2);
    size_t c2 = pin2->connectedTo.count(pin1);
    DVASSERT(c1 == c2);
    if (c1 > 0 && c1 == c2)
        return true;
    return false;
}

void VisualScriptPin::Disconnect(VisualScriptPin* disconnectFrom)
{
    connectedTo.erase(disconnectFrom);
    disconnectFrom->connectedTo.erase(this);
}

const Any& VisualScriptPin::GetValue() const
{
    if (attr == Attribute::ATTR_IN)
    {
        VisualScriptPin* outputPin = GetConnectedTo();
        if (outputPin)
            return outputPin->GetValue();
        else
        {
            DVASSERT(defaultParam == DefaultParam::DEFAULT_PARAM);
            return defaultValue;
        }
    }
    else if (attr == Attribute::ATTR_OUT)
    {
        return value;
    }
    DVASSERT(0 && "Called get value for execution nodes");
    static Any dummy;
    return dummy;
}

} //DAVA
