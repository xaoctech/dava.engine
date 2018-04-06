#include "sl_Common.h"

#include "sl_GeneratorMSL.h"
#include "sl_Parser.h"
#include "sl_Tree.h"

#include "Logger/Logger.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/Common/rhi_Utils.h"

#define RHI_METAL_DISABLE_SAMPLING 0
#define RHI_METAL_SAMPLING_REPLACEMENT "DefaultSample DISABLE_SAMPLING("

namespace sl
{
class SamplerFetchVisitor : public HLSLTreeVisitor
{
public:
    MSLGenerator::SamplerInfo::FunctionInfo* functionInfo;
    MSLGenerator::SamplerInfo* samplerInfo;

    SamplerFetchVisitor(MSLGenerator::SamplerInfo* sInfo, MSLGenerator::SamplerInfo::FunctionInfo* fInfo)
        : samplerInfo(sInfo)
        , functionInfo(fInfo)
    {
    }

    void VisitFunctionCall(HLSLFunctionCall* node) override
    {
        if (String_Equal(node->function->name, "FramebufferFetch"))
        {
            bool allOk = false;
            if ((node->numArguments == 1) && (node->argument->nodeType == HLSLNodeType_LiteralExpression))
            {
                HLSLLiteralExpression* literalExpression = static_cast<HLSLLiteralExpression*>(node->argument);
                if (literalExpression->type == HLSLBaseType_Int)
                {
                    functionInfo->fbInfo.insert(literalExpression->iValue);
                    allOk = true;
                }
            }
            if (!allOk)
                DAVA::Logger::Error("line %d  - FramebufferFetch badly formatted", node->line);
        }
        else if (String_Equal(node->function->name, "tex2D") || String_Equal(node->function->name, "tex2Dlod") ||
                 String_Equal(node->function->name, "texCUBE") || String_Equal(node->function->name, "texCUBElod") ||
                 String_Equal(node->function->name, "tex2Dcmp"))
        {
            DVASSERT(node->argument->nodeType == HLSLNodeType_IdentifierExpression);
            HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(node->argument);
            DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);

            bool allOk = false;
            for (size_t i = 0, sz = samplerInfo->texInfo.size(); i < sz; ++i)
            {
                if (strcmp(identifier->name, samplerInfo->texInfo[i].name.c_str()) == 0)
                {
                    functionInfo->texInfo.insert(static_cast<DAVA::uint32>(i));
                    allOk = true;
                    break;
                }
            }
            if (!allOk)
                DAVA::Logger::Error("line %d  - unknown sampler reference %s", node->line, identifier->name);
        }
        else
        {
            //here we try to find fetches if calling already parsed functions
            auto it = samplerInfo->samplerMap.find(node->function->name);
            if (it != samplerInfo->samplerMap.end())
            {
                //copy all sampler calls from calling function
                for (DAVA::uint32 t : it->second.texInfo)
                    functionInfo->texInfo.insert(t);
                for (DAVA::uint32 t : it->second.fbInfo)
                    functionInfo->fbInfo.insert(t);
            }
        }

        HLSLTreeVisitor::VisitFunctionCall(node);
    }
};

const char* MSLGenerator::GetTypeName(const HLSLType& type)
{
    switch (type.baseType)
    {
    case HLSLBaseType_Void:
        return "void";
    case HLSLBaseType_Float:
        return "float";
    case HLSLBaseType_Float2:
        return "float2";
    case HLSLBaseType_Float3:
        return "float3";
    case HLSLBaseType_Float4:
        return "float4";
    case HLSLBaseType_Float3x3:
        return "float3x3";
    case HLSLBaseType_Float4x4:
        return "float4x4";
    case HLSLBaseType_Half:
        return "half";
    case HLSLBaseType_Half2:
        return "half2";
    case HLSLBaseType_Half3:
        return "half3";
    case HLSLBaseType_Half4:
        return "half4";
    case HLSLBaseType_Half3x3:
        return "half3x3";
    case HLSLBaseType_Half4x4:
        return "half4x4";
    case HLSLBaseType_Bool:
        return "bool";
    case HLSLBaseType_Int:
        return "int";
    case HLSLBaseType_Int2:
        return "int2";
    case HLSLBaseType_Int3:
        return "int3";
    case HLSLBaseType_Int4:
        return "int4";
    case HLSLBaseType_Uint:
        return "uint";
    case HLSLBaseType_Uint2:
        return "uint2";
    case HLSLBaseType_Uint3:
        return "uint3";
    case HLSLBaseType_Uint4:
        return "uint4";
    case HLSLBaseType_Texture:
        return "texture";
    case HLSLBaseType_Sampler:
        return "sampler";
    case HLSLBaseType_Sampler2D:
        return "sampler2D";
    case HLSLBaseType_Sampler3D:
        return "sampler3D";
    case HLSLBaseType_SamplerCube:
        return "samplerCUBE";
    case HLSLBaseType_Sampler2DShadow:
        return "sampler2DShadow";
    case HLSLBaseType_Sampler2DMS:
        return "sampler2DMS";
    case HLSLBaseType_UserDefined:
        return type.typeName;
    default:
        break; // to shut up goddamn warning
    }
    return "?";
}

MSLGenerator::MSLGenerator(Allocator* allocator)
{
}

