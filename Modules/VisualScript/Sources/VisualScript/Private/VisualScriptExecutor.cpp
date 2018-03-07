#include "VisualScript/VisualScriptExecutor.h"
#include "VisualScript/Nodes/VisualScriptAnotherScriptNode.h"
#include "VisualScript/Nodes/VisualScriptBranchNode.h"
#include "VisualScript/Nodes/VisualScriptEventNode.h"
#include "VisualScript/Nodes/VisualScriptForNode.h"
#include "VisualScript/Nodes/VisualScriptFunctionNode.h"
#include "VisualScript/Nodes/VisualScriptGetMemberNode.h"
#include "VisualScript/Nodes/VisualScriptGetVarNode.h"
#include "VisualScript/Nodes/VisualScriptSetMemberNode.h"
#include "VisualScript/Nodes/VisualScriptSetVarNode.h"
#include "VisualScript/Nodes/VisualScriptWhileNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include "VisualScript/VisualScriptNode.h"

#include <Math/Color.h>

#include <iomanip>
#include <algorithm>
#include <cstring>

namespace DAVA
{
/*
     + Simple interpreter of code
     - Complex interpreter with ability to debug
     - Ability to generate C++ code from graph
 */

/*
     Example of generated code
 
 
     int ExecutionNode_X()
     {
         int arg0 = sum(data.intValue0, data.intValue1);
         bool condition
         if (
 
     }
 */

using PrinterFn = void (*)(std::ostringstream&, const Any&);
using PrintersTable = Map<const Type*, PrinterFn>;

const PrintersTable valuePrinters =
{
  { Type::Instance<int32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int32>(); } },
  { Type::Instance<uint32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint32>(); } },
  { Type::Instance<int64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int64>(); } },
  { Type::Instance<uint64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint64>(); } },
  { Type::Instance<float32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float32>(); } },
  { Type::Instance<float64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float64>(); } },
  { Type::Instance<String>(), [](std::ostringstream& out, const Any& any) { out << any.Get<String>().c_str(); } },
  { Type::Instance<FastName>(), [](std::ostringstream& out, const Any& any) { out << any.Get<FastName>().c_str(); } },
  { Type::Instance<size_t>(), [](std::ostringstream& out, const Any& any) { out << any.Get<size_t>(); } },
  { Type::Instance<Vector2>(), [](std::ostringstream& out, const Any& any) { Vector2 v = any.Get<Vector2>(); out << Format("Vector2(%.3f, %.3f)", v.x, v.y); } },
  { Type::Instance<bool>(), [](std::ostringstream& out, const Any& any) { out << any.Get<bool>(); } },
  { Type::Instance<Color>(), [](std::ostringstream& out, const Any& any) { Color c = any.Get<Color>();  out  << Format("Color(%.3f, %.3f, %.3f, %.3f)", c.r, c.g, c.b, c.a); } },
  { Type::Instance<void>(), [](std::ostringstream& out, const Any& any) { out << "???"; } }
};

const PrinterFn pointerPrinter = [](std::ostringstream& out, const Any& any) { out << "0x" << std::setw(8) << std::setfill('0') << std::hex << any.Cast<void*>(); };

static std::pair<PrinterFn, PrinterFn> GetPrinterFns(const Type* type)
{
    std::pair<PrinterFn, PrinterFn> ret = { nullptr, nullptr };

    if (nullptr != type)
    {
        const PrintersTable* pt = &valuePrinters;
        const Type* keyType = type;
        if (type->IsPointer())
        {
            ret.first = pointerPrinter;
            return ret;
        }

        if (nullptr != type->Decay())
            type = type->Decay();

        auto it = pt->find(type);
        if (it != pt->end())
        {
            ret.first = it->second;
        }

        ret.second = pt->at(Type::Instance<void>());
    }
    else
    {
        ret.second = [](std::ostringstream& out, const Any&)
        {
            out << "__null__";
        };
    }

    return ret;
}

String VisualScriptExecutor::DumpAny(Any& any)
{
    std::ostringstream line;
    std::pair<PrinterFn, PrinterFn> fns = GetPrinterFns(any.GetType());
    if (fns.first != nullptr)
    {
        (*fns.first)(line, any);
    }
    else
    {
        (*fns.second)(line, any);
    }
    return line.str();
}

