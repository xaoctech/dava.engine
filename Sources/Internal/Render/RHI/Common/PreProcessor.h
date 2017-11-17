#pragma once

#include "ExpressionEvaluator.h"

namespace DAVA
{
class PreProc
{
public:
    struct FileCallback
    {
        virtual ~FileCallback() = default;
        virtual bool Open(const char* /*file_name*/) = 0;
        virtual void Close() = 0;
        virtual uint32 Size() const = 0;
        virtual uint32 Read(uint32 /*max_sz*/, void* /*dst*/) = 0;
    };
    using TextBuffer = std::vector<char>;

public:
    PreProc(FileCallback* fc = nullptr);
    ~PreProc();

    bool ProcessFile(const char* file_name, TextBuffer* output);
    bool Process(const char* src_text, TextBuffer* output);
    void Clear();

    bool AddDefine(const char* name, const char* value);

private:
    struct Line
    {
        uint32 line_n;
        const char* text;
        Line(const char* t, uint32 n)
            : text(t)
            , line_n(n)
        {
        }
    };
    using LineVector = std::vector<Line>;

    void Reset();
    char* AllocBuffer(uint32 sz);
    bool ProcessInplaceInternal(char* src_text, TextBuffer* output);
    bool ProcessBuffer(char* text, LineVector& line);
    bool ProcessInclude(const char* file_name, LineVector& line);
    bool ProcessDefine(const char* name, const char* val);
    void Undefine(const char* name);
    void GenerateOutput(TextBuffer* output, LineVector& line);

    char* GetExpression(char* txt, char** end) const;
    char* GetIdentifier(char* txt, char** end) const;
    int32 GetNameAndValue(char* txt, char** name, char** value, char** end) const;
    void ReportExprEvalError(uint32 line_n);
    char* ExpandMacroInLine(char* txt);
    char* GetToken(char* str, ptrdiff_t strSize, const char* m, ptrdiff_t tokenSize);

private:
    struct Buffer
    {
        void* mem;
    };

    struct Var
    {
        char name[64];
        int32 val;
    };

    struct MacroName
    {
        char name[128]{};
        uint32 nameLength = 0;

        MacroName() = default;

        MacroName(const char* nm, ptrdiff_t sz)
            :
            nameLength(sz)
        {
            memcpy(name, nm, std::min<ptrdiff_t>(sz, sizeof(name)));
        }

        bool operator==(const MacroName& r) const
        {
            bool equals = false;
            if (nameLength == r.nameLength)
            {
                equals = (strcmp(name, r.name) == 0);
            }
            return equals;
        }
    };

    struct MacroValue
    {
        char value[128]{};
        uint32 valueLength = 0;

        MacroValue() = default;

        MacroValue(const char* nm, ptrdiff_t sz)
            :
            valueLength(sz)
        {
            memcpy(value, nm, std::min<ptrdiff_t>(sz, sizeof(value)));
        }
    };

    struct MacroNameHash
    {
        uint64 operator()(const MacroName& m) const
        {
            const uint32 _FNV_offset_basis = 2166136261U;
            const uint32 _FNV_prime = 16777619U;
            uint32 result = _FNV_offset_basis;
            for (uint32 i = 0; i < m.nameLength; ++i)
            {
                result ^= static_cast<uint32>(m.name[i]);
                result *= _FNV_prime;
            }
            return result;
        }
    };

    enum : uint32
    {
        InvalidValue = static_cast<uint32>(-1)
    };

    enum : char
    {
        Tab = '\t',
        Zero = '\0',
        Space = ' ',
        NewLine = '\n',
        DoubleQuotes = '\"',
        CarriageReturn = '\r',
    };

    Vector<Buffer> buffer;
    Vector<Var> variable;
    UnorderedMap<MacroName, MacroValue, MacroNameHash> macro;
    ExpressionEvaluator evaluator;
    FileCallback* fileCB = nullptr;
    const char* curFileName = "<buffer>";
    uint32 minMacroLength = InvalidValue;
};
}