bool MSLGenerator::Generate(HLSLTree* tree_, Target target_, const char* entryName_, std::string* code)
{
    tree = tree_;
    entryName = entryName_;
    target = target_;
    isInsideBuffer = false;

    writer.Reset(code);
    samplerInfo.Clear();

    static const char* mtl_define =
    R"(
#include <metal_stdlib>
#include <metal_graphics>
#include <metal_matrix>
#include <metal_geometric>
#include <metal_math>
#include <metal_texture>

using namespace metal;
constexpr sampler shared_compare_sampler(filter::nearest, compare_func::less_equal);
constexpr constant float4 DefaultSample = float4(0.0, 0.0, 1.0, 1.0);
constexpr constant float4 ndcToUvMapping = float4(0.5, -0.5, 0.5, 0.5);
constexpr constant float2 ndcToZMapping = float2(1.0, 0.0);
constexpr constant float2 centerPixelMapping = float2(0.0, 0.0);

#define lerp(a, b, t)           mix((a), (b), (t))
#define frac(a)                 fract(a)
#define mul(a, b)               ((b) * (a))
#define FP_A8(t)                (t).a
#define ddx(f)                  dfdx(f)
#define ddy(f)                  dfdy(f)
#define equal(a, b)             ((a) == (b))
)";

    writer.WriteLine(0, mtl_define);

#if (RHI_METAL_DISABLE_SAMPLING)
    writer.WriteLine(0, "#define DISABLE_SAMPLING(...)");
    writer.WriteLine(0, "#define FP_SHADOW(t) (t).x");
#else
    writer.WriteLine(0, "#define FP_SHADOW(t) (t)");
#endif

    HLSLRoot* root = tree->GetRoot();

    HLSLStatement* statement = root->statement;
    while (statement)
    {
        if (statement->hidden)
        {
            statement = statement->nextStatement;
            continue;
        }
        if (statement->nodeType == HLSLNodeType_Function)
        {
            HLSLFunction* function = static_cast<HLSLFunction*>(statement);
            SamplerFetchVisitor visitor(&samplerInfo, &samplerInfo.samplerMap[function->name]);
            visitor.VisitStatements(function->statement);
        }
        if (statement->nodeType == HLSLNodeType_Declaration)
        {
            HLSLDeclaration* declaration = static_cast<HLSLDeclaration*>(statement);
            bool isSamplerType = IsSamplerType(declaration->type);
            if (isSamplerType)
            {
                SamplerInfo::tex_t tex;
                tex.name = declaration->name;
                tex.type = declaration->type.baseType;
                tex.unit = unsigned(samplerInfo.texInfo.size());
                samplerInfo.texInfo.push_back(tex);
            }
        }
        statement = statement->nextStatement;
    }

    OutputStatements(0, root->statement);

    tree = nullptr;
    return true;
}

const char* MSLGenerator::GetResult() const
{
    return writer.GetResult();
}

void MSLGenerator::OutputExpressionList(HLSLExpression* expression)
{
    int numExpressions = 0;
    while (expression != NULL)
    {
        if (numExpressions > 0)
        {
            writer.Write(", ");
        }
        OutputExpression(expression);
        expression = expression->nextExpression;
        ++numExpressions;
    }
}

