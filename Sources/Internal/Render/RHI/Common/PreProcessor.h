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

    char* GetNextToken(char* txt, ptrdiff_t txtSize, ptrdiff_t& tokenSize);
    const char* GetNextToken(const char* txt, ptrdiff_t txtSize, ptrdiff_t& tokenSize);

public:
    enum : uint32
    {
        MaxMacroNameLength = 256,
        MaxLocalStringLength = 8192,
    };

    struct MacroStringBuffer
    {
        enum class Transient : uint32
        {
        };
        enum class Permanent : uint32
        {
        };

        const char* transientValue = nullptr;
        char value[MaxMacroNameLength]{};
        uint32 length = 0;
        bool isTransient = false;

        MacroStringBuffer() = default;

        MacroStringBuffer(Permanent p, const char* nm, uint32 sz)
            : length(sz)
        {
            memcpy(value, nm, std::min(sz, uint32(MaxMacroNameLength)));
        }

        MacroStringBuffer(Transient t, const char* nm, uint32 sz)
            : transientValue(nm)
            , length(sz)
            , isTransient(true)
        {
        }

        bool operator==(const MacroStringBuffer& r) const
        {
            bool equals = false;
            if (length == r.length)
            {
                equals = (strncmp(GetValue(), r.GetValue(), length) == 0);
            }
            return equals;
        }

        const char* GetValue() const
        {
            return isTransient ? transientValue : value;
        }
    };

    struct MacroStringBufferHash
    {
        uint64 operator()(const MacroStringBuffer& m) const
        {
            enum : uint32
            {
                _FNV_offset_basis = 2166136261U,
                _FNV_prime = 16777619U,
            };

            const char* ptr = m.GetValue();
            uint32 result = _FNV_offset_basis;
            for (uint32 i = 0; i < m.length; ++i)
            {
                result ^= static_cast<uint32>(ptr[i]);
                result *= _FNV_prime;
            }
            return result;
        }
    };

    using MacroMap = UnorderedMap<MacroStringBuffer, MacroStringBuffer, MacroStringBufferHash>;

private:
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

    MacroMap macro;
    Vector<char*> buffer;
    ExpressionEvaluator evaluator;
    FileCallback* fileCB = nullptr;
    const char* curFileName = "<buffer>";
};
}