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

    void SetType(eType type);
    eType GetType() const;
    const FastName& GetTypeName();

    void SetName(const FastName& name);
    const FastName& GetName() const;

    VisualScriptPin* RegisterPin(VisualScriptPin* pin);
    void UnregisterPin(VisualScriptPin* pin);

    VisualScriptPin* GetPinByName(const FastName& pinName);
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
    Set<std::pair<VisualScriptPin*, VisualScriptPin*>> GetAllConnections() const;

    void SetScript(VisualScript* script);
    VisualScript* GetScript() const;

    virtual void Save(YamlNode* node) const;
    virtual void Load(const YamlNode* node);

protected:
    void SaveDefaults(YamlNode* node) const;
    void LoadDefaults(const YamlNode* node);

    eType type = NONE;
    VisualScript* script = nullptr;
    FastName name;

    Vector<VisualScriptPin*> dataInputPins;
    Vector<VisualScriptPin*> dataOutputPins;
    Vector<VisualScriptPin*> execInPins;
    Vector<VisualScriptPin*> execOutPins;

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
