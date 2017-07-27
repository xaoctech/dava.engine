#pragma once

#include "Render/RHI/Common/Preprocessor/PreprocessorHelpers.h"

using DAVA::uint32;
using DAVA::float32;

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
    std::vector<SyntaxTreeNode> operatorStack;
    std::vector<uint32> nodeStack;
    std::vector<SyntaxTreeNode> nodeArray;

    std::unordered_map<uint32, float32> varMap;

    static std::unordered_map<uint32, FuncImpl> FuncImplMap;

    mutable uint32 lastErrorCode;
    mutable uint32 lastErrorIndex;
};