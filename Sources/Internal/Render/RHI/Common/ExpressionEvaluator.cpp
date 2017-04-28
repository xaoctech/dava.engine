#include "ExpressionEvaluator.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Base/Hash.h"

#include <string.h>
#include <string.h>

using DAVA::InvalidIndex;

//------------------------------------------------------------------------------

const char* ExpressionEvaluator::_Operators = "+-*/^!\x01\x02\x03\x04\x06\x07";
const char ExpressionEvaluator::_OpEqual = '\x01';
const char ExpressionEvaluator::_OpNotEqual = '\x02';
const char ExpressionEvaluator::_OpLogicalAnd = '\x03';
const char ExpressionEvaluator::_OpLogicalOr = '\x04';
const char ExpressionEvaluator::_OpLogicalNot = '\x08';
const char ExpressionEvaluator::_OpFunctionCall = '\x05';
const char ExpressionEvaluator::_OpDefined = '\x06';
const char ExpressionEvaluator::_OpNotDefined = '\x07';

static const float Epsilon = 0.000001f;

static const char* ExprEvalError[] =
{
  "", "one of operands is missed", "unmatched parenthesis", "unknown symbol"
};

struct
ExpressionEvaluator::SyntaxTreeNode
{
    float operand;

    unsigned left_i;
    unsigned right_i;

    unsigned expr_index; // for error reporting

    char operation;

    SyntaxTreeNode()
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(0)
        , operation(0){};
    SyntaxTreeNode(float number, unsigned index)
        : operand(number)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(0){};
    SyntaxTreeNode(char op, unsigned index)
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(op){};
    SyntaxTreeNode(char op, uint8_t func_code, unsigned index)
        : operand(0.0f)
        , left_i(InvalidIndex)
        , right_i(InvalidIndex)
        , expr_index(index)
        , operation(op){};
};

//------------------------------------------------------------------------------

ExpressionEvaluator::ExpressionEvaluator()
    : _expression(nullptr)
{
    _reset();
}

//------------------------------------------------------------------------------

ExpressionEvaluator::~ExpressionEvaluator()
{
}

//------------------------------------------------------------------------------

void
ExpressionEvaluator::_reset()
{
    _operator_stack.clear();
    _node_stack.clear();
    _node.clear();

    if (_expression)
    {
        ::free(_expression);
    }

    _last_error_code = 0;
    _last_error_index = 0;
}

//------------------------------------------------------------------------------

void
ExpressionEvaluator::_PopConnectPush()
{
    _operator_stack.back().right_i = _node_stack.back();
    _node_stack.pop_back();
    if (_operator_stack.back().operation != _OpDefined && _operator_stack.back().operation != _OpNotDefined)
    {
        _operator_stack.back().left_i = _node_stack.back();
        _node_stack.pop_back();
    }

    _node_stack.push_back(unsigned(_node.size()));
    _node.push_back(_operator_stack.back());
    _operator_stack.pop_back();
}

//------------------------------------------------------------------------------

