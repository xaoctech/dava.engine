#pragma once

#include "ExpressionEvaluator.h"

class PreProc
{
public:
    struct FileCallback
    {
        virtual ~FileCallback() = default;
        virtual bool Open(const char* /*file_name*/) = 0;
        virtual void Close() = 0;
        virtual uint32_t Size() const = 0;
        virtual uint32_t Read(uint32_t /*max_sz*/, void* /*dst*/) = 0;
    };
    using TextBuffer = std::vector<char>;

public:
    PreProc(FileCallback* fc = nullptr);
    ~PreProc();

    bool ProcessFile(const char* file_name, TextBuffer* output);
    bool Process(const char* src_text, TextBuffer* output);
    void Clear();

    bool AddDefine(const char* name, const char* value);

    void Dump() const;

private:
    struct Line
    {
        uint32_t line_n;
        const char* text;
        Line(const char* t, uint32_t n)
            : text(t)
            , line_n(n)
        {
        }
    };
    using LineVector = std::vector<Line>;

    void Reset();
    char* AllocBuffer(uint32_t sz);
    bool ProcessInplaceInternal(char* src_text, TextBuffer* output);
    bool ProcessBuffer(char* text, LineVector& line);
    bool ProcessInclude(const char* file_name, LineVector& line);
    bool ProcessDefine(const char* name, const char* val);
    void Undefine(const char* name);
    void GenerateOutput(TextBuffer* output, LineVector& line);

    char* GetExpression(char* txt, char** end) const;
    char* GetIdentifier(char* txt, char** end) const;
    int32_t GetNameAndValue(char* txt, char** name, char** value, char** end) const;
    void ReportExprEvalError(uint32_t line_n);

private:
    struct Buffer
    {
        void* mem;
    };

    struct Var
    {
        char name[64];
        int32_t val;
    };

    struct Macro
    {
        char name[128];
        char value[128];
        uint32_t name_len;
        uint32_t value_len;
    };

    enum : uint32_t
    {
        InvalidValue = static_cast<uint32_t>(-1)
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

    std::vector<Buffer> buffer;
    std::vector<Var> variable;
    std::vector<Macro> macro;
    ExpressionEvaluator evaluator;
    FileCallback* fileCB = nullptr;
    const char* curFileName = "<buffer>";
    uint32_t minMacroLength = InvalidValue;
};
