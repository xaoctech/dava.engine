#include "ExpressionEvaluator.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Base/Hash.h"

#include <string.h>
#include <string.h>
#include <math.h>

using DAVA::InvalidIndex;
using DAVA::float32;

//------------------------------------------------------------------------------

const char* ExpressionEvaluator::Operators = "+-*/^!\x01\x02\x03\x04\x06\x07";
const char ExpressionEvaluator::OpEqual = '\x01';
const char ExpressionEvaluator::OpNotEqual = '\x02';
const char ExpressionEvaluator::OpLogicalAnd = '\x03';
const char ExpressionEvaluator::OpLogicalOr = '\x04';
const char ExpressionEvaluator::OpLogicalNot = '\x08';
const char ExpressionEvaluator::OpFunctionCall = '\x05';
const char ExpressionEvaluator::OpDefined = '\x06';
const char ExpressionEvaluator::OpNotDefined = '\x07';

static const float32 Epsilon = 0.000001f;

static const char* ExprEvalError[] =
{
  "", "one of operands is missed", "unmatched parenthesis", "unknown symbol"
};

struct
ExpressionEvaluator::SyntaxTreeNode
{
    float32 operand;

    uint32 left_i;
    uint32 right_i;

    uint32 expr_index; // for error reporting

    char operation;
    FuncImpl func = nullptr;

    SyntaxTreeNode()
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(0)
        , operation(0){};
    SyntaxTreeNode(float32 number, uint32 index)
        : operand(number)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(0){};
    SyntaxTreeNode(char op, uint32 index)
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(op){};
    SyntaxTreeNode(char op, FuncImpl f, uint32 index)
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(op)
        , func(f){};
};

//------------------------------------------------------------------------------

ExpressionEvaluator::ExpressionEvaluator()
    : expressionText(nullptr)
{
    Reset();
}

//------------------------------------------------------------------------------

ExpressionEvaluator::~ExpressionEvaluator()
{
}

//------------------------------------------------------------------------------

void
ExpressionEvaluator::Reset()
{
    operatorStack.clear();
    nodeStack.clear();
    nodeArray.clear();

    if (expressionText)
    {
        ::free(expressionText);
    }

    lastErrorCode = 0;
    lastErrorIndex = 0;
}

//------------------------------------------------------------------------------

void
ExpressionEvaluator::PopConnectPush()
{
    operatorStack.back().right_i = nodeStack.back();
    nodeStack.pop_back();
    if (operatorStack.back().operation != OpDefined && operatorStack.back().operation != OpNotDefined)
    {
        operatorStack.back().left_i = nodeStack.back();
        nodeStack.pop_back();
    }

    nodeStack.push_back(uint32(nodeArray.size()));
    nodeArray.push_back(operatorStack.back());
    operatorStack.pop_back();
}

//------------------------------------------------------------------------------

bool
ExpressionEvaluator::EvaluateInternal(const SyntaxTreeNode* node, float32* out, uint32* err_code, uint32* err_index)
{
    DVASSERT(out);
    *out = 0;

    if (node)
    {
        if (node->operation)
        {
            if (((node->operation == OpFunctionCall || node->operation == OpLogicalNot || node->operation == OpDefined || node->operation == OpNotDefined) && node->right_i != InvalidIndex) // for funcs only right arg makes sense
                || (node->left_i != InvalidIndex && node->right_i != InvalidIndex) // for normal ops - binary operator is assumed
                )
            {
                if (node->operation == OpDefined)
                {
                    float32 val;

                    if (EvaluateInternal((&nodeArray[0]) + node->right_i, &val, err_code, err_index))
                    {
                        *out = (val == 0.0f) ? 0.0f : 1.0f;
                    }
                    else
                    {
                        return false; // error code and index already filled
                    }
                }
                else if (node->operation == OpNotDefined)
                {
                    float32 val;

                    if (EvaluateInternal((&nodeArray[0]) + node->right_i, &val, err_code, err_index))
                    {
                        *out = (val == 0.0f) ? 0.0f : 1.0f;
                    }
                    else
                    {
                        return false; // error code and index already filled
                    }
                }
                else
                {
                    float32 x, y;

                    if ((node->left_i == InvalidIndex || EvaluateInternal((&nodeArray[0]) + node->left_i, &x, err_code, err_index))
                        && EvaluateInternal((&nodeArray[0]) + node->right_i, &y, err_code, err_index)
                        )
                    {
                        switch (node->operation)
                        {
                        case '+':
                            *out = x + y;
                            break;
                        case '-':
                            *out = x - y;
                            break;
                        case '*':
                            *out = x * y;
                            break;
                        case '/':
                            *out = x / y;
                            break;

                        case '^':
                            *out = pow(x, y);
                            break;

                        case OpEqual:
                            *out = fabs(x - y) < Epsilon ? 1.0f : 0.0f;
                            break;

                        case OpNotEqual:
                            *out = fabs(x - y) < Epsilon ? 0.0f : 1.0f;
                            break;

                        case OpLogicalAnd:
                            *out = (fabs(x) > Epsilon && fabs(y) > Epsilon) ? 1.0f : 0.0f;
                            break;

                        case OpLogicalOr:
                            *out = (fabs(x) > Epsilon || fabs(y) > Epsilon) ? 1.0f : 0.0f;
                            break;

                        case OpLogicalNot:
                            *out = (fabs(y) > Epsilon) ? 0.0f : 1.0f;
                            break;

                        case OpFunctionCall:
                            *out = (node->func)(y);
                            break;
                        }
                    }
                    else
                    {
                        return false; // error code and index already filled
                    }
                }
            }
            else
            {
                // not enough operands
                *err_code = 1;
                *err_index = node->expr_index;
                return false;
            }
        }
        else
        {
            *out = node->operand;
        }
    }

    return true;
}

