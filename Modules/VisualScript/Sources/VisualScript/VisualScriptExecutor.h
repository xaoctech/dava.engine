#pragma once
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class VisualScriptExecutor
{
public:
    /**
         Execution can be started only in event node, and reflection should be provided to start event.
     */
    void Execute(VisualScriptPin* entryPin);
    void Compile(VisualScriptPin* entryPin);

private:
    void CompileNode(VisualScriptNode* node);

    void ExecuteSubTree(VisualScriptNode* entryPoint, VisualScriptPin* entryPin);

    void ExecuteGetVarNode(VisualScriptGetVarNode* node, VisualScriptPin* entryPin);
    void ExecuteSetVarNode(VisualScriptSetVarNode* node, VisualScriptPin* entryPin);
    void ExecuteFunctionNode(VisualScriptFunctionNode* node, VisualScriptPin* entryPin);
    void ExecuteBranchNode(VisualScriptBranchNode* node, VisualScriptPin* entryPin);
    void ExecuteForNode(VisualScriptForNode* node, VisualScriptPin* entryPin);
    void ExecuteWhileNode(VisualScriptWhileNode* node, VisualScriptPin* entryPin);
    void ExecuteDoNNode(VisualScriptNode* node, VisualScriptPin* entryPin);
    void ExecuteEventNode(VisualScriptEventNode* node, VisualScriptPin* entryPin);
    void ExecuteCustomEventNode(VisualScriptEventNode* node, VisualScriptPin* entryPin);
    void ExecuteAnotherScriptNode(VisualScriptAnotherScriptNode* node, VisualScriptPin* entryPin);
    void ExecuteGetMemberNode(VisualScriptGetMemberNode* node, VisualScriptPin* entryPin);
    void ExecuteSetMemberNode(VisualScriptSetMemberNode* node, VisualScriptPin* entryPin);

    String DumpAny(Any& any);

    void PushInstruction(VisualScriptPin* activationPin);

    Stack<VisualScriptPin*> instructionStack;
    uint32 lastExecutionFrame = 1;
};

class VisualScriptCodeGenerator
{
public:
    void Generate(VisualScriptNode* entryPoint);
};
}