void VisualScriptExecutor::ExecuteGetVarNode(VisualScriptGetVarNode* node, VisualScriptPin* entryPin)
{
    Any result = node->GetReflection().GetValue();
    node->GetDataOutputPin(0)->SetValue(result);
    Logger::Debug("Get Var: %s => %s", node->GetVarPath().c_str(), DumpAny(result).c_str());
}

void VisualScriptExecutor::ExecuteSetVarNode(VisualScriptSetVarNode* node, VisualScriptPin* entryPin)
{
    if (entryPin == node->GetExecInputPin(0))
    {
        Any setValue = node->GetDataInputPin(0)->GetValue();
        node->GetReflection().SetValueWithCast(setValue);
        Logger::Debug("Set Var: %s <= %s", node->GetVarPath().c_str(), DumpAny(setValue).c_str());
    }

    Any result = node->GetReflection().GetValue();
    node->GetDataOutputPin(0)->SetValue(result);
    Logger::Debug("Get Var: %s => %s", node->GetVarPath().c_str(), DumpAny(result).c_str());

    if (node->GetExecOutputPins().size() == 1)
    {
        VisualScriptPin* execInPin = node->GetExecOutputPins()[0]->GetConnectedTo();
        if (execInPin)
            PushInstruction(execInPin);
    }
}

void VisualScriptExecutor::ExecuteGetMemberNode(VisualScriptGetMemberNode* node, VisualScriptPin* entryPin)
{
    const ValueWrapper* vw = node->GetValueWrapper();
    if (!vw)
        return;

    Any object = node->GetDataInputPin(0)->GetValue();
    DVASSERT(object.GetType());
    if (object.GetType()->IsPointer())
    {
        const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType()->Deref());
        if (objType)
        {
            ReflectedObject refObj(object.Cast<void*>(), objType);

            Any result = vw->GetValue(refObj);
            node->GetDataOutputPin(0)->SetValue(result);
            Logger::Debug("Get Member: %s::%s => %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(result).c_str());
        }
    }
    else
    {
        const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType());
        if (objType)
        {
            ReflectedObject refObj(const_cast<void*>(object.GetData()), objType);

            Any result = vw->GetValue(refObj);
            node->GetDataOutputPin(0)->SetValue(result);
            Logger::Debug("Get Member: %s::%s => %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(result).c_str());
        }
    }
}

void VisualScriptExecutor::ExecuteSetMemberNode(VisualScriptSetMemberNode* node, VisualScriptPin* entryPin)
{
    const ValueWrapper* vw = node->GetValueWrapper();
    if (!vw)
        return;

    if (entryPin == node->GetExecInputPin(0))
    {
        Any object = node->GetDataInputPin(0)->GetValue();
        if (object.GetType()->IsPointer())
        {
            const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType()->Deref());
            if (objType)
            {
                ReflectedObject refObj(object.Cast<void*>(), objType);

                Any value = node->GetDataInputPin(1)->GetValue();
                vw->SetValueWithCast(refObj, value);
                Logger::Debug("Set Member: %s::%s <= %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(value).c_str());
            }
        }
        else
        {
            const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType());
            if (objType)
            {
                ReflectedObject refObj(const_cast<void*>(object.GetData()), objType);

                Any value = node->GetDataInputPin(1)->GetValue();
                vw->SetValueWithCast(refObj, value);
                Logger::Debug("Set Member: %s::%s <= %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(value).c_str());
            }
        }
    }

    Any object = node->GetDataInputPin(0)->GetValue();
    if (object.GetType()->IsPointer())
    {
        const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType()->Deref());
        if (objType)
        {
            ReflectedObject refObj(object.Cast<void*>(), objType);

            Any result = vw->GetValue(refObj);
            node->GetDataOutputPin(0)->SetValue(result);
            Logger::Debug("Set Member: %s::%s => %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(result).c_str());
        }
    }
    else
    {
        const ReflectedType* objType = ReflectedTypeDB::GetByType(object.GetType());
        if (objType)
        {
            ReflectedObject refObj(const_cast<void*>(object.GetData()), objType);

            Any result = vw->GetValue(refObj);
            node->GetDataOutputPin(0)->SetValue(result);
            Logger::Debug("Set Member: %s::%s => %s", node->GetClassName().c_str(), node->GetFieldName().c_str(), DumpAny(result).c_str());
        }
    }

    if (node->GetExecOutputPins().size() == 1)
    {
        VisualScriptPin* execInPin = node->GetExecOutputPins()[0]->GetConnectedTo();
        if (execInPin)
        {
            PushInstruction(execInPin);
        }
    }
}