//------------------------------------------------------------------------------

static inline uint32
_GetOperand(const char* expression, float32* operand)
{
    uint32 ret = 0;
    const char* expr = expression;

    while (*expr && (isdigit(*expr) || (*expr == '.')))
    {
        ++expr;
        ++ret;
    }

    if (operand)
        *operand = std::stof(expression);

    return ret;
}

//------------------------------------------------------------------------------

static inline uint32
_GetVariable(const char* expression)
{
    uint32 ret = 0;
    const char* expr = expression;

    while (*expr && (isalnum(*expr) || *expr == '_'))
    {
        ++expr;
        ++ret;
    }

    return ret;
}

//------------------------------------------------------------------------------

uint32
ExpressionEvaluator::OperationPriority(char operation)
{
    uint32 ret = 0;

    switch (operation)
    {
    case '!':
        ret += 2;
    case OpDefined:
    case OpNotDefined:
        ret += 3;
    case OpLogicalNot:
        ret += 3;
    case '^':
        ++ret;
    case '*':
    case '/':
        ++ret;
    case '+':
    case '-':
        ++ret;
    case OpEqual:
    case OpNotEqual:
    case OpLogicalAnd:
    case OpLogicalOr:
        ++ret;
    case OpFunctionCall:
        ++ret;
    }

    return ret;
}

//------------------------------------------------------------------------------

