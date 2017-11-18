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

    struct Macro
    {
        char name[128];
        char value[128];
        uint32 name_len = 0;
        uint32 value_len = 0;

        Macro(const char* nm, uint32 nmLen, const char* val, uint32 valLen)
            : name_len(nmLen)
            , value_len(valLen)
        {
            memset(name, 0, sizeof(name));
            memset(value, 0, sizeof(value));
            strncpy(name, nm, std::min<size_t>(name_len, sizeof(name)));
            strncpy(value, val, std::min<size_t>(value_len, sizeof(value)));
        }

        bool operator<(const Macro& r) const
        {
            if (name_len == r.name_len)
                return strcmp(name, r.name) > 0;

            return name_len > r.name_len;
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
    Set<Macro> macro;
    ExpressionEvaluator evaluator;
    FileCallback* fileCB = nullptr;
    const char* curFileName = "<buffer>";
    uint32 minMacroLength = InvalidValue;
};
}