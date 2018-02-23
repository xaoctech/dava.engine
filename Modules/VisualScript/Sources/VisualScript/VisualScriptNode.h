#pragma once

#include <Base/Any.h>
#include <Base/AnyFn.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/Result.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
/*
     Value, values -> return multiple values
     Object -> is it something separate from value?
     Condition -> eval, change execution direction
     Function -> eval, out parameters
 */
class VisualScript;
class VisualScriptPin;
class VisualScriptNode;
class YamlNode;
class VisualScriptExecutor;

class VisualScriptNode : public ReflectionBase
{
public:
    DAVA_VIRTUAL_REFLECTION(VisualScriptNode);

    enum eType
    {
        NONE,
        GET_VAR,
        SET_VAR,
        FUNCTION,
        BRANCH,
        WHILE,
        DO_N,
        FOR,
        WAIT,
        EVENT,
        CUSTOM_EVENT,
        SCRIPT,
        GET_MEMBER,
        SET_MEMBER,

        TYPE_COUNT,
    };

    VisualScriptNode();
    virtual ~VisualScriptNode();

    void SetReflection(const Reflection& ref_);
    Reflection& GetReflection();

    virtual void BindReflection(const Reflection& ref){};

    void SetFunction(const AnyFn& func);
    const AnyFn& GetFunction() const;

    void SetType(eType type);
    eType GetType() const;

    const FastName& GetTypeName();

    void SetName(const FastName& name);
    const FastName& GetName() const;

    VisualScriptPin* RegisterPin(VisualScriptPin* pin);
    VisualScriptPin* GetPinByName(const FastName& pinName);
    void UnregisterPin(VisualScriptPin* pin);

    VisualScriptPin* GetDataInputPin(uint32 index) const;
    VisualScriptPin* GetDataOutputPin(uint32 index) const;
    VisualScriptPin* GetExecInputPin(uint32 index) const;
    VisualScriptPin* GetExecOutputPin(uint32 index) const;

    const Vector<VisualScriptPin*>& GetAllInputPins() const;
    const Vector<VisualScriptPin*>& GetAllOutputPins() const;
    const Vector<VisualScriptPin*>& GetDataInputPins() const;
    const Vector<VisualScriptPin*>& GetDataOutputPins() const;
    const Vector<VisualScriptPin*>& GetExecInputPins() const;
    const Vector<VisualScriptPin*>& GetExecOutputPins() const;

    void SetLastExecutionFrame(uint32 lastExecutionFrame_);
    uint32 GetLastExecutionFrame() const;

    void SetScript(VisualScript* script);
    VisualScript* GetScript() const;

    Set<std::pair<VisualScriptPin*, VisualScriptPin*>> GetAllConnections() const;

    virtual void Save(YamlNode* node) const;
    virtual void Load(const YamlNode* node);

    Result GetCompileResult() const;

protected:
    eType type = NONE;
    FastName name;
    AnyFn function;
    Reflection ref; // Check what to do with that?
    VisualScript* script;

    Vector<VisualScriptPin*> dataInputPins;
    Vector<VisualScriptPin*> dataOutputPins;
    Vector<VisualScriptPin*> execInPins;
    Vector<VisualScriptPin*> execOutPins;
    uint32 lastExecutionFrame = 0;

    Vector<VisualScriptPin*> allInputPins;
    Vector<VisualScriptPin*> allOutputPins;
    UnorderedMap<FastName, VisualScriptPin*> allPinsMap;

    static FastName typeNames[];

    // Think where it should be
    Vector<VisualScriptNode*> compiledExecOrder;
    friend class VisualScriptExecutor;

public:
    Vector2 position;
};












} //DAVA