bool
ExpressionEvaluator::Evaluate(const char* expression, float32* result)
{
    uint32 len = uint32(strlen(expression));
    char* text = (char*)(::malloc(len + 1));

    DVASSERT(result);
    DVASSERT(len > 0);

    const char* s = expression;
    char* d = text;

    while (*s && *s != '\n' && *s != '\r')
    {
        if (*s == '=' && *(s + 1) == '=')
        {
            *d++ = OpEqual;
            s += 2;
        }
        else if (*s == '!' && *(s + 1) == '=')
        {
            *d++ = OpNotEqual;
            s += 2;
        }
        else if (*s == '&' && *(s + 1) == '&')
        {
            *d++ = OpLogicalAnd;
            s += 2;
        }
        else if (*s == '|' && *(s + 1) == '|')
        {
            *d++ = OpLogicalOr;
            s += 2;
        }
        else if (strnicmp(s, "!defined", 8) == 0)
        {
            *d++ = OpNotDefined;
            s += 8 + 1;
        }
        else if (strnicmp(s, "defined", 7) == 0)
        {
            *d++ = OpDefined;
            s += 7 + 1;
        }
        else if (*s == '!')
        {
            const char* ns1 = s + 1;
            while (*ns1 && (*ns1 == ' ' || *ns1 == '\t'))
                ++ns1;

            if (*ns1 == '(')
            {
                *d++ = OpLogicalNot;
                s += 1;
            }
            else
            {
                *d++ = *s++;
            }
        }
        else
        {
            *d++ = *s++;
        }
    }
    *d = '\0';

    Reset();
    expressionText = text;

    // build expr.tree

    const char* expr = text; // expression;
    char var[1024] = "";
    bool last_token_operand = false;
    bool negate_operand_value = false;
    bool invert_operand_value = false;

    while (*expr)
    {
        uint32 offset = 0;

        // skip spaces
        if (isspace(*expr))
        {
            while (isspace(*(expr + offset)))
                ++offset;
        }
        // process operands
        else if (isdigit(*expr))
        {
            SyntaxTreeNode node;
            offset = _GetOperand(expr, &node.operand);
            node.expr_index = uint32(expr - text);

            if (negate_operand_value)
                node.operand = -node.operand;
            if (invert_operand_value)
                node.operand = (fabs(node.operand) > Epsilon) ? 0.0f : 1.0f;

            nodeStack.push_back(uint32(nodeArray.size()));
            nodeArray.push_back(node);

            last_token_operand = true;
            negate_operand_value = false;
            invert_operand_value = false;
        }
        // process variables/functions
        else if (isalpha(*expr))
        {
            offset = _GetVariable(expr);

            strncpy(&var[0], expr, offset);
            var[offset] = '\0';
            uint32 vhash = DAVA::HashValue_N(var, offset);
            std::unordered_map<uint32_t, FuncImpl>::iterator func = FuncImplMap.find(vhash);

            if (func != FuncImplMap.end())
            {
                operatorStack.push_back(SyntaxTreeNode(OpFunctionCall, func->second, expr - text));
                last_token_operand = false;
            }
            else
            {
                if (varMap.find(vhash) != varMap.end())
                {
                    float32 value = varMap[vhash];
                    if (negate_operand_value)
                        value = -value;
                    if (invert_operand_value)
                        value = (fabs(value) > Epsilon) ? 0.0f : 1.0f;

                    if (operatorStack.size() && operatorStack.back().operation == OpDefined)
                        value = 1.0f;
                    if (operatorStack.size() && operatorStack.back().operation == OpNotDefined)
                        value = 0.0f;

                    nodeStack.push_back(uint32(nodeArray.size()));
                    nodeArray.push_back(SyntaxTreeNode(value, uint32(expr - text)));

                    last_token_operand = true;
                    negate_operand_value = false;
                    invert_operand_value = false;
                }
                else
                {
                    if (operatorStack.size() && (operatorStack.back().operation == OpDefined || operatorStack.back().operation == OpNotDefined))
                    {
                        nodeStack.push_back(uint32(nodeArray.size()));
                        nodeArray.push_back(SyntaxTreeNode((operatorStack.back().operation == OpDefined) ? 0.0f : 1.0f, InvalidIndex));

                        last_token_operand = true;
                        negate_operand_value = false;
                        invert_operand_value = false;
                    }
                    else
                    {
                        // undefined symbol
                        lastErrorCode = 3;
                        lastErrorIndex = uint32(expr - text);
                        return false;
                    }
                }
            }
        }
        else if (*expr == OpLogicalNot)
        {
            operatorStack.push_back(SyntaxTreeNode(OpLogicalNot, expr - text));
            last_token_operand = false;
            offset = 1;
        }
        // process operators
        else if (strchr(Operators, *expr))
        {
            if (*expr == '-' && !last_token_operand)
            {
                negate_operand_value = true;

                ++expr;
                continue;
            }

            if (*expr == '!' && !last_token_operand)
            {
                invert_operand_value = true;

                ++expr;
                continue;
            }

            if (operatorStack.size() == 0
                || OperationPriority(operatorStack.back().operation) < OperationPriority(*expr)
                )
            {
                operatorStack.push_back(SyntaxTreeNode(*expr, uint32(expr - text)));
            }
            else
            {
                // we need to clear stack from higher priority operators
                // and from the same-priority ( we calculate left part first )
                while (operatorStack.size() != 0
                       && OperationPriority(operatorStack.back().operation) >= OperationPriority(*expr)
                       )
                {
                    if (nodeStack.size() < 2 && operatorStack.back().operation != OpDefined && operatorStack.back().operation != OpNotDefined)
                    {
                        // not enough operands
                        lastErrorCode = 1;
                        lastErrorIndex = operatorStack.back().expr_index;
                        return false;
                    }
                    PopConnectPush();
                }

                operatorStack.push_back(SyntaxTreeNode(*expr, uint32(expr - text)));
            }

            last_token_operand = false;

            offset = 1;
        }
        // process parenthesis
        else if (*expr == '(')
        {
            operatorStack.push_back(SyntaxTreeNode(*expr, uint32(expr - text)));
            offset = 1;

            last_token_operand = false;
        }
        else if (*expr == ')')
        {
            // find open parentesis
            while ((operatorStack.size() != 0) && (operatorStack.back().operation != '('))
            {
                if (nodeStack.size() < 2)
                {
                    // not enough operands
                    lastErrorCode = 1;
                    lastErrorIndex = operatorStack.back().expr_index;
                    return false;
                }
                PopConnectPush();
            }

            // check that stack  didn't run out
            if (operatorStack.size() == 0)
            {
                //parenthesis are unbalanced
                lastErrorCode = 2;
                lastErrorIndex = uint32(expr - text);
                return false;
            }

            operatorStack.pop_back();

            // check if it was logical-NOT in form !(some-expression)
            if ((operatorStack.size() != 0) && (operatorStack.back().operation == OpLogicalNot))
            {
                operatorStack.back().right_i = nodeStack.back();
                nodeStack.pop_back();

                nodeStack.push_back(nodeArray.size());
                nodeArray.push_back(operatorStack.back());

                operatorStack.pop_back();
            }

            // check if it was function call
            if ((operatorStack.size() != 0) && (operatorStack.back().operation == OpFunctionCall))
            {
                operatorStack.back().right_i = nodeStack.back();
                nodeStack.pop_back();

                nodeStack.push_back(nodeArray.size());
                nodeArray.push_back(operatorStack.back());

                operatorStack.pop_back();
            }

            last_token_operand = false;

            offset = 1;
        }
        else
        {
            offset = 1; // found nothing, increment and try again
        }

        expr += offset;
    }

    while (operatorStack.size() != 0)
    {
        if (operatorStack.back().operation == '(')
        {
            //parenthesis are unbalanced
            lastErrorCode = 2;
            lastErrorIndex = operatorStack.back().expr_index;
            return false;
        }
        else if (nodeStack.size() < 2 && operatorStack.back().operation != OpDefined && operatorStack.back().operation != OpNotDefined)
        {
            // not enough operands
            lastErrorCode = 1;
            lastErrorIndex = operatorStack.back().expr_index;
            return false;
        }

        PopConnectPush();
    }

    return EvaluateInternal((&nodeArray[0]) + nodeStack.back(), result, &lastErrorCode, &lastErrorIndex);
}