void VisualScriptExecutor::ExecuteFunctionNode(VisualScriptFunctionNode* node, VisualScriptPin* entryPin)
{
    const AnyFn& function = node->GetFunction();
    if (!function.IsValid())
        return;

    Vector<Any> params;
    auto& inputPins = node->GetDataInputPins();
    size_t size = inputPins.size();
    for (size_t pinIndex = 0; pinIndex < size; ++pinIndex)
    {
        VisualScriptPin* pin = inputPins[pinIndex];
        if (!pin)
        {
            DAVA_THROW(Exception, Format("Pin #%lld is nullptr", pinIndex));
        }

        VisualScriptPin* connectedTo = pin->GetConnectedTo();
        if (connectedTo)
        {
            VisualScriptNode* owner = connectedTo->GetExecutionOwner();
            DVASSERT(owner != nullptr);
        }
        else if (!pin->HasDefaultValue())
        {
            DAVA_THROW(Exception, Format("Pin #%lld is not connected", pinIndex));
        }

        Any value = pin->GetValue();
        params.emplace_back(value);
    }

    // TODO: Support cast check between Type* and Type* or Any and Type*
    //    const Vector<const Type*>& types = function.GetInvokeParams().argsType;
    //    for (size_t ti = 0; ti < size; ++ti)
    //    {
    //        if (types[ti] != params[ti].GetType())
    //        {
    //            DAVA_THROW(Exception, Format("Incorrect param type at #%lld position", ti));
    //        }
    //    }

    // Debug
    String strParams;
    for (size_t ti = 0; ti < size; ++ti)
    {
        strParams += DumpAny(params[ti]);
        if (ti != size - 1)
            strParams += ",";
    }

    Any result;
    if (size == 0)
        result = function.Invoke();
    else if (size == 1)
        result = function.InvokeWithCast(params[0]);
    else if (size == 2)
        result = function.InvokeWithCast(params[0], params[1]);
    else if (size == 3)
        result = function.InvokeWithCast(params[0], params[1], params[2]);
    else if (size == 4)
        result = function.InvokeWithCast(params[0], params[1], params[2], params[3]);

    // Has return types
    if (node->GetDataOutputPins().size() >= 1)
    {
        node->GetDataOutputPin(0)->SetValue(result);
    }

    Logger::Debug("%s = %s::%s(%s)", DumpAny(result).c_str(), node->GetClassName().c_str(), node->GetFunctionName().c_str(), strParams.c_str());

    if (node->GetExecOutputPins().size() == 1)
    {
        VisualScriptPin* execInPin = node->GetExecOutputPins()[0]->GetConnectedTo();
        if (execInPin)
            PushInstruction(execInPin);
    }
}

void VisualScriptExecutor::ExecuteBranchNode(VisualScriptBranchNode* node, VisualScriptPin* entryPin)
{
    Any conditionAny = node->conditionIn->GetValue(); // TODO: change 0 to connected index
    bool condition = conditionAny.Get<bool>();
    if (condition)
    {
        if (node->execOutTrue->GetConnectedTo())
            PushInstruction(node->execOutTrue->GetConnectedTo());
    }
    else
    {
        if (node->execOutFalse->GetConnectedTo())
            PushInstruction(node->execOutFalse->GetConnectedTo());
    }
}

