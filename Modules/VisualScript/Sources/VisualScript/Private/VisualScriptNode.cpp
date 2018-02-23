#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptNode.h"
#include "VisualScript/VisualScriptPin.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/ReflectedStructure.h"
#include "FileSystem/YamlNode.h"
#include "Job/JobManager.h"

#include <locale>

namespace DAVA
{
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
        if (pin->GetExecutionOwner() == this)
        {
            delete pin;
        }
    }
}

void VisualScriptNode::SetReflection(const Reflection& ref_)
{
    ref = ref_;
};
void VisualScriptNode::SetFunction(VisualScriptFunction* function_)
{
    function = function_;
};

VisualScriptFunction* VisualScriptNode::GetFunction() const
{
    return function;
};

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
};

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
};

VisualScriptPin* VisualScriptNode::GetDataOutputPin(uint32 index) const
{
    return dataOutputPins[index];
};

VisualScriptPin* VisualScriptNode::GetExecInputPin(uint32 index) const
{
    return execInPins[index];
}

VisualScriptPin* VisualScriptNode::GetExecOutputPin(uint32 index) const
{
    return execOutPins[index];
}

Reflection& VisualScriptNode::GetReflection()
{
    return ref;
};

const Vector<VisualScriptPin*>& VisualScriptNode::GetDataInputPins() const
{
    return dataInputPins;
};

const Vector<VisualScriptPin*>& VisualScriptNode::GetDataOutputPins() const
{
    return dataOutputPins;
};
const Vector<VisualScriptPin*>& VisualScriptNode::GetExecInputPins() const
{
    return execInPins;
};

const Vector<VisualScriptPin*>& VisualScriptNode::GetExecOutputPins() const
{
    return execOutPins;
};

void VisualScriptNode::SetLastExecutionFrame(uint32 lastExecutionFrame_)
{
    lastExecutionFrame = lastExecutionFrame_;
};

uint32 VisualScriptNode::GetLastExecutionFrame() const
{
    return lastExecutionFrame;
};

void VisualScriptNode::SetScript(VisualScript* script_)
{
    script = script_;
}

VisualScript* VisualScriptNode::GetScript() const
{
    return script;
}

Set<std::pair<VisualScriptPin*, VisualScriptPin*>> VisualScriptNode::GetAllConnections()
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

Result VisualScriptNode::GetCompileResult() const
{
    return Result();
}

void VisualScriptNode::Save(YamlNode* node) const
{
    node->Add("position", position);
}

void VisualScriptNode::Load(const YamlNode* node)
{
    position = node->Get("position")->AsVector2();
}

// Visual Script Node implementations

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptGetVarNode)
{
    ReflectionRegistrator<VisualScriptGetVarNode>::Begin()
    .ConstructorByPointer<>()
    .ConstructorByPointer<const Reflection&, const FastName&>()
    .Field("varPath", &VisualScriptGetVarNode::varPath)
    .End();
}

VisualScriptGetVarNode::VisualScriptGetVarNode()
{
    SetType(GET_VAR);
    outValuePin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("value"), nullptr));
}

VisualScriptGetVarNode::VisualScriptGetVarNode(const Reflection& ref, const FastName& varPath)
    : VisualScriptGetVarNode()
{
    SetVarPath(varPath);
    BindReflection(ref);
}

void VisualScriptGetVarNode::SetVarPath(const DAVA::FastName& varPath_)
{
    varPath = varPath_;

    String newName = String("Get ") + varPath.c_str();
    SetName(FastName(newName));
}

const FastName& VisualScriptGetVarNode::GetVarPath() const
{
    return varPath;
}

void VisualScriptGetVarNode::BindReflection(const Reflection& ref)
{
    Reflection refField = ref.GetField(varPath);
    const Type* type = refField.GetValue().GetType();
    outValuePin->SetType(type);
    SetReflection(refField);
}

void VisualScriptGetVarNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("varPath", varPath.c_str());
}

void VisualScriptGetVarNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetVarPath(node->Get("varPath")->AsFastName());
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptSetVarNode)
{
    ReflectionRegistrator<VisualScriptSetVarNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const Reflection&, const FastName&>()
    .Field("varPath", &VisualScriptSetVarNode::varPath)
    .End();
}

VisualScriptSetVarNode::VisualScriptSetVarNode()
{
    SetType(SET_VAR);

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    varInPin = new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("set"), nullptr, VisualScriptPin::DEFAULT_PARAM);
    RegisterPin(varInPin);

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));
    varOutPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("get"), nullptr));
}

VisualScriptSetVarNode::VisualScriptSetVarNode(const Reflection& ref, const FastName& varPath)
    : VisualScriptSetVarNode()
{
    SetVarPath(varPath);
    BindReflection(ref);
}

void VisualScriptSetVarNode::SetVarPath(const DAVA::FastName& varPath_)
{
    varPath = varPath_;

    String newName = String("Set ") + varPath.c_str();
    SetName(FastName(newName));
}

const FastName& VisualScriptSetVarNode::GetVarPath() const
{
    return varPath;
}

void VisualScriptSetVarNode::BindReflection(const Reflection& ref_)
{
    Reflection fieldReflection = ref_.GetField(varPath);
    const Type* type = fieldReflection.GetValue().GetType();
    ref = fieldReflection;

    varInPin->SetType(type);
    //    varInPin->SetName(varPath);

    varOutPin->SetType(type);
    //    varOutPin->SetName(varPath);
}

VisualScriptSetVarNode::~VisualScriptSetVarNode()
{
}

void VisualScriptSetVarNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("varPath", varPath.c_str());
}

void VisualScriptSetVarNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetVarPath(node->Get("varPath")->AsFastName());
}

//
// VisualScriptGetMemberNode
//

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptGetMemberNode)
{
    ReflectionRegistrator<VisualScriptGetMemberNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptGetMemberNode::VisualScriptGetMemberNode()
{
    SetType(GET_MEMBER);
}

VisualScriptGetMemberNode::VisualScriptGetMemberNode(const FastName& className_, const FastName& fieldName_)
    : VisualScriptGetMemberNode()
{
    SetClassName(className_);
    SetFieldName(fieldName_);
    InitPins();
}

VisualScriptGetMemberNode::~VisualScriptGetMemberNode() = default;

void VisualScriptGetMemberNode::SetClassName(const FastName& className_)
{
    className = className_;
}

const FastName& VisualScriptGetMemberNode::GetClassName() const
{
    return className;
}

void VisualScriptGetMemberNode::SetFieldName(const FastName& fieldName_)
{
    fieldName = fieldName_;
}

const FastName& VisualScriptGetMemberNode::GetFieldName() const
{
    return fieldName;
}

void VisualScriptGetMemberNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("className", className.c_str());
    node->Add("fieldName", fieldName.c_str());
}

void VisualScriptGetMemberNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetClassName(node->Get("className")->AsFastName());
    SetFieldName(node->Get("fieldName")->AsFastName());
    InitPins();
}

const ValueWrapper* VisualScriptGetMemberNode::GetValueWrapper() const
{
    return valueWrapper;
}

void VisualScriptGetMemberNode::InitPins()
{
    if (className.IsValid() && fieldName.IsValid())
    {
        String fName = fieldName.c_str();
        {
            fName[0] = std::toupper(fName[0], std::locale());
        }
        String nodeName = className.c_str() + String("::Get") + fName;
        name = FastName(nodeName);

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        const ReflectedStructure* rs = type->GetStructure();
        const ValueWrapper* vw = nullptr;
        for (auto& field : rs->fields)
        {
            if (field->name == fieldName)
            {
                vw = field->valueWrapper.get();
                break;
            }
        }
        if (!vw)
        {
            Logger::Error("Failed to find value wrapper for %s in class %s", fieldName.c_str(), className.c_str());
        }
        InitNodeWithValueWrapper(vw);
    }
}