bool
ExpressionEvaluator::SetVariable(const char* var, float32 value)
{
    bool success = false;
    uint32 var_id = DAVA::HashValue_N(var, uint32(strlen(var)));

    varMap[var_id] = value;

    return success;
}

void
ExpressionEvaluator::RemoveVariable(const char* var)
{
    uint32 var_id = DAVA::HashValue_N(var, uint32(strlen(var)));

    varMap.erase(var_id);
}

bool
ExpressionEvaluator::HasVariable(const char* name) const
{
    bool success = false;
    uint32 var_id = DAVA::HashValue_N(name, uint32(strlen(name)));

    return varMap.find(var_id) != varMap.end();
}

void
ExpressionEvaluator::ClearVariables()
{
    varMap.clear();
}

bool
ExpressionEvaluator::GetLastError(char* err_buffer, uint32 err_buffer_size)
{
    bool ret = false;

    if (lastErrorCode)
    {
        uint32 len = uint32(::strlen(expressionText));
        char buf[2048];

        ::memset(buf, ' ', len);
        buf[len] = '\0';
        buf[lastErrorIndex] = '^';

        Snprintf(err_buffer, err_buffer_size, "%s\n%s\n%s\n", ExprEvalError[lastErrorCode], expressionText, buf);
        ret = true;
    }

    return ret;
}

bool ExpressionEvaluator::RegisterFunction(const char* name, FuncImpl impl)
{
    bool success = false;
    uint32 func_id = DAVA::HashValue_N(name, uint32(strlen(name)));

    if (FuncImplMap.find(func_id) == FuncImplMap.end())
    {
        FuncImplMap[func_id] = impl;
        success = true;
    }

    return success;
}

static float32 EV_Sin(float32 x)
{
    return ::sinf(x);
}
static float32 EV_Cos(float32 x)
{
    return ::cosf(x);
}
static float32 EV_Abs(float32 x)
{
    return ::fabs(x);
}
void ExpressionEvaluator::RegisterCommonFunctions()
{
    ExpressionEvaluator::RegisterFunction("sin", &EV_Sin);
    ExpressionEvaluator::RegisterFunction("cos", &EV_Cos);
    ExpressionEvaluator::RegisterFunction("abs", &EV_Abs);
}

std::unordered_map<uint32, ExpressionEvaluator::FuncImpl> ExpressionEvaluator::FuncImplMap;
