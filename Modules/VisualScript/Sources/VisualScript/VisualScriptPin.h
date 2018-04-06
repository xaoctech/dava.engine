#pragma once

#include <Base/Any.h>
#include <Base/AnyFn.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class VisualScriptNode;

class VisualScriptPin final
{
    DAVA_REFLECTION(VisualScriptPin);

public:
    enum class Attribute
    {
        ATTR_IN = 0,
        ATTR_OUT,
        EXEC_IN,
        EXEC_OUT,
    };

    enum class DefaultParam
    {
        NO_DEFAULT_PARAM = 0,
        DEFAULT_PARAM,
    };

    enum class CanConnectResult
    {
        CANNOT_CONNECT = 0,
        CAN_CONNECT = 1,
        CAN_CONNECT_WITH_CAST = 2,
    };

    VisualScriptPin(VisualScriptNode* owner_, Attribute attr_, const FastName& name_, const Type* type_, DefaultParam defaultParam_ = DefaultParam::NO_DEFAULT_PARAM);
    ~VisualScriptPin();

    void SetType(const Type* type);
    const Type* GetType() const;

    void SetName(const FastName& name_);
    const FastName& GetName() const;

    bool IsInputDataRequired() const;
    bool IsExecutionPin() const;
    bool IsDataPin() const;
    bool IsInputPin() const;
    bool IsOutputPin() const;

    bool Connect(VisualScriptPin* connectTo);
    void Disconnect(VisualScriptPin* disconnectFrom);
    VisualScriptPin* GetConnectedTo() const;
    const Set<VisualScriptPin*>& GetConnectedSet() const;

    Attribute GetAttribute() const;

    bool HasDefaultValue() const;
    void SetDefaultValue(Any defaultValue_);
    const Any& GetDefaultValue() const;

    void SetValue(const Any& value_);
    const Any& GetValue() const;

    /*
        Used for execution part of the script
     */
    VisualScriptNode* GetExecutionOwner() const;

    /*
        Required for AnotherScriptNodes when they use pins from other script.
        Used only for serialization & deserialization
        When it's null, returns default owner that is also execution owner.
     */
    void SetSerializationOwner(VisualScriptNode* serializationOwner_);
    VisualScriptNode* GetSerializationOwner() const;

    static bool IsConnected(VisualScriptPin* pin1, VisualScriptPin* pin2);
    static CanConnectResult CanConnect(VisualScriptPin* pin1, VisualScriptPin* pin2);
    static CanConnectResult CanConnectDataPinsOutputToInput(VisualScriptPin* outputPin, VisualScriptPin* inputPin);

private:
    static bool ConnectDataPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);
    static bool ConnectExecPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);

    VisualScriptNode* owner = nullptr;
    VisualScriptNode* serializationOwner = nullptr; // Required for serialization & deserialization

    const Type* type = nullptr;
    Attribute attr = Attribute::ATTR_IN;
    DefaultParam defaultParam = DefaultParam::NO_DEFAULT_PARAM;
    Set<VisualScriptPin*> connectedTo;
    FastName name;
    Any defaultValue;
    Any value;
};

inline VisualScriptPin* VisualScriptPin::GetConnectedTo() const
{
    DVASSERT(attr == Attribute::ATTR_IN || attr == Attribute::EXEC_OUT);
    if (connectedTo.size() != 1)
        return nullptr;
    return *connectedTo.begin();
}

inline const Set<VisualScriptPin*>& VisualScriptPin::GetConnectedSet() const
{
    return connectedTo;
}

inline void VisualScriptPin::SetType(const Type* type_)
{
    type = type_;
}

inline const Type* VisualScriptPin::GetType() const
{
    return type;
}

inline void VisualScriptPin::SetName(const FastName& name_)
{
    name = name_;
}

inline const FastName& VisualScriptPin::GetName() const
{
    return name;
}

inline VisualScriptNode* VisualScriptPin::GetExecutionOwner() const
{
    return owner;
};

inline void VisualScriptPin::SetSerializationOwner(VisualScriptNode* serializationOwner_)
{
    serializationOwner = serializationOwner_;
}

inline VisualScriptNode* VisualScriptPin::GetSerializationOwner() const
{
    if (serializationOwner == nullptr)
        return owner;
    return serializationOwner;
}

inline VisualScriptPin::Attribute VisualScriptPin::GetAttribute() const
{
    return attr;
}

inline bool VisualScriptPin::IsInputDataRequired() const
{
    return !(defaultParam == DefaultParam::DEFAULT_PARAM && defaultValue.IsEmpty() == false);
}

inline bool VisualScriptPin::HasDefaultValue() const
{
    return (defaultParam == DefaultParam::DEFAULT_PARAM);
}

inline void VisualScriptPin::SetDefaultValue(Any defaultValue_)
{
    defaultValue = defaultValue_;
}

inline const Any& VisualScriptPin::GetDefaultValue() const
{
    return defaultValue;
}

inline void VisualScriptPin::SetValue(const Any& value_)
{
    value = value_;
}

} //DAVA