void MSLGenerator::OutputExpression(HLSLExpression* expression)
{
    if (expression->nodeType == HLSLNodeType_IdentifierExpression)
    {
        HLSLIdentifierExpression* identifierExpression = static_cast<HLSLIdentifierExpression*>(expression);
        const char* name = identifierExpression->name;

        if (IsSamplerType(identifierExpression->expressionType) && identifierExpression->global)
        {
            if (identifierExpression->expressionType.baseType == HLSLBaseType_Sampler2D || identifierExpression->expressionType.baseType == HLSLBaseType_SamplerCube)
            {
            #if (RHI_METAL_DISABLE_SAMPLING)
                writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
            #else
                writer.Write("%s_texture.sample( %s_sampler ", name, name);
            #endif
            }
        }
        else
        {
            writer.Write("%s", name);
        }
    }
    else if (expression->nodeType == HLSLNodeType_CastingExpression)
    {
        HLSLCastingExpression* castingExpression = static_cast<HLSLCastingExpression*>(expression);
        const char* tname = GetTypeName(castingExpression->type);

        if (strstr(tname, "float") || strstr(tname, "half"))
        {
            OutputDeclaration(castingExpression->type, "");
            writer.Write("(");
            OutputExpression(castingExpression->expression);
            writer.Write(")");
        }
        else
        {
            writer.Write("(");
            OutputDeclaration(castingExpression->type, "");
            writer.Write(")(");
            OutputExpression(castingExpression->expression);
            writer.Write(")");
        }
    }
    else if (expression->nodeType == HLSLNodeType_ConstructorExpression)
    {
        HLSLConstructorExpression* constructorExpression = static_cast<HLSLConstructorExpression*>(expression);
        writer.Write("%s(", GetTypeName(constructorExpression->type));
        OutputExpressionList(constructorExpression->argument);
        writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_LiteralExpression)
    {
        HLSLLiteralExpression* literalExpression = static_cast<HLSLLiteralExpression*>(expression);
        switch (literalExpression->type)
        {
        case HLSLBaseType_Half:
        {
            // Don't use printf directly so that we don't use the system locale.
            char buffer[64] = {};
            String_FormatFloat(buffer, sizeof(buffer), literalExpression->fValue);
            writer.Write("%sh", buffer);
            break;
        }
        case HLSLBaseType_Float:
        {
            // Don't use printf directly so that we don't use the system locale.
            char buffer[64] = {};
            String_FormatFloat(buffer, sizeof(buffer), literalExpression->fValue);
            writer.Write("%sf", buffer);
            break;
        }
        case HLSLBaseType_Int:
            writer.Write("%d", literalExpression->iValue);
            break;
        case HLSLBaseType_Bool:
            writer.Write("%s", literalExpression->bValue ? "true" : "false");
            break;
        default:
            DVASSERT(0);
        }
    }
    else if (expression->nodeType == HLSLNodeType_UnaryExpression)
    {
        HLSLUnaryExpression* unaryExpression = static_cast<HLSLUnaryExpression*>(expression);
        const char* op = "?";
        bool pre = true;
        switch (unaryExpression->unaryOp)
        {
        case HLSLUnaryOp_Negative:
            op = "-";
            break;
        case HLSLUnaryOp_Positive:
            op = "+";
            break;
        case HLSLUnaryOp_Not:
            op = "!";
            break;
        case HLSLUnaryOp_PreIncrement:
            op = "++";
            break;
        case HLSLUnaryOp_PreDecrement:
            op = "--";
            break;
        case HLSLUnaryOp_PostIncrement:
            op = "++";
            pre = false;
            break;
        case HLSLUnaryOp_PostDecrement:
            op = "--";
            pre = false;
            break;
        default:
            break; // to shut up goddamn warning
        }
        writer.Write("(");
        if (pre)
        {
            writer.Write("%s", op);
            OutputExpression(unaryExpression->expression);
        }
        else
        {
            OutputExpression(unaryExpression->expression);
            writer.Write("%s", op);
        }
        writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_BinaryExpression)
    {
        HLSLBinaryExpression* binaryExpression = static_cast<HLSLBinaryExpression*>(expression);
        writer.Write("(");
        OutputExpression(binaryExpression->expression1);
        const char* op = "?";
        switch (binaryExpression->binaryOp)
        {
        case HLSLBinaryOp_Add:
            op = " + ";
            break;
        case HLSLBinaryOp_Sub:
            op = " - ";
            break;
        case HLSLBinaryOp_Mul:
            op = " * ";
            break;
        case HLSLBinaryOp_Div:
            op = " / ";
            break;
        case HLSLBinaryOp_Less:
            op = " < ";
            break;
        case HLSLBinaryOp_Greater:
            op = " > ";
            break;
        case HLSLBinaryOp_LessEqual:
            op = " <= ";
            break;
        case HLSLBinaryOp_GreaterEqual:
            op = " >= ";
            break;
        case HLSLBinaryOp_Equal:
            op = " == ";
            break;
        case HLSLBinaryOp_NotEqual:
            op = " != ";
            break;
        case HLSLBinaryOp_Assign:
            op = " = ";
            break;
        case HLSLBinaryOp_AddAssign:
            op = " += ";
            break;
        case HLSLBinaryOp_SubAssign:
            op = " -= ";
            break;
        case HLSLBinaryOp_MulAssign:
            op = " *= ";
            break;
        case HLSLBinaryOp_DivAssign:
            op = " /= ";
            break;
        case HLSLBinaryOp_And:
            op = " && ";
            break;
        case HLSLBinaryOp_Or:
            op = " || ";
            break;
        default:
            DVASSERT(0);
        }
        writer.Write("%s", op);
        OutputExpression(binaryExpression->expression2);
        writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_ConditionalExpression)
    {
        HLSLConditionalExpression* conditionalExpression = static_cast<HLSLConditionalExpression*>(expression);
        writer.Write("((");
        OutputExpression(conditionalExpression->condition);
        writer.Write(")?(");
        OutputExpression(conditionalExpression->trueExpression);
        writer.Write("):(");
        OutputExpression(conditionalExpression->falseExpression);
        writer.Write("))");
    }
    else if (expression->nodeType == HLSLNodeType_MemberAccess)
    {
        HLSLMemberAccess* memberAccess = static_cast<HLSLMemberAccess*>(expression);
        OutputExpression(memberAccess->object);
        writer.Write(".%s", memberAccess->field);
    }
    else if (expression->nodeType == HLSLNodeType_ArrayAccess)
    {
        HLSLArrayAccess* arrayAccess = static_cast<HLSLArrayAccess*>(expression);
        OutputExpression(arrayAccess->array);
        writer.Write("[");
        OutputExpression(arrayAccess->index);
        writer.Write("]");
    }
    else if (expression->nodeType == HLSLNodeType_FunctionCall)
    {
        HLSLFunctionCall* functionCall = static_cast<HLSLFunctionCall*>(expression);
        const char* name = functionCall->function->name;
        bool sampler_call = false;
        bool sampler_lod = String_Equal(name, "tex2Dlod") || String_Equal(name, "texCUBElod");
        bool sampler_cmp = String_Equal(name, "tex2Dcmp");
        bool sampler_prj = String_Equal(name, "tex2Dproj");
        bool sampler_grad = String_Equal(name, "tex2Dgrad");

        if (String_Equal(name, "tex2D") ||
            String_Equal(name, "tex2Dproj") ||
            String_Equal(name, "tex2Dcmp") ||
            String_Equal(name, "tex2Dlod") ||
            String_Equal(name, "texCUBE") ||
            String_Equal(name, "texCUBElod") ||
            String_Equal(name, "tex2Dgrad"))
        {
            sampler_call = true;
        }
        if (sampler_call)
        {
            if (sampler_lod)
            {
                DVASSERT(functionCall->argument->nodeType == HLSLNodeType_IdentifierExpression);
                HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(functionCall->argument);
                DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);

                if (identifier->expressionType.baseType == HLSLBaseType_Sampler2D)
                {
                #if (RHI_METAL_DISABLE_SAMPLING)
                    writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
                #else
                    writer.Write("%s_texture.sample( %s_sampler ", identifier->name, identifier->name);
                #endif
                    int arg = 2;

                    for (HLSLExpression* expr = identifier->nextExpression; expr; expr = expr->nextExpression)
                    {
                        writer.Write(", ");

                        if (arg == 3)
                            writer.Write("level(");
                        OutputExpression(expr);
                        if (arg == 3)
                            writer.Write(")");

                        ++arg;
                    }
                    writer.Write(")");
                }
                else if (identifier->expressionType.baseType == HLSLBaseType_SamplerCube)
                {
                #if (RHI_METAL_DISABLE_SAMPLING)
                    writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
                #else
                    writer.Write("%s_texture.sample( %s_sampler ", identifier->name, identifier->name);
                #endif
                    int arg = 2;

                    for (HLSLExpression* expr = identifier->nextExpression; expr; expr = expr->nextExpression)
                    {
                        writer.Write(", ");

                        if (arg == 3)
                            writer.Write("level(");
                        OutputExpression(expr);
                        if (arg == 3)
                            writer.Write(")");

                        ++arg;
                    }
                    writer.Write(")");
                }
            }
            else if (sampler_prj)
            {
                DVASSERT(functionCall->argument->nodeType == HLSLNodeType_IdentifierExpression);
                HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(functionCall->argument);
                DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);
                
            #if (RHI_METAL_DISABLE_SAMPLING)
                writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
            #else
                writer.Write("%s_texture.sample( %s_sampler, (", identifier->name, identifier->name);
            #endif
                HLSLExpression* expr = identifier->nextExpression;
                OutputExpression(expr);
                writer.Write(").xy / (");
                OutputExpression(expr);
                writer.Write(").w ");

                writer.Write(")");
            }
            else if (sampler_cmp)
            {
                DVASSERT(functionCall->argument->nodeType == HLSLNodeType_IdentifierExpression);
                HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(functionCall->argument);
                DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);

                if (identifier->expressionType.baseType == HLSLBaseType_Sampler2DShadow)
                {
                    HLSLExpression* expr = identifier->nextExpression;
                #if (RHI_METAL_DISABLE_SAMPLING)
                    writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
                #else
                    writer.Write("%s_texture.sample_compare(shared_compare_sampler ", identifier->name, identifier->name);
                #endif
                    writer.Write(", ");
                    OutputExpression(expr);
                    writer.Write(".xy, ");
                    OutputExpression(expr);
                    writer.Write(".z, ");
                    writer.Write("level(0) ");
                    writer.Write(")");
                }
            }
            else if (sampler_grad)
            {
                DVASSERT(functionCall->argument->nodeType == HLSLNodeType_IdentifierExpression);
                HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(functionCall->argument);
                DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);

