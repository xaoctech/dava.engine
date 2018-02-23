#pragma once
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class VisualScriptNode;

class VisualScriptPin final
{
public:
    enum eAttribute
    {
        ATTR_IN,
        ATTR_OUT,
        EXEC_IN,
        EXEC_OUT,
    };

    enum eDefaultParam
    {
        NO_DEFAULT_PARAM,
        DEFAULT_PARAM,
    };

    VisualScriptPin(VisualScriptNode* owner_, eAttribute attr_, const FastName& name_, const Type* type_, eDefaultParam defaultParam_ = NO_DEFAULT_PARAM);
    ~VisualScriptPin();

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

    enum eCanConnectResult
    {
        CANNOT_CONNECT = 0,
        CAN_CONNECT = 1,
        CAN_CONNECT_WITH_CAST = 2,
    };

    static eCanConnectResult CanConnect(VisualScriptPin* pin1, VisualScriptPin* pin2);
    static eCanConnectResult CanConnectDataPinsOutputToInput(VisualScriptPin* outputPin, VisualScriptPin* inputPin);

    bool Connect(VisualScriptPin* connectTo);
    void Disconnect(VisualScriptPin* disconnectFrom);
    bool IsExecutionPin() const;
    bool IsDataPin() const;
    bool IsInputPin() const;
    bool IsOutputPin() const;

    static bool IsConnected(VisualScriptPin* pin1, VisualScriptPin* pin2);

    VisualScriptPin* GetConnectedTo() const;
    const Set<VisualScriptPin*>& GetConnectedSet() const;

    void SetType(const Type* type);
    const Type* GetType() const;

    void SetName(const FastName& name_);
    const FastName& GetName() const;

    eAttribute GetAttribute() const;

    bool IsInputDataRequired() const;

    bool HasDefaultValue() const;
    void SetDefaultValue(Any defaultValue_);
    void SetValue(Any value_);
    const Any& GetValue() const;

    DAVA_REFLECTION(VisualScriptPin);

private:
    static bool ConnectDataPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);
    static bool ConnectExecPins(VisualScriptPin* inputPin, VisualScriptPin* outputPin);

    VisualScriptNode* owner = nullptr;
    VisualScriptNode* serializationOwner = nullptr; // Required for serialization & deserialization

    eAttribute attr = ATTR_IN;
    FastName name;
    const Type* type = nullptr;
    eDefaultParam defaultParam = NO_DEFAULT_PARAM;
    Set<VisualScriptPin*> connectedTo;
    Any defaultValue;
    Any value;
};

} //DAVA