void VisualScriptGetMemberNode::InitNodeWithValueWrapper(const ValueWrapper* wrapper)
{
    valueWrapper = wrapper;

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("object"), ReflectedTypeDB::GetByPermanentName(className.c_str())->GetType()));

    const Type* mType = valueWrapper->GetType(ReflectedObject());
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("get"), mType));
}

//
// VisualScriptSetMemberNode
//

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptSetMemberNode)
{
    ReflectionRegistrator<VisualScriptSetMemberNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptSetMemberNode::VisualScriptSetMemberNode()
{
    SetType(SET_MEMBER);
}

VisualScriptSetMemberNode::VisualScriptSetMemberNode(const FastName& className_, const FastName& fieldName_)
    : VisualScriptSetMemberNode()
{
    SetClassName(className_);
    SetFieldName(fieldName_);
    InitPins();
}

VisualScriptSetMemberNode::~VisualScriptSetMemberNode() = default;

void VisualScriptSetMemberNode::SetClassName(const FastName& className_)
{
    className = className_;
}

const FastName& VisualScriptSetMemberNode::GetClassName() const
{
    return className;
}

void VisualScriptSetMemberNode::SetFieldName(const FastName& fieldName_)
{
    fieldName = fieldName_;
}

const FastName& VisualScriptSetMemberNode::GetFieldName() const
{
    return fieldName;
}

void VisualScriptSetMemberNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("className", className.c_str());
    node->Add("fieldName", fieldName.c_str());
}

void VisualScriptSetMemberNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    SetClassName(node->Get("className")->AsFastName());
    SetFieldName(node->Get("fieldName")->AsFastName());
    InitPins();
}

const ValueWrapper* VisualScriptSetMemberNode::GetValueWrapper() const
{
    return valueWrapper;
}

void VisualScriptSetMemberNode::InitPins()
{
    if (className.IsValid() && fieldName.IsValid())
    {
        String fName = fieldName.c_str();
        {
            fName[0] = std::toupper(fName[0], std::locale());
        }
        String nodeName = className.c_str() + String("::Set") + fName;
        name = FastName(nodeName);

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        const ReflectedStructure* rs = type->GetStructure();
        const ValueWrapper* vw = nullptr;
        for (auto& field : rs->fields)
        {
            if (field->name == fieldName)
            {
                vw = field->valueWrapper.get();
                break;
            }
        }
        if (!vw)
        {
            Logger::Error("Failed to find value wrapper for %s in class %s", fieldName.c_str(), className.c_str());
        }
        InitNodeWithValueWrapper(vw);
    }
}

void VisualScriptSetMemberNode::InitNodeWithValueWrapper(const ValueWrapper* wrapper)
{
    valueWrapper = wrapper;

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("object"), ReflectedTypeDB::GetByPermanentName(className.c_str())->GetType()));

    const Type* mType = valueWrapper->GetType(ReflectedObject());
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("set"), mType, VisualScriptPin::DEFAULT_PARAM));
    RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("get"), mType));
}

