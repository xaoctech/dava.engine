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
    enum eAttribute
    {
        ATTR_IN = 0,
        ATTR_OUT,
        EXEC_IN,
        EXEC_OUT,
    };

    enum eDefaultParam
    {
        NO_DEFAULT_PARAM = 0,
        DEFAULT_PARAM,
    };

    enum eCanConnectResult
    {
        CANNOT_CONNECT = 0,
        CAN_CONNECT = 1,
        CAN_CONNECT_WITH_CAST = 2,
    };

    VisualScriptPin(VisualScriptNode* owner_, eAttribute attr_, const FastName& name_, const Type* type_, eDefaultParam defaultParam_ = NO_DEFAULT_PARAM);
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

    eAttribute GetAttribute() const;

    bool HasDefaultValue() const;
    void SetDefaultValue(Any defaultValue_);
    const Any& GetDefaultValue() const;

    void SetValue(Any value_);
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
    static eCanConnectResult CanConnect(VisualScriptPin* pin1, VisualScriptPin* pin2);
    static eCanConnectResult CanConnectDataPinsOutputToInput(VisualScriptPin* outputPin, VisualScriptPin* inputPin);

private:
    static bool ConnectDataPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);
    static bool ConnectExecPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);

    VisualScriptNode* owner = nullptr;
    VisualScriptNode* serializationOwner = nullptr; // Required for serialization & deserialization

    const Type* type = nullptr;
    eAttribute attr = ATTR_IN;
    eDefaultParam defaultParam = NO_DEFAULT_PARAM;
    Set<VisualScriptPin*> connectedTo;
    FastName name;
    Any defaultValue;
    Any value;
};

} //DAVA
