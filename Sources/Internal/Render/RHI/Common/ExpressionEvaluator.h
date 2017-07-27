#pragma once

#include "Render/RHI/Common/Preprocessor/PreprocessorHelpers.h"

namespace DAVA
{
class ExpressionEvaluator
{
public:
    ExpressionEvaluator();
    ~ExpressionEvaluator();

    bool Evaluate(const char* expression, float32* out);

    bool SetVariable(const char* var, float32 value);
    void RemoveVariable(const char* var);
    void ClearVariables();

    bool HasVariable(const char* name) const;

    // returns
    // true, if there was error and fills provided buffer with error message
    // false, when no error occured (err_buffer is not changed)
    bool GetLastError(char* err_buffer, uint32 err_buffer_size);

    typedef float32 (*FuncImpl)(float32 arg);

    static bool RegisterFunction(const char* name, FuncImpl impl);
    static void RegisterCommonFunctions();

private:
    struct SyntaxTreeNode;

    void Reset();
    void PopConnectPush();
    bool EvaluateInternal(const SyntaxTreeNode* node, float32* out, uint32* err_code, uint32* err_index);

    char* expressionText;
    Vector<SyntaxTreeNode> operatorStack;
    Vector<uint32> nodeStack;
    Vector<SyntaxTreeNode> nodeArray;
    UnorderedMap<uint32, float32> varMap;

    static UnorderedMap<uint32, FuncImpl> FuncImplMap;

    mutable uint32 lastErrorCode;
    mutable uint32 lastErrorIndex;
};
}