//
// VisualScriptFunctionNode
//

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptFunctionNode)
{
    ReflectionRegistrator<VisualScriptFunctionNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptFunctionNode::VisualScriptFunctionNode()
{
    SetType(FUNCTION);
}

VisualScriptFunctionNode::VisualScriptFunctionNode(const FastName& className_, const FastName& functionName_)
    : VisualScriptFunctionNode()
{
    SetClassName(className_);
    SetFunctionName(functionName_);
    InitPins();
}

void VisualScriptFunctionNode::SetClassName(const FastName& className_)
{
    className = className_;
}
void VisualScriptFunctionNode::SetFunctionName(const FastName& functionName_)
{
    functionName = functionName_;
}

const FastName& VisualScriptFunctionNode::GetClassName() const
{
    return className;
}

const FastName& VisualScriptFunctionNode::GetFunctionName() const
{
    return functionName;
}

void VisualScriptFunctionNode::InitPins()
{
    if (className.IsValid() && functionName.IsValid())
    {
        String nodeName = className.c_str();
        nodeName += String("::") + functionName.c_str();
        name = FastName(nodeName);

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        AnyFn function = type->GetStructure()->GetMethod(functionName);
        if (function.IsValid() == false)
        {
            Logger::Error("Failed to find function %s in class %s", functionName.c_str(), className.c_str());
        }
        InitNodeWithAnyFn(function);
    }
}

void VisualScriptFunctionNode::InitNodeWithAnyFn(const AnyFn& function)
{
    if (!function.IsConst() && !function.IsStatic())
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));

    auto& functionTypes = function.GetInvokeParams().GetArgumentTypes();
    uint32 index = 0;

    for (auto type : functionTypes)
    {
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName(Format("arg%d", index++)), type));
        //Logger::Debug("arg%d - %s", index - 1, type->GetDemangledName().c_str());
    }

    if (!function.IsConst() && !function.IsStatic())
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));

    const Type* returnType = function.GetInvokeParams().GetReturnType();
    if (returnType != Type::Instance<void>())
    {
        RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("result"), returnType));
    }

    SetFunction(new VisualScriptFunction(function));
}

VisualScriptFunctionNode::~VisualScriptFunctionNode()
{
}

void VisualScriptFunctionNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);

    node->Add("className", className.c_str());
    node->Add("functionName", functionName.c_str());
}

void VisualScriptFunctionNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);

    className = FastName(node->Get("className")->AsString());
    functionName = FastName(node->Get("functionName")->AsString());
    InitPins();
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptBranchNode)
{
    ReflectionRegistrator<VisualScriptBranchNode>::Begin()
    .ConstructorByPointer()
    .End();
}

/*String VisualScriptFunctionNode::GetName() const
{
    if (className.IsValid())
    {
        String c = className.c_str();
        return c + "::" + functionName.c_str();
    }

    return functionName.c_str();
}*/

VisualScriptBranchNode::VisualScriptBranchNode()
    : VisualScriptNode()
{
    SetType(BRANCH);
    SetName(GetTypeName());

    RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    conditionIn = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("condition"), Type::Instance<bool>()));
    execOutTrue = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("true"), nullptr));
    execOutFalse = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("false"), nullptr));
}

VisualScriptBranchNode::~VisualScriptBranchNode()
{
}

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