#if (RHI_METAL_DISABLE_SAMPLING)
                writer.Write(RHI_METAL_SAMPLING_REPLACEMENT);
#else
                writer.Write("%s_texture.sample( %s_sampler ", identifier->name, identifier->name);
#endif
                int arg = 2;

                for (HLSLExpression* expr = identifier->nextExpression; expr; expr = expr->nextExpression)
                {
                    writer.Write(", ");

                    if (arg == 3)
                        writer.Write("gradient2d(");
                    OutputExpression(expr);
                    if (arg == 4)
                        writer.Write(")");

                    ++arg;
                }
                writer.Write(")");
            }
            else
            {
                OutputExpressionList(functionCall->argument);
                writer.Write(")");
            }
        }
        else if (String_Equal(name, "FramebufferFetch"))
        {
            bool allOk = false;
            if ((functionCall->numArguments == 1) && (functionCall->argument->nodeType == HLSLNodeType_LiteralExpression))
            {
                HLSLLiteralExpression* literalExpression = static_cast<HLSLLiteralExpression*>(functionCall->argument);
                if (literalExpression->type == HLSLBaseType_Int)
                {
                    writer.Write("framebuffer_%d", literalExpression->iValue);
                    allOk = true;
                }
            }
            if (!allOk)
            {
                DAVA::Logger::Error("line %d  - FramebufferFetch badly formatted", functionCall->line);
                writer.Write("InvalidFramebufferFetch");
            }
        }
        else
        {
            writer.Write("%s(", name);
            OutputExpressionList(functionCall->argument);
            for (DAVA::uint32 t : samplerInfo.samplerMap[name].texInfo)
            {
                writer.Write(", %s_texture", samplerInfo.texInfo[t].name.c_str());
                writer.Write(", %s_sampler", samplerInfo.texInfo[t].name.c_str());
            }
            for (DAVA::uint32 fb : samplerInfo.samplerMap[name].fbInfo)
            {
                writer.Write(", framebuffer_%d", fb);
            }
            writer.Write(")");
        }
    }
    else
    {
        writer.Write("<unknown expression>");
    }
}

