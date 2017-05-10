#pragma once

#include "base/BaseTypes.h"
#include <vector>
#include <unordered_map>

using DAVA::uint32;

class
ExpressionEvaluator
{
public:
    ExpressionEvaluator();
    ~ExpressionEvaluator();

    bool evaluate(const char* expression, float* out);

    bool set_variable(const char* var, float value);
    void remove_variable(const char* var);
    void clear_variables();

    bool has_variable(const char* name) const;

    // returns
    // true, if there was error and fills provided buffer with error message
    // false, when no error occured (err_buffer is not changed)
    bool get_last_error(char* err_buffer, unsigned err_buffer_size);

    typedef float (*FuncImpl)(float arg);

    static bool RegisterFunction(const char* name, FuncImpl impl);
    static void RegisterCommonFunctions();

private:
    struct SyntaxTreeNode;

    void _reset();
    void _PopConnectPush();
    bool _Evaluate(const SyntaxTreeNode* node, float* out, unsigned* err_code, unsigned* err_index);

    char* _expression;
    std::vector<SyntaxTreeNode> _operator_stack;
    std::vector<unsigned> _node_stack;
    std::vector<SyntaxTreeNode> _node;

    std::unordered_map<uint32, float> _var;

    static std::unordered_map<uint32, FuncImpl> _FuncImpl;

    mutable unsigned _last_error_code;
    mutable unsigned _last_error_index;

    static unsigned _Priority(char operation);

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