VisualScriptEventNode::~VisualScriptEventNode()
{
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
    //DVASSERT(eventOutputs.size() == dataOutputPins.size());
    for (size_t i = 0; i < dataOutputPins.size(); ++i)
    {
        Reflection refField = reflection.GetField(dataOutputPins[i]->GetName());
        DVASSERT(refField.IsValid());
        dataOutputPins[i]->SetType(refField.GetValueType());
        dataOutputPins[i]->SetValue(refField.GetValue());
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptAnotherScriptNode)
{
    ReflectionRegistrator<VisualScriptAnotherScriptNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptAnotherScriptNode::VisualScriptAnotherScriptNode()
{
    SetType(SCRIPT);
}

VisualScriptAnotherScriptNode::~VisualScriptAnotherScriptNode()
{
}

void VisualScriptAnotherScriptNode::SetScriptFilepath(const FilePath& scriptFilepath_)
{
    scriptFilepath = scriptFilepath_;
    SetName(FastName(scriptFilepath.GetBasename()));
    anotherScript = GetEngineContext()->assetManager->LoadAsset<VisualScript>(scriptFilepath,
                                                                              nullptr, false);
    // MakeFunction(this, &VisualScriptAnotherScriptNode::AssetLoadedCallback);
    GetEngineContext()->jobManager->WaitWorkerJobs();
    AssetLoadedCallback(anotherScript);
}

const FilePath& VisualScriptAnotherScriptNode::GetScriptFilepath() const
{
    return scriptFilepath;
}

void VisualScriptAnotherScriptNode::AssetLoadedCallback(Asset<AssetBase> asset)
{
    DVASSERT(asset == anotherScript);
    const Vector<VisualScriptNode*>& nodes = anotherScript->GetNodes();

    Vector<VisualScriptPin*> allInputPins;
    Vector<VisualScriptPin*> allOutputPins;

    uint32 execInputPins = 0;
    for (const auto& node : nodes)
    {
        const Vector<VisualScriptPin*>& inputPins = node->GetAllInputPins();
        /*
         //
         // More than one exec pin? (is it a problem or not?)
         // For now decided that is not
         //

        for (const auto& inPin: inputPins)
        {
            if (inPin->GetAttribute() == VisualScriptPin::EXEC_IN)
                execInputPins++;
        }*/
        for (const auto& inPin : inputPins)
        {
            if (inPin->GetConnectedSet().size() == 0)
                allInputPins.emplace_back(inPin);
        }

        const Vector<VisualScriptPin*>& outputPins = node->GetAllOutputPins();
        for (const auto& outPin : outputPins)
        {
            if (outPin->GetConnectedSet().size() == 0)
                allOutputPins.emplace_back(outPin);
        }
    }

    /*
        Here we take all open unused pins from script we've loaded and expose them as input & output parameters of our node.
        We copy it's original pins to our node. After that we setup serialization owner. It means that
        when owner will serialize this script, all connections connected to that internal nodes, will be
        treated as connections to this AnotherScriptNode.
     
        Execution of nodes work automagically because during compilation phase, script compiles as it has
        all nodes included to itself.
     */
    uint32 index = 0;
    for (auto& pin : allInputPins)
    {
        /*
         TODO: We can unify all names in one place. Instead of fragmented code.
         It's only 2-3 places, so not critical right now.
         */
        pin->SetName(FastName(Format("arg%d", index++)));
        RegisterPin(pin);
        pin->SetSerializationOwner(this);
    }

    index = 0;
    for (auto& pin : allOutputPins)
    {
        pin->SetName(FastName(Format("res%d", index++)));
        RegisterPin(pin);
        pin->SetSerializationOwner(this);
    }
}

void VisualScriptAnotherScriptNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("scriptName", scriptFilepath.GetAbsolutePathname());
}

void VisualScriptAnotherScriptNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);

    scriptFilepath = FilePath(node->Get("scriptName")->AsString());
    SetScriptFilepath(scriptFilepath);
}

void VisualScriptAnotherScriptNode::BindReflection(const Reflection& reflection)
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptWhileNode)
{
    ReflectionRegistrator<VisualScriptWhileNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptWhileNode::VisualScriptWhileNode()
{
    SetType(WHILE);
    SetName(GetTypeName());

    execPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    conditionPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("condition"), Type::Instance<bool>()));
    loopBodyPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("loop body"), nullptr));
    loopCompletedPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("completed"), nullptr));

    cyclePin = new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("cycle"), nullptr);
}

VisualScriptWhileNode::~VisualScriptWhileNode()
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptForNode)
{
    ReflectionRegistrator<VisualScriptForNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptForNode::VisualScriptForNode()
{
    SetType(FOR);
    SetName(GetTypeName());

    execPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
    firstIndexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("first index"), Type::Instance<int32>()));
    lastIndexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, FastName("last index"), Type::Instance<int32>()));
    breakPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("break"), nullptr));

    // hidden pin
    cyclePin = new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("cycle"), nullptr);

    loopBodyPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("loop body"), nullptr));
    indexPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("index"), Type::Instance<int32>()));
    loopCompletedPin = RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("completed"), nullptr));
}

VisualScriptForNode::~VisualScriptForNode()
{
}

} //DAVA