void MSLGenerator::OutputArguments(HLSLArgument* argument)
{
    int numArgs = 0;
    while (argument != NULL)
    {
        if (numArgs > 0)
        {
            writer.Write(", ");
        }

        switch (argument->modifier)
        {
        case HLSLArgumentModifier_In:
            writer.Write("in ");
            break;
        case HLSLArgumentModifier_Out:
            writer.Write("out ");
            break;
        case HLSLArgumentModifier_Inout:
            writer.Write("inout ");
            break;
        case HLSLArgumentModifier_Uniform:
            writer.Write("uniform ");
            break;
        default:
            break; // to shut up goddamn warning
        }

        const char* semantic = argument->sv_semantic ? argument->sv_semantic : argument->semantic;
        int originalFlags = argument->type.flags;
        argument->type.flags |= HLSLTypeFlag_ThreadConts;
        argument->type.flags |= HLSLTypeFlag_Reference;
        OutputDeclaration(argument->type, argument->name, semantic, nullptr, argument->defaultValue);
        argument->type.flags = originalFlags;

        argument = argument->nextArgument;
        ++numArgs;
    }
}

const char* MSLGenerator::GetAttributeName(HLSLAttributeType attributeType)
{
    if (attributeType == HLSLAttributeType_Unroll)
        return "unroll";
    if (attributeType == HLSLAttributeType_Branch)
        return "branch";
    if (attributeType == HLSLAttributeType_Flatten)
        return "flatten";
    return NULL;
}

void MSLGenerator::OutputAttributes(int indent, HLSLAttribute* attribute)
{
    while (attribute != NULL)
    {
        const char* attributeName = GetAttributeName(attribute->attributeType);

        if (attributeName != NULL)
        {
            writer.WriteLineWithNumber(indent, attribute->fileName, attribute->line, "[%s]", attributeName);
        }

        attribute = attribute->nextAttribute;
    }
}