void VisualScriptExecutor::ExecuteForNode(VisualScriptForNode* node, VisualScriptPin* entryPin)
{
    if (entryPin == node->execPin)
    {
        Any firstIndexVal = node->firstIndexPin->GetValue();
        node->index = firstIndexVal.Get<int32>();
        PushInstruction(node->cyclePin); // return to cycle pin after body execution.
        PushInstruction(node->loopBodyPin->GetConnectedTo());
        node->index++;

        Logger::Debug("index: %d ", node->index);
        node->indexPin->SetValue(Any(node->index));
    }
    else if (entryPin == node->cyclePin)
    {
        node->index++;
        int32 lastIndex = node->lastIndexPin->GetValue().Get<int32>();
        if (node->index == lastIndex + 1)
        {
            VisualScriptPin* loopCompletedExecIn = node->loopCompletedPin->GetConnectedTo();
            if (loopCompletedExecIn != nullptr)
                PushInstruction(loopCompletedExecIn);

            Logger::Debug("completed: %d ", node->index);
        }
        else
        {
            Logger::Debug("cycle: %d ", node->index);

            PushInstruction(node->cyclePin); // return to cycle pin after body execution
            PushInstruction(node->loopBodyPin->GetConnectedTo());
            node->indexPin->SetValue(Any(node->index));
        }
    }
    else if (entryPin == node->breakPin)
    {
        while (instructionStack.size() > 0)
        {
            auto pin = instructionStack.top();
            instructionStack.pop();
            if (pin == node->cyclePin || pin == node->loopCompletedPin)
                break;
        };
    }
}

void VisualScriptExecutor::ExecuteWhileNode(VisualScriptWhileNode* node, VisualScriptPin* entryPin)
{
    if (entryPin == node->execPin || entryPin == node->cyclePin)
    {
        Any conditionAny = node->conditionPin->GetValue(); // TODO: change 0 to connected index
        bool condition = conditionAny.Get<bool>();

        if (condition)
        {
            VisualScriptPin* loopBodyExecIn = node->loopBodyPin->GetConnectedTo();
            PushInstruction(node->cyclePin);
            if (loopBodyExecIn != nullptr)
                PushInstruction(loopBodyExecIn);
        }
        else
        {
            VisualScriptPin* loopCompletedExecIn = node->loopCompletedPin->GetConnectedTo();
            if (loopCompletedExecIn != nullptr)
                PushInstruction(loopCompletedExecIn);
        }
    }
}

void VisualScriptExecutor::ExecuteDoNNode(VisualScriptNode* node, VisualScriptPin* entryPin)
{
}

void VisualScriptExecutor::ExecuteEventNode(VisualScriptEventNode* node, VisualScriptPin* entryPin)
{
    PushInstruction(node->GetExecOutputPins()[0]->GetConnectedTo());
}

void VisualScriptExecutor::ExecuteAnotherScriptNode(VisualScriptAnotherScriptNode* node, VisualScriptPin* entryPin)
{
}

void VisualScriptExecutor::CompileNode(VisualScriptNode* node)
{
    std::queue<std::pair<VisualScriptNode*, uint32>> queue;
    Vector<VisualScriptNode*> executionOrder;

    queue.push(std::make_pair(node, 0));
    while (!queue.empty())
    {
        std::pair<VisualScriptNode*, uint32> top = queue.front();
        VisualScriptNode* currentNode = top.first;
        queue.pop();
        executionOrder.push_back(currentNode);

        auto& inputPins = currentNode->GetDataInputPins();
        size_t size = inputPins.size();
        for (size_t pinIndex = 0; pinIndex < size; ++pinIndex)
        {
            VisualScriptPin* pin = inputPins[pinIndex];
            if (!pin)
            {
                // throw exception
                DAVA_THROW(DAVA::Exception, "Pin is not initialized");
            }

            VisualScriptPin* connectedTo = pin->GetConnectedTo();
            if (connectedTo)
            {
                VisualScriptNode* owner = connectedTo->GetExecutionOwner();
                if (owner)
                {
                    VisualScriptNode::eType type = owner->GetType();

                    /*
                     Types that support back-propagation of calls.
                     Only get-vars & functions that do not modify internal state
                     */

                    if (type == VisualScriptNode::GET_VAR)
                    {
                        queue.push(std::make_pair(owner, top.second + 1));
                    }
                    else if (type == VisualScriptNode::GET_MEMBER)
                    {
                        queue.push(std::make_pair(owner, top.second + 1));
                    }
                    else if (type == VisualScriptNode::FUNCTION)
                    {
                        if (owner->GetExecInputPins().size() == 0)
                        {
                            queue.push(std::make_pair(owner, top.second + 1));
                        }
                    }
                }
            }
            else
            {
                if (pin->IsInputDataRequired())
                    DAVA_THROW(DAVA::Exception, Format("Pin %s is not connected", pin->GetName().c_str()));
            }
        }
    }

    std::reverse(std::begin(executionOrder), std::end(executionOrder));
    node->compiledExecOrder = executionOrder;
}

