#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetBase.h"
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/FastName.h"
#include "Base/Result.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{
/*
     Value, values -> return multiple values
     Object -> is it something separate from value?
     Condition -> eval, change execution direction
     Function -> eval, outparameters
 */
class VisualScript;
class VisualScriptPin;
class VisualScriptNode;
class YamlNode;
class VisualScriptExecutor;

enum VisualScriptFunctionResult
{
    NOT_FIRE_EXEC_OUT = 0,
    FIRE_EXEC_OUT = 1,
};

class VisualScriptFunction final
{
public:
    VisualScriptFunction(const AnyFn& function_)
        : function(function_)
    {
    }

    AnyFn function;
};

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

    void SetFunction(VisualScriptFunction* function_);
    VisualScriptFunction* GetFunction() const;

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

    Set<std::pair<VisualScriptPin*, VisualScriptPin*>> GetAllConnections();

    virtual void Save(YamlNode* node) const;
    virtual void Load(const YamlNode* node);

    Result GetCompileResult() const;

protected:
    eType type = NONE;
    FastName name;
    VisualScriptFunction* function = nullptr;
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

class VisualScriptGetVarNode : public VisualScriptNode
{
public:
    VisualScriptGetVarNode();
    VisualScriptGetVarNode(const Reflection& ref, const FastName& varPath);
    ~VisualScriptGetVarNode() = default;

    void SetVarPath(const FastName& varPath);
    const FastName& GetVarPath() const;

    void BindReflection(const Reflection& ref) override;
    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    FastName varPath;
    VisualScriptPin* outValuePin = nullptr;

    DAVA_VIRTUAL_REFLECTION(VisualScriptGetVarNode, VisualScriptNode);
};

class VisualScriptSetVarNode : public VisualScriptNode
{
public:
    VisualScriptSetVarNode();
    VisualScriptSetVarNode(const Reflection& ref, const FastName& varPath);
    ~VisualScriptSetVarNode();

    void SetVarPath(const FastName& varPath_);
    const FastName& GetVarPath() const;

    void BindReflection(const Reflection& ref) override;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    FastName varPath;
    VisualScriptPin *varInPin, *varOutPin;

    DAVA_VIRTUAL_REFLECTION(VisualScriptSetVarNode, VisualScriptNode);
};

class VisualScriptGetMemberNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptGetMemberNode, VisualScriptNode);

public:
    VisualScriptGetMemberNode();
    VisualScriptGetMemberNode(const FastName& className, const FastName& fieldName);
    ~VisualScriptGetMemberNode() override;

    void SetClassName(const FastName& className);
    const FastName& GetClassName() const;

    void SetFieldName(const FastName& fieldName);
    const FastName& GetFieldName() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

    const ValueWrapper* GetValueWrapper() const;

public:
    void InitPins();
    void InitNodeWithValueWrapper(const ValueWrapper* wrapper);

    FastName className;
    FastName fieldName;

    const ValueWrapper* valueWrapper = nullptr;
};

class VisualScriptSetMemberNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptSetMemberNode, VisualScriptNode);

public:
    VisualScriptSetMemberNode();
    VisualScriptSetMemberNode(const FastName& className, const FastName& fieldName);
    ~VisualScriptSetMemberNode() override;

    void SetClassName(const FastName& className);
    const FastName& GetClassName() const;

    void SetFieldName(const FastName& fieldName);
    const FastName& GetFieldName() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

    const ValueWrapper* GetValueWrapper() const;

public:
    void InitPins();
    void InitNodeWithValueWrapper(const ValueWrapper* wrapper);

    FastName className;
    FastName fieldName;

    const ValueWrapper* valueWrapper = nullptr;
};

class VisualScriptFunctionNode : public VisualScriptNode
{
public:
    VisualScriptFunctionNode();
    VisualScriptFunctionNode(const FastName& className_, const FastName& functionName_);

    ~VisualScriptFunctionNode();

    void SetClassName(const FastName& className);
    void SetFunctionName(const FastName& functionName);

    const FastName& GetClassName() const;
    const FastName& GetFunctionName() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    void InitPins();
    void InitNodeWithAnyFn(const AnyFn& function);

    FastName className;
    FastName functionName;

    DAVA_VIRTUAL_REFLECTION(VisualScriptFunctionNode, VisualScriptNode);
};

class VisualScriptBranchNode : public VisualScriptNode
{
public:
    VisualScriptBranchNode();
    ~VisualScriptBranchNode();

    VisualScriptPin *execOutTrue = nullptr, *execOutFalse = nullptr, *conditionIn = nullptr;

private:
    DAVA_VIRTUAL_REFLECTION(VisualScriptBranchNode, VisualScriptNode);
};

class VisualScriptEventNode : public VisualScriptNode
{
public:
    VisualScriptEventNode();
    ~VisualScriptEventNode();

    void SetEventName(const FastName& eventName_);
    const FastName& GetEventName() const;

    void BindReflection(const Reflection& ref_) override;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

protected:
    FastName eventName;
    DAVA_VIRTUAL_REFLECTION(VisualScriptEventNode, VisualScriptNode);
};

class VisualScriptAnotherScriptNode : public VisualScriptNode
{
public:
    VisualScriptAnotherScriptNode();
    ~VisualScriptAnotherScriptNode();

    void SetScriptFilepath(const FilePath& scriptFilepath_);
    const FilePath& GetScriptFilepath() const;

    void BindReflection(const Reflection& ref_) override;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

protected:
    FilePath scriptFilepath;
    Asset<VisualScript> anotherScript;

    void AssetLoadedCallback(Asset<AssetBase> asset);

private:
    DAVA_VIRTUAL_REFLECTION(VisualScriptAnotherScriptNode, VisualScriptNode);
};

class VisualScriptWhileNode : public VisualScriptNode
{
public:
    VisualScriptWhileNode();
    ~VisualScriptWhileNode();

    VisualScriptPin *execPin = nullptr, *conditionPin = nullptr, *loopBodyPin = nullptr, *cyclePin = nullptr, *loopCompletedPin = nullptr;

private:
    DAVA_VIRTUAL_REFLECTION(VisualScriptWhileNode, VisualScriptNode);
};

class VisualScriptForNode : public VisualScriptNode
{
public:
    VisualScriptForNode();
    ~VisualScriptForNode();

    VisualScriptPin *execPin = nullptr, *breakPin = nullptr, *cyclePin = nullptr, *loopBodyPin = nullptr, *loopCompletedPin = nullptr;
    VisualScriptPin *firstIndexPin = nullptr, *lastIndexPin = nullptr, *indexPin = nullptr;
    int32 index = 0;

private:
    DAVA_VIRTUAL_REFLECTION(VisualScriptForNode, VisualScriptNode);
};

class VisualWaitNode : public VisualScriptNode
{
public:
    VisualWaitNode(VisualScript* script_);
    ~VisualWaitNode();

protected:
};

} //DAVA