void MSLGenerator::OutputStatements(int indent, HLSLStatement* statement)
{
    while (statement != nullptr)
    {
        if (statement->hidden)
        {
            statement = statement->nextStatement;
            continue;
        }

        OutputAttributes(indent, statement->attributes);

        if (statement->nodeType == HLSLNodeType_Declaration)
        {
            HLSLDeclaration* declaration = static_cast<HLSLDeclaration*>(statement);
            if (!(declaration->type.flags & HLSLTypeFlag_Property))
            {
                writer.BeginLine(indent, declaration->fileName, declaration->line);
                OutputDeclaration(declaration);
                writer.EndLine(";");
            }
        }
        else if (statement->nodeType == HLSLNodeType_Struct)
        {
            HLSLStruct* structure = static_cast<HLSLStruct*>(statement);
            writer.WriteLineWithNumber(indent, structure->fileName, structure->line, "struct %s", structure->name);
            writer.WriteLineWithNumber(indent, structure->fileName, structure->line, "{");
            HLSLStructField* field = structure->field;
            while (field != NULL)
            {
                if (!field->hidden)
                {
                    writer.BeginLine(indent + 1, field->fileName, field->line);
                    const char* semantic = field->sv_semantic ? field->sv_semantic : field->semantic;
                    char attr_name[64];

                    if (structure->usage == struct_VertexIn || structure->usage == struct_VertexOut || structure->usage == struct_FragmentIn || structure->usage == struct_FragmentOut)
                    {
                        struct
                        {
                            const char* sv_semantic;
                            unsigned vattr;
                            const char* mtl_semantic;
                            const char* mtl_fp_semantic;
                        } attr[] =
                        {
                          { "POSITION", rhi::VATTR_POSITION_0, "position0", "" },
                          { "POSITION0", rhi::VATTR_POSITION_1, "position0", "" },
                          { "POSITION1", rhi::VATTR_POSITION_1, "position1", "" },
                          { "POSITION2", rhi::VATTR_POSITION_2, "position2", "" },
                          { "POSITION3", rhi::VATTR_POSITION_3, "position3", "" },
                          { "SV_POSITION", rhi::VATTR_POSITION_0, "position", "" },
                          { "NORMAL", rhi::VATTR_NORMAL_0, "normal0", "" },
                          { "NORMAL0", rhi::VATTR_NORMAL_0, "normal0", "" },
                          { "NORMAL1", rhi::VATTR_NORMAL_1, "normal1", "" },
                          { "NORMAL2", rhi::VATTR_NORMAL_2, "normal2", "" },
                          { "NORMAL3", rhi::VATTR_NORMAL_3, "normal3", "" },
                          { "TEXCOORD", rhi::VATTR_TEXCOORD_0, "texcoord0", "" },
                          { "TEXCOORD0", rhi::VATTR_TEXCOORD_0, "texcoord0", "" },
                          { "TEXCOORD1", rhi::VATTR_TEXCOORD_1, "texcoord1", "" },
                          { "TEXCOORD2", rhi::VATTR_TEXCOORD_2, "texcoord2", "" },
                          { "TEXCOORD3", rhi::VATTR_TEXCOORD_3, "texcoord3", "" },
                          { "TEXCOORD4", rhi::VATTR_TEXCOORD_4, "texcoord4", "" },
                          { "TEXCOORD5", rhi::VATTR_TEXCOORD_5, "texcoord5", "" },
                          { "TEXCOORD6", rhi::VATTR_TEXCOORD_6, "texcoord6", "" },
                          { "TEXCOORD7", rhi::VATTR_TEXCOORD_7, "texcoord7", "" },
                          { "COLOR", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "COLOR0", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "COLOR1", rhi::VATTR_COLOR_1, "color1", "color(1)" },
                          { "SV_TARGET", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "SV_TARGET0", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "SV_TARGET1", rhi::VATTR_COLOR_1, "color1", "color(1)" },
                          { "SV_TARGET2", rhi::VATTR_COLOR_1, "color2", "color(2)" },
                          { "SV_TARGET3", rhi::VATTR_COLOR_1, "color3", "color(3)" },
                          { "SV_TARGET4", rhi::VATTR_COLOR_1, "color4", "color(4)" },
                          { "SV_TARGET5", rhi::VATTR_COLOR_1, "color5", "color(5)" },
                          { "TANGENT", rhi::VATTR_TANGENT, "tangent", "" },
                          { "BINORMAL", rhi::VATTR_BINORMAL, "binormal", "" },
                          { "BLENDWEIGHT", rhi::VATTR_BLENDWEIGHT, "blend_weight", "" },
                          { "BLENDINDICES", rhi::VATTR_BLENDINDEX, "blend_index", "" }
                        };

                        for (unsigned i = 0; i != countof(attr); ++i)
                        {
                            if (stricmp(attr[i].sv_semantic, semantic) == 0)
                            {
                                if (structure->usage == struct_VertexIn)
                                    Snprintf(attr_name, countof(attr_name), "[[ attribute(%u) ]]", attr[i].vattr);
                                else if (structure->usage == struct_VertexOut && stricmp(semantic, "SV_POSITION") == 0)
                                    Snprintf(attr_name, countof(attr_name), "[[ position ]]");
                                else if (structure->usage == struct_VertexOut || structure->usage == struct_FragmentIn)
                                    Snprintf(attr_name, countof(attr_name), "[[ user(%s) ]]", attr[i].mtl_semantic);
                                else if (structure->usage == struct_FragmentOut)
                                    Snprintf(attr_name, countof(attr_name), "[[ %s ]]", attr[i].mtl_fp_semantic);

                                semantic = attr_name;
                                break;
                            }
                        }
                    }

                    OutputDeclaration(field->type, field->name, semantic);
                    writer.Write(";");
                    writer.EndLine();
                }
                field = field->nextField;
            }
            writer.WriteLine(indent, "};");
        }
        else if (statement->nodeType == HLSLNodeType_Buffer)
        {
            HLSLBuffer* buffer = static_cast<HLSLBuffer*>(statement);
            HLSLLiteralExpression* arr_sz = static_cast<HLSLLiteralExpression*>(buffer->field->type.arraySize);

            writer.WriteLineWithNumber
            (
            indent, buffer->fileName, buffer->line,
            "struct __%s { float4 data[%i]; };",
            buffer->name,
            buffer->registerCount
            );
        }
        else if (statement->nodeType == HLSLNodeType_Function)
        {
            HLSLFunction* function = static_cast<HLSLFunction*>(statement);
            HLSLArgument* arg = function->argument;

            if (strcmp(function->name, entryName) == 0)
            {
                const char* returnTypeName = GetTypeName(function->returnType);
                char btype = (target == TARGET_VERTEX) ? 'V' : 'F';

                writer.BeginLine(indent, function->fileName, function->line);
                writer.WriteLine(indent, "%s %s %s(", (target == TARGET_VERTEX) ? "vertex" : "[[early_fragment_tests]] fragment", returnTypeName, function->name);

                if (arg != nullptr)
                {
                    writer.WriteLine(indent + 1, "%s %s [[ stage_in ]]", GetTypeName(arg->type), arg->name);
                }

                for (unsigned i = 0; i != 32; ++i)
                {
                    char bname[64];
                    Snprintf(bname, countof(bname), "%cP_Buffer%u_t", btype, i);
                    if (tree->FindBuffer(bname))
                    {
                        writer.WriteLine(indent + 1, ", constant __%cP_Buffer%u_t& buf%u [[ buffer(%u) ]]", btype, i, i, (target == TARGET_VERTEX) ? rhi::MAX_VERTEX_STREAM_COUNT + i : i);
                    }
                }

                for (unsigned t = 0; t != samplerInfo.texInfo.size(); ++t)
                {
                    const char* ttype = "invalid_texture_type";
                    if (samplerInfo.texInfo[t].type == HLSLBaseType_Sampler2D)
                    {
                        ttype = "texture2d<float>";
                    }
                    else if (samplerInfo.texInfo[t].type == HLSLBaseType_SamplerCube)
                    {
                        ttype = "texturecube<float>";
                    }
                    else if (samplerInfo.texInfo[t].type == HLSLBaseType_Sampler2DShadow)
                    {
                        ttype = "depth2d<float>";
                    }

                    writer.WriteLine(indent + 1, ", %s %s_texture [[ texture(%u) ]]", ttype, samplerInfo.texInfo[t].name.c_str(), samplerInfo.texInfo[t].unit);
                    writer.WriteLine(indent + 1, ", sampler %s_sampler [[ sampler(%u) ]]", samplerInfo.texInfo[t].name.c_str(), samplerInfo.texInfo[t].unit);
                }
                for (DAVA::int32 buffer : samplerInfo.samplerMap[function->name].fbInfo)
                {
                    writer.WriteLine(indent + 1, ", float4 framebuffer_%d [[color(%d)]]", buffer, buffer);
                }
                writer.WriteLine(indent, ")");
                writer.WriteLine(indent, "{");
                for (unsigned i = 0; i != 32; ++i)
                {
                    char bname[64];

                    Snprintf(bname, countof(bname), "%cP_Buffer%u_t", btype, i);
                    if (tree->FindBuffer(bname))
                    {
                        writer.WriteLine(indent + 1, "constant float4* %cP_Buffer%u = buf%u.data;", btype, i, i);
                    }
                }

                for (HLSLStatement* s = tree->GetRoot()->statement; s; s = s->nextStatement)
                {
                    if (s->nodeType == HLSLNodeType_Declaration)
                    {
                        HLSLDeclaration* decl = static_cast<HLSLDeclaration*>(s);

                        if (!decl->hidden && (decl->type.flags & HLSLTypeFlag_Property))
                        {
                            writer.BeginLine(indent + 1);
                            if (decl->type.array)
                            {
                                writer.Write("constant float4* %s = (constant float4*)%s", decl->name, decl->registerName);
                            }
                            else
                            {
                                OutputDeclarationType(decl->type);
                                OutputDeclarationBody(decl->type, decl->name, decl->semantic, decl->registerName, decl->assignment);
                            }
                            writer.Write(";");
                            writer.EndLine();
                        }
                    }
                }

                OutputStatements(indent + 1, function->statement);
                writer.WriteLine(indent, "};");
            }
            else
            {
                // Use an alternate name for the function which is supposed to be entry point
                // so that we can supply our own function which will be the actual entry point.
                const char* functionName = function->name;
                const char* returnTypeName = GetTypeName(function->returnType);

                writer.BeginLine(indent, function->fileName, function->line);
                writer.Write("inline %s %s(", returnTypeName, functionName);

                OutputArguments(arg);

                SamplerInfo::FunctionInfo& info = samplerInfo.samplerMap[functionName];
                if (info.texInfo.size())
                    writer.WriteLine(0, "");

                for (DAVA::uint32 t : info.texInfo)
                {
                    const char* ttype = "";
                    if (samplerInfo.texInfo[t].type == HLSLBaseType_Sampler2D)
                    {
                        ttype = "texture2d<float>";
                    }
                    else if (samplerInfo.texInfo[t].type == HLSLBaseType_SamplerCube)
                    {
                        ttype = "texturecube<float>";
                    }
                    else if (samplerInfo.texInfo[t].type == HLSLBaseType_Sampler2DShadow)
                    {
                        ttype = "depth2d<float>";
                    }

                    writer.WriteLine(indent + 1, ", %s %s_texture", ttype, samplerInfo.texInfo[t].name.c_str(), samplerInfo.texInfo[t].unit);
                    writer.WriteLine(indent + 1, ", sampler %s_sampler", samplerInfo.texInfo[t].name.c_str(), samplerInfo.texInfo[t].unit);
                }
                for (DAVA::int32 buffer : info.fbInfo)
                {
                    writer.WriteLine(indent + 1, ", float4 framebuffer_%d", buffer, buffer);
                }

                const char* semantic = function->sv_semantic ? function->sv_semantic : function->semantic;
                if (semantic != NULL)
                {
                    writer.Write(") : %s {", semantic);
                }
                else
                {
                    writer.Write(") {");
                }

                writer.EndLine();

                OutputStatements(indent + 1, function->statement);
                writer.WriteLine(indent, "};");
            }
        }
        else if (statement->nodeType == HLSLNodeType_ExpressionStatement)
        {
            HLSLExpressionStatement* expressionStatement = static_cast<HLSLExpressionStatement*>(statement);
            writer.BeginLine(indent, statement->fileName, statement->line);
            OutputExpression(expressionStatement->expression);
            writer.EndLine(";");
        }
        else if (statement->nodeType == HLSLNodeType_ReturnStatement)
        {
            HLSLReturnStatement* returnStatement = static_cast<HLSLReturnStatement*>(statement);
            if (returnStatement->expression != NULL)
            {
                writer.BeginLine(indent, returnStatement->fileName, returnStatement->line);
                writer.Write("return ");
                OutputExpression(returnStatement->expression);
                writer.EndLine(";");
            }
            else
            {
                writer.WriteLineWithNumber(indent, returnStatement->fileName, returnStatement->line, "return;");
            }
        }
        else if (statement->nodeType == HLSLNodeType_DiscardStatement)
        {
            HLSLDiscardStatement* discardStatement = static_cast<HLSLDiscardStatement*>(statement);
            writer.WriteLineWithNumber(indent, discardStatement->fileName, discardStatement->line, "discard_fragment();");
        }
        else if (statement->nodeType == HLSLNodeType_BreakStatement)
        {
            HLSLBreakStatement* breakStatement = static_cast<HLSLBreakStatement*>(statement);
            writer.WriteLineWithNumber(indent, breakStatement->fileName, breakStatement->line, "break;");
        }
        else if (statement->nodeType == HLSLNodeType_ContinueStatement)
        {
            HLSLContinueStatement* continueStatement = static_cast<HLSLContinueStatement*>(statement);
            writer.WriteLineWithNumber(indent, continueStatement->fileName, continueStatement->line, "continue;");
        }
        else if (statement->nodeType == HLSLNodeType_IfStatement)
        {
            HLSLIfStatement* ifStatement = static_cast<HLSLIfStatement*>(statement);
            writer.BeginLine(indent, ifStatement->fileName, ifStatement->line);
            writer.Write("if (");
            OutputExpression(ifStatement->condition);
            writer.Write(") {");
            writer.EndLine();
            OutputStatements(indent + 1, ifStatement->statement);
            writer.WriteLine(indent, "}");
            if (ifStatement->elseStatement != NULL)
            {
                writer.WriteLine(indent, "else {");
                OutputStatements(indent + 1, ifStatement->elseStatement);
                writer.WriteLine(indent, "}");
            }
        }
        else if (statement->nodeType == HLSLNodeType_ForStatement)
        {
            HLSLForStatement* forStatement = static_cast<HLSLForStatement*>(statement);
            writer.BeginLine(indent, forStatement->fileName, forStatement->line);
            writer.Write("for (");
            OutputDeclaration(forStatement->initialization);
            writer.Write("; ");
            OutputExpression(forStatement->condition);
            writer.Write("; ");
            OutputExpression(forStatement->increment);
            writer.Write(") {");
            writer.EndLine();
            OutputStatements(indent + 1, forStatement->statement);
            writer.WriteLine(indent, "}");
        }
        else if (statement->nodeType == HLSLNodeType_BlockStatement)
        {
            HLSLBlockStatement* blockStatement = static_cast<HLSLBlockStatement*>(statement);
            writer.WriteLineWithNumber(indent, blockStatement->fileName, blockStatement->line, "{");
            OutputStatements(indent + 1, blockStatement->statement);
            writer.WriteLine(indent, "}");
        }
        else if (statement->nodeType == HLSLNodeType_Technique)
        {
            // Techniques are ignored.
        }
        else if (statement->nodeType == HLSLNodeType_Pipeline)
        {
            // Pipelines are ignored.
        }
        else
        {
            // Unhanded statement type.
            DVASSERT(0);
        }

        statement = statement->nextStatement;
    }
}

