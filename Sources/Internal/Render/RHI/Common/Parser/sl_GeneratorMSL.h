#pragma once

#include "sl_CodeWriter.h"
#include "sl_Tree.h"

namespace sl
{
class HLSLTree;
class SamplerFetchVisitor;
struct HLSLFunction;
struct HLSLStruct;

class MSLGenerator
{
public:
    explicit MSLGenerator(Allocator* allocator);

    bool Generate(HLSLTree* tree, Target target, const char* entryName, std::string* code);
    const char* GetResult() const;

private:
    static const char* GetAttributeName(HLSLAttributeType attributeType);
    static const char* GetTypeName(const HLSLType& type);

    void OutputExpressionList(HLSLExpression* expression);
    void OutputExpression(HLSLExpression* expression);
    void OutputArguments(HLSLArgument* argument);
    void OutputAttributes(int indent, HLSLAttribute* attribute);
    void OutputStatements(int indent, HLSLStatement* statement);
    void OutputDeclaration(HLSLDeclaration* declaration);
    void OutputDeclaration(const HLSLType& type, const char* name, const char* semantic = NULL, const char* registerName = NULL, HLSLExpression* defaultValue = NULL);
    void OutputDeclarationType(const HLSLType& type);
    void OutputDeclarationBody(const HLSLType& type, const char* name, const char* semantic = NULL, const char* registerName = NULL, HLSLExpression* assignment = NULL);

private:
    struct SamplerInfo
    {
        struct tex_t
        {
            std::string name;
            HLSLBaseType type;
            unsigned unit;
        };

        struct FunctionInfo
        {
            DAVA::Set<DAVA::uint32> texInfo;
            DAVA::Set<DAVA::uint32> fbInfo;
        };

        DAVA::Vector<tex_t> texInfo;
        DAVA::Map<DAVA::String, FunctionInfo> samplerMap;

        void Clear()
        {
            texInfo.clear();
            samplerMap.clear();
        }
    };

private:
    CodeWriter writer;
    SamplerInfo samplerInfo;
    const HLSLTree* tree = nullptr;
    const char* entryName = nullptr;
    Target target = Target::TARGET_VERTEX;
    bool isInsideBuffer = false;

    friend class SamplerFetchVisitor;
};

} // namespace sl