void VisualScriptExecutor::ExecuteSubTree(VisualScriptNode* node, VisualScriptPin* entryPin)
{
    const Vector<VisualScriptNode*>& executionOrder = node->compiledExecOrder;
    DVASSERT(executionOrder.size() != 0);
    size_t size = executionOrder.size();
    DVASSERT(size != 0);

    for (size_t i = 0; i < size; ++i)
    {
        VisualScriptPin* passPin = (executionOrder[i] == node) ? (entryPin) : (nullptr);

        switch (executionOrder[i]->GetType())
        {
        case VisualScriptNode::GET_VAR:
            ExecuteGetVarNode(static_cast<VisualScriptGetVarNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::SET_VAR:
            ExecuteSetVarNode(static_cast<VisualScriptSetVarNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::FUNCTION:
            ExecuteFunctionNode(static_cast<VisualScriptFunctionNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::BRANCH:
            ExecuteBranchNode(static_cast<VisualScriptBranchNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::FOR:
            ExecuteForNode(static_cast<VisualScriptForNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::WHILE:
            ExecuteWhileNode(static_cast<VisualScriptWhileNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::EVENT:
            ExecuteEventNode(static_cast<VisualScriptEventNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::GET_MEMBER:
            ExecuteGetMemberNode(static_cast<VisualScriptGetMemberNode*>(executionOrder[i]), passPin);
            break;
        case VisualScriptNode::SET_MEMBER:
            ExecuteSetMemberNode(static_cast<VisualScriptSetMemberNode*>(executionOrder[i]), passPin);
            break;
        default:
            DAVA_THROW(Exception, Format("Untyped node: %d", executionOrder[i]->GetType()));
        }
    }
}

void VisualScriptExecutor::Compile(VisualScriptPin* entryPin)
{
    DVASSERT(instructionStack.empty() == true);
    PushInstruction(entryPin);
    while (instructionStack.size() > 0)
    {
        VisualScriptPin* activationPin = instructionStack.top();
        instructionStack.pop();
        DVASSERT(activationPin->GetAttribute() == VisualScriptPin::EXEC_IN);
        VisualScriptNode* currentNode = activationPin->GetExecutionOwner();
        CompileNode(currentNode);

        for (VisualScriptPin* execOutPin : currentNode->GetExecOutputPins())
        {
            if (execOutPin != nullptr)
            {
                VisualScriptPin* nextNodeExecInputPin = execOutPin->GetConnectedTo();
                if (nextNodeExecInputPin)
                    PushInstruction(nextNodeExecInputPin);
            }
        }
    }
}

void VisualScriptExecutor::PushInstruction(VisualScriptPin* activationPin)
{
    DVASSERT(activationPin != nullptr);
    DVASSERT(activationPin->GetAttribute() == VisualScriptPin::EXEC_IN);
    instructionStack.push(activationPin);
}

void VisualScriptExecutor::Execute(VisualScriptPin* entryPin)
{
    DVASSERT(instructionStack.empty() == true);
    PushInstruction(entryPin);
    while (instructionStack.size() > 0)
    {
        VisualScriptPin* activationPin = instructionStack.top();
        instructionStack.pop();
        DVASSERT(activationPin->GetAttribute() == VisualScriptPin::EXEC_IN);
        VisualScriptNode* currentNode = activationPin->GetExecutionOwner();
        ExecuteSubTree(currentNode, activationPin);
    }
}
}