void MSLGenerator::OutputDeclaration(HLSLDeclaration* declaration)
{
    //GFX_COMPLETE - samper info is now collected on pre-phase
    //samplers in metal are passed as function arguments
    if (IsSamplerType(declaration->type))
    {
        return;
    }

    OutputDeclarationType(declaration->type);
    OutputDeclarationBody(declaration->type, declaration->name, declaration->semantic, declaration->registerName, declaration->assignment);
    declaration = declaration->nextDeclaration;

    while (declaration != NULL)
    {
        writer.Write(", ");
        OutputDeclarationBody(declaration->type, declaration->name, declaration->semantic, declaration->registerName, declaration->assignment);
        declaration = declaration->nextDeclaration;
    };
}

void MSLGenerator::OutputDeclarationType(const HLSLType& type)
{
    const char* typeName = GetTypeName(type);

    if (type.flags & HLSLTypeFlag_Const)
    {
        writer.Write("constant ");
    }
    if (type.flags & HLSLTypeFlag_ThreadConts)
    {
        writer.Write("thread const ");
    }
    if (type.flags & HLSLTypeFlag_Static && !(type.flags & HLSLTypeFlag_Property))
    {
        writer.Write("static ");
    }

    // Interpolation modifiers.
    if (type.flags & HLSLTypeFlag_Centroid)
    {
        writer.Write("centroid ");
    }
    if (type.flags & HLSLTypeFlag_Linear)
    {
        writer.Write("linear ");
    }
    if (type.flags & HLSLTypeFlag_NoInterpolation)
    {
        writer.Write("nointerpolation ");
    }
    if (type.flags & HLSLTypeFlag_NoPerspective)
    {
        writer.Write("noperspective ");
    }
    if (type.flags & HLSLTypeFlag_Sample) // @@ Only in shader model >= 4.1
    {
        writer.Write("sample ");
    }

    if (type.flags & HLSLTypeFlag_Reference)
    {
        writer.Write("%s& ", typeName);
    }
    else
    {
        writer.Write("%s ", typeName);
    }
}

void MSLGenerator::OutputDeclarationBody(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
{
    writer.Write("%s", name);

    if (type.array)
    {
        DVASSERT(semantic == NULL);
        writer.Write("[");
        if (type.arraySize != NULL)
        {
            OutputExpression(type.arraySize);
        }
        writer.Write("]");
    }

    if (semantic != NULL)
    {
        writer.Write(" %s", semantic);
    }

    if (registerName != NULL)
    {
        if (isInsideBuffer)
        {
            writer.Write(" : packoffset(%s)", registerName);
        }
        else
        {
            writer.Write(" : register(%s)", registerName);
        }
    }

    if (assignment != NULL && !IsSamplerType(type))
    {
        writer.Write(" = ");
        if (type.array)
        {
            writer.Write("{ ");
            OutputExpressionList(assignment);
            writer.Write(" }");
        }
        else
        {
            OutputExpression(assignment);
        }
    }
}

void MSLGenerator::OutputDeclaration(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
{
    OutputDeclarationType(type);
    OutputDeclarationBody(type, name, semantic, registerName, assignment);
}

} // namespace sl