bool
ExpressionEvaluator::_Evaluate(const SyntaxTreeNode* node, float* out, unsigned* err_code, unsigned* err_index)
{
    DVASSERT(out);
    *out = 0;

    if (node)
    {
        if (node->operation)
        {
            if (((node->operation == _OpFunctionCall || node->operation == _OpLogicalNot || node->operation == _OpDefined || node->operation == _OpNotDefined) && node->right_i != InvalidIndex) // for funcs only right arg makes sense
                || (node->left_i != InvalidIndex && node->right_i != InvalidIndex) // for normal ops - binary operator is assumed
                )
            {
                if (node->operation == _OpDefined)
                {
                    float val;

                    if (_Evaluate((&_node[0]) + node->right_i, &val, err_code, err_index))
                    {
                        *out = (val == 0.0f) ? 0.0f : 1.0f;
                    }
                    else
                    {
                        return false; // error code and index already filled
                    }
                }
                else if (node->operation == _OpNotDefined)
                {
                    float val;

                    if (_Evaluate((&_node[0]) + node->right_i, &val, err_code, err_index))
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
                    float x, y;

                    if ((node->left_i == InvalidIndex || _Evaluate((&_node[0]) + node->left_i, &x, err_code, err_index))
                        && _Evaluate((&_node[0]) + node->right_i, &y, err_code, err_index)
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

                        case _OpEqual:
                            *out = fabs(x - y) < Epsilon ? 1.0f : 0.0f;
                            break;

                        case _OpNotEqual:
                            *out = fabs(x - y) < Epsilon ? 0.0f : 1.0f;
                            break;

                        case _OpLogicalAnd:
                            *out = (fabs(x) > Epsilon && fabs(y) > Epsilon) ? 1.0f : 0.0f;
                            break;

                        case _OpLogicalOr:
                            *out = (fabs(x) > Epsilon || fabs(y) > Epsilon) ? 1.0f : 0.0f;
                            break;

                        case _OpLogicalNot:
                            *out = (fabs(y) > Epsilon) ? 0.0f : 1.0f;
                            break;

                            //                        case _OpFunctionCall :
                            //                            *out = func[node->function_code](y);
                            //                            break;
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

static inline unsigned
_GetOperand(const char* expression, float* operand)
{
    unsigned ret = 0;
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

static inline unsigned
_GetVariable(const char* expression)
{
    unsigned ret = 0;
    const char* expr = expression;

    while (*expr && (isalnum(*expr) || *expr == '_'))
    {
        ++expr;
        ++ret;
    }

    return ret;
}

//------------------------------------------------------------------------------

unsigned
ExpressionEvaluator::_Priority(char operation)
{
    unsigned ret = 0;

    switch (operation)
    {
    case '!':
        ret += 2;
    case _OpDefined:
    case _OpNotDefined:
        ret += 3;
    case _OpLogicalNot:
        ret += 3;
    case '^':
        ++ret;
    case '*':
    case '/':
        ++ret;
    case '+':
    case '-':
        ++ret;
    case _OpEqual:
    case _OpNotEqual:
    case _OpLogicalAnd:
    case _OpLogicalOr:
        ++ret;
    case _OpFunctionCall:
        ++ret;
    }

    return ret;
}

//------------------------------------------------------------------------------

bool
ExpressionEvaluator::evaluate(const char* expression, float* result)
{
    unsigned len = unsigned(strlen(expression));
    char* text = (char*)(::malloc(len + 1));

    DVASSERT(result);
    DVASSERT(len > 0);

    const char* s = expression;
    char* d = text;

    while (*s && *s != '\n' && *s != '\r')
    {
        if (*s == '=' && *(s + 1) == '=')
        {
            *d++ = _OpEqual;
            s += 2;
        }
        else if (*s == '!' && *(s + 1) == '=')
        {
            *d++ = _OpNotEqual;
            s += 2;
        }
        else if (*s == '&' && *(s + 1) == '&')
        {
            *d++ = _OpLogicalAnd;
            s += 2;
        }
        else if (*s == '|' && *(s + 1) == '|')
        {
            *d++ = _OpLogicalOr;
            s += 2;
        }
        else if (strnicmp(s, "!defined", 8) == 0)
        {
            *d++ = _OpNotDefined;
            s += 8 + 1;
        }
        else if (strnicmp(s, "defined", 7) == 0)
        {
            *d++ = _OpDefined;
            s += 7 + 1;
        }
        else if (*s == '!')
        {
            const char* ns1 = s + 1;
            while (*ns1 && (*ns1 == ' ' || *ns1 == '\t'))
                ++ns1;

            if (*ns1 == '(')
            {
                *d++ = _OpLogicalNot;
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

    _reset();
    _expression = text;

    // build expr.tree

    const char* expr = text; // expression;
    char var[1024] = "";
    bool last_token_operand = false;
    bool negate_operand_value = false;
    bool invert_operand_value = false;

    while (*expr)
    {
        unsigned offset = 0;

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
            node.expr_index = unsigned(expr - text);

            if (negate_operand_value)
                node.operand = -node.operand;
            if (invert_operand_value)
                node.operand = (fabs(node.operand) > Epsilon) ? 0.0f : 1.0f;

            _node_stack.push_back(unsigned(_node.size()));
            _node.push_back(node);

            last_token_operand = true;
            negate_operand_value = false;
            invert_operand_value = false;
        }
        // process variables
        else if (isalpha(*expr))
        {
            offset = _GetVariable(expr);

            strncpy(&var[0], expr, offset);
            var[offset] = '\0';
            uint32 vhash = DAVA::HashValue_N(var, offset);

            if (_var.find(vhash) != _var.end())
            {
                float value = _var[vhash];
                if (negate_operand_value)
                    value = -value;
                if (invert_operand_value)
                    value = (fabs(value) > Epsilon) ? 0.0f : 1.0f;

                if (_operator_stack.size() && _operator_stack.back().operation == _OpDefined)
                    value = 1.0f;
                if (_operator_stack.size() && _operator_stack.back().operation == _OpNotDefined)
                    value = 0.0f;

                _node_stack.push_back(unsigned(_node.size()));
                _node.push_back(SyntaxTreeNode(value, unsigned(expr - text)));

                last_token_operand = true;
                negate_operand_value = false;
                invert_operand_value = false;
            }
            else
            {
                if (_operator_stack.size() && (_operator_stack.back().operation == _OpDefined || _operator_stack.back().operation == _OpNotDefined))
                {
                    _node_stack.push_back(unsigned(_node.size()));
                    _node.push_back(SyntaxTreeNode((_operator_stack.back().operation == _OpDefined) ? 0.0f : 1.0f, InvalidIndex));

                    last_token_operand = true;
                    negate_operand_value = false;
                    invert_operand_value = false;
                }
                else
                {
                    // undefined symbol
                    _last_error_code = 3;
                    _last_error_index = unsigned(expr - text);
                    return false;
                }
            }
        }
        else if (*expr == _OpLogicalNot)
        {
            _operator_stack.push_back(SyntaxTreeNode(_OpLogicalNot, expr - text));
            last_token_operand = false;
            offset = 1;
        }
        // process operators
        else if (strchr(_Operators, *expr))
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

            if (_operator_stack.size() == 0
                || _Priority(_operator_stack.back().operation) < _Priority(*expr)
                )
            {
                _operator_stack.push_back(SyntaxTreeNode(*expr, unsigned(expr - text)));
            }
            else
            {
                // we need to clear stack from higher priority operators
                // and from the same-priority ( we calculate left part first )
                while (_operator_stack.size() != 0
                       && _Priority(_operator_stack.back().operation) >= _Priority(*expr)
                       )
                {
                    if (_node_stack.size() < 2 && _operator_stack.back().operation != _OpDefined && _operator_stack.back().operation != _OpNotDefined)
                    {
                        // not enough operands
                        _last_error_code = 1;
                        _last_error_index = _operator_stack.back().expr_index;
                        return false;
                    }
                    _PopConnectPush();
                }

                _operator_stack.push_back(SyntaxTreeNode(*expr, unsigned(expr - text)));
            }

            last_token_operand = false;

            offset = 1;
        }
        // process parenthesis
        else if (*expr == '(')
        {
            _operator_stack.push_back(SyntaxTreeNode(*expr, unsigned(expr - text)));
            offset = 1;

            last_token_operand = false;
        }
        else if (*expr == ')')
        {
            // find open parentesis
            while ((_operator_stack.size() != 0) && (_operator_stack.back().operation != '('))
            {
                if (_node_stack.size() < 2)
                {
                    // not enough operands
                    _last_error_code = 1;
                    _last_error_index = _operator_stack.back().expr_index;
                    return false;
                }
                _PopConnectPush();
            }

            // check that stack  didn't run out
            if (_operator_stack.size() == 0)
            {
                //parenthesis are unbalanced
                _last_error_code = 2;
                _last_error_index = unsigned(expr - text);
                return false;
            }

            _operator_stack.pop_back();

            // check if it was logical-NOT in form !(some-expression)
            if ((_operator_stack.size() != 0) && (_operator_stack.back().operation == _OpLogicalNot))
            {
                _operator_stack.back().right_i = _node_stack.back();
                _node_stack.pop_back();

                _node_stack.push_back(_node.size());
                _node.push_back(_operator_stack.back());

                _operator_stack.pop_back();
            }
            /*
            // check if it was function call
            if( (_operator_stack.size() != 0) && (_operator_stack.back().operation == _OpFunctionCall) )
            {
                _operator_stack.back().right_i = _node_stack.back();
                _node_stack.pop_back();

                _node_stack.push_back( _node.size() );
                _node.push_back( _operator_stack.back() );

                _operator_stack.pop_back();
            }
*/
            last_token_operand = false;

            offset = 1;
        }
        else
        {
            offset = 1; // found nothing, increment and try again
        }

        expr += offset;
    }

    while (_operator_stack.size() != 0)
    {
        if (_operator_stack.back().operation == '(')
        {
            //parenthesis are unbalanced
            _last_error_code = 2;
            _last_error_index = _operator_stack.back().expr_index;
            return false;
        }
        else if (_node_stack.size() < 2 && _operator_stack.back().operation != _OpDefined && _operator_stack.back().operation != _OpNotDefined)
        {
            // not enough operands
            _last_error_code = 1;
            _last_error_index = _operator_stack.back().expr_index;
            return false;
        }

        _PopConnectPush();
    }

    return _Evaluate((&_node[0]) + _node_stack.back(), result, &_last_error_code, &_last_error_index);
}

bool
ExpressionEvaluator::set_variable(const char* var, float value)
{
    bool success = false;
    uint32 var_id = DAVA::HashValue_N(var, uint32(strlen(var)));

    _var[var_id] = value;

    return success;
}

void
ExpressionEvaluator::remove_variable(const char* var)
{
    uint32 var_id = DAVA::HashValue_N(var, uint32(strlen(var)));

    _var.erase(var_id);
}

bool
ExpressionEvaluator::has_variable(const char* name) const
{
    bool success = false;
    uint32 var_id = DAVA::HashValue_N(name, uint32(strlen(name)));

    return _var.find(var_id) != _var.end();
}

void
ExpressionEvaluator::clear_variables()
{
    _var.clear();
}

bool
ExpressionEvaluator::get_last_error(char* err_buffer, unsigned err_buffer_size)
{
    bool ret = false;

    if (_last_error_code)
    {
        unsigned len = unsigned(::strlen(_expression));
        char buf[2048];

        ::memset(buf, ' ', len);
        buf[len] = '\0';
        buf[_last_error_index] = '^';

        Snprintf(err_buffer, err_buffer_size, "%s\n%s\n%s\n", ExprEvalError[_last_error_code], _expression, buf);
        ret = true;
    }

    return ret;
}
