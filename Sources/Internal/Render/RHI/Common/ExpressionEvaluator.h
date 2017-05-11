#pragma once

#include "base/BaseTypes.h"
#include <vector>
#include <unordered_map>

using DAVA::uint32;
using DAVA::float32;

class
ExpressionEvaluator
{
public:
    ExpressionEvaluator();
    ~ExpressionEvaluator();

    bool evaluate(const char* expression, float32* out);

    bool set_variable(const char* var, float32 value);
    void remove_variable(const char* var);
    void clear_variables();

    bool has_variable(const char* name) const;

    // returns
    // true, if there was error and fills provided buffer with error message
    // false, when no error occured (err_buffer is not changed)
    bool get_last_error(char* err_buffer, uint32 err_buffer_size);

    typedef float32 (*FuncImpl)(float32 arg);

    static bool RegisterFunction(const char* name, FuncImpl impl);
    static void RegisterCommonFunctions();

private:
    struct SyntaxTreeNode;

    void _reset();
    void _PopConnectPush();
    bool _Evaluate(const SyntaxTreeNode* node, float32* out, uint32* err_code, uint32* err_index);

    char* _expression;
    std::vector<SyntaxTreeNode> _operator_stack;
    std::vector<uint32> _node_stack;
    std::vector<SyntaxTreeNode> _node;

    std::unordered_map<uint32, float32> _var;

    static std::unordered_map<uint32, FuncImpl> _FuncImpl;

    mutable uint32 _last_error_code;
    mutable uint32 _last_error_index;

    static uint32 _Priority(char operation);

    static const char* _Operators;
    static const char _OpEqual;
    static const char _OpNotEqual;
    static const char _OpLogicalAnd;
    static const char _OpLogicalOr;
    static const char _OpLogicalNot;
    static const char _OpFunctionCall;
    static const char _OpDefined;
    static const char _OpNotDefined;
};