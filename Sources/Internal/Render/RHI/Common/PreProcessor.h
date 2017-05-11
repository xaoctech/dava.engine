#pragma once

#include <vector>
#include "FileSystem/File.h"
#include "ExpressionEvaluator.h"

typedef std::vector<char> TextBuf;

class
PreProc
{
public:
    class FileCallback;

    PreProc(FileCallback* fc = nullptr);
    ~PreProc();

    bool ProcessFile(const char* file_name, TextBuf* output);
    bool ProcessInplace(char* src_text, TextBuf* output);
    bool Process(const char* src_text, TextBuf* output);
    void Clear();

    bool AddDefine(const char* name, const char* value);

    void Dump() const;

public:
    class
    FileCallback
    {
    public:
        virtual ~FileCallback()
        {
        }

        virtual bool Open(const char* /*file_name*/) = 0;
        virtual void Close() = 0;
        virtual unsigned Size() const = 0;
        virtual unsigned Read(unsigned /*max_sz*/, void* /*dst*/) = 0;
    };

private:
    struct Line;
    struct Buffer;
    struct Var;

    void Reset();
    char* AllocBuffer(unsigned sz);
    bool ProcessInplaceInternal(char* src_text, TextBuf* output);
    bool ProcessBuffer(char* text, std::vector<Line>* line);
    bool ProcessInclude(const char* file_name, std::vector<Line>* line);
    bool ProcessDefine(const char* name, const char* val);
    void Undefine(const char* name);
    void GenerateOutput(TextBuf* output);

    char* GetExpression(char* txt, char** end) const;
    char* GetIdentifier(char* txt, char** end) const;
    int GetNameAndValue(char* txt, char** name, char** value, char** end) const;
    void ReportExprEvalError(unsigned line_n);

    struct
    Line
    {
        char* text;
        unsigned line_n;

        Line(char* t, unsigned n)
            : text(t)
            , line_n(n)
        {
        }
    };

    struct
    Buffer
    {
        void* mem;
    };

    struct
    Var
    {
        char name[64];
        int val;
    };

    std::vector<Buffer> buffer;
    std::vector<Line> line;
    std::vector<Var> variable;
    ExpressionEvaluator evaluator;

    FileCallback* fileCB;
    const char* curFileName;

    class
    DefaultFileCallback
    : public FileCallback
    {
    public:
        ~DefaultFileCallback()
        {
            if (in)
                in->Release();
        }
        virtual bool Open(const char* file_name)
        {
            in = DAVA::File::Create(file_name, DAVA::File::READ | DAVA::File::OPEN);
            return (in) ? true : false;
        }
        virtual void Close()
        {
            DVASSERT(in);
            in->Release();
            in = nullptr;
        }
        virtual unsigned Size() const
        {
            return (in) ? unsigned(in->GetSize()) : 0;
        }
        virtual unsigned Read(unsigned max_sz, void* dst)
        {
            return (in) ? in->Read(dst, max_sz) : 0;
        }

    private:
        DAVA::File* in = nullptr;
    };

    struct
    macro_t
    {
        char name[128];
        char value[128];
        unsigned name_len;
        unsigned value_len;
    };

    std::vector<macro_t> macro;
    unsigned minMacroLength;

    static DefaultFileCallback DefFileCallback;
};
