#include "Render/RHI/Common/PreProcessor.h"
#include "Render/RHI/Common/Preprocessor/PreprocessorHelpers.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "rhi_Utils.h"

namespace DAVA
{
namespace PreprocessorHelpers
{
struct DefaultFileCallback : public PreProc::FileCallback
{
    ScopedPtr<File> in;

    bool Open(const char* file_name) override
    {
        in.reset(File::Create(file_name, File::READ | File::OPEN));
        return (in.get() != nullptr);
    }
    void Close() override
    {
        DVASSERT(in.get() != nullptr);
        in.reset(nullptr);
    }
    uint32 Size() const override
    {
        return (in.get() != nullptr) ? static_cast<uint32>(in->GetSize()) : 0;
    }
    uint32 Read(uint32 max_sz, void* dst) override
    {
        return (in.get() != nullptr) ? in->Read(dst, max_sz) : 0;
    }
};

static DefaultFileCallback defaultFileCallback;
}

PreProc::PreProc(FileCallback* fc)
    : fileCB((fc) ? fc : &PreprocessorHelpers::defaultFileCallback)
{
}

PreProc::~PreProc()
{
    Clear();
}

//------------------------------------------------------------------------------

bool PreProc::ProcessFile(const char* file_name, TextBuffer* output)
{
    bool success = false;

    if (fileCB->Open(file_name))
    {
        Reset();
        curFileName = file_name;

        uint32 text_sz = fileCB->Size();
        char* text = AllocBuffer(text_sz + 1);
        fileCB->Read(text_sz, text);
        fileCB->Close();

        success = ProcessInplaceInternal(text, output);
    }
    else
    {
        Logger::Error("Failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::Process(const char* src_text, TextBuffer* output)
{
    Reset();

    char* text = AllocBuffer(uint32(strlen(src_text)) + 1);
    strcpy(text, src_text);

    return ProcessInplaceInternal(text, output);
}

//------------------------------------------------------------------------------

void PreProc::Clear()
{
    Reset();
    minMacroLength = InvalidValue;
    macro.clear();
}

//------------------------------------------------------------------------------

bool PreProc::AddDefine(const char* name, const char* value)
{
    return ProcessDefine(name, value);
}

//------------------------------------------------------------------------------

void PreProc::Dump() const
{
    /*
    for (uint32 i = 0; i != line.size(); ++i)
    {
        Logger::Info("%04u | %s", line[i].line_n, line[i].text);
    }
    */
}

//------------------------------------------------------------------------------

void PreProc::Reset()
{
    for (uint32 b = 0; b != buffer.size(); ++b)
        ::free(buffer[b].mem);
    buffer.clear();

    curFileName = "<buffer>";
}

//------------------------------------------------------------------------------

char* PreProc::AllocBuffer(uint32 sz)
{
    Buffer buf;
    buf.mem = ::calloc(1, sz);
    buffer.push_back(buf);
    return reinterpret_cast<char*>(buf.mem);
}

//------------------------------------------------------------------------------

inline char* PreProc::GetExpression(char* txt, char** end) const
{
    *end = SeekToLineEnding(txt);
    **end = 0;
    return txt;
}

//------------------------------------------------------------------------------

char* PreProc::GetIdentifier(char* txt, char** end) const
{
    char* t = txt;
    char* n = nullptr;

    while (!IsValidAlphaNumericChar(*t))
    {
        if (*t == Zero)
            return nullptr;
        ++t;
    }

    if (*t == Zero)
        return nullptr;

    n = t;

    while (IsValidAlphaNumericChar(*t))
        ++t;

    if (*t == Zero)
        return nullptr;

    if (*t != NewLine)
    {
        *t = Zero;
        ++t;
    }

    while (*t != NewLine)
    {
        if (*t == Zero)
            return nullptr;

        ++t;
    }

    *t = Zero;
    *end = t;
    return n;
}

//------------------------------------------------------------------------------

int32 PreProc::GetNameAndValue(char* txt, char** name, char** value, char** end) const
{
    // returns:
    // non-zero when name/value successfully retrieved
    // zero, if name/value not retieved;
    // -1, if end-of-file was encountered (name/value successfully retrieved)

    char* t = txt;
    char* n0 = nullptr;
    char* n1 = nullptr;
    char* v0 = nullptr;
    char* v1 = nullptr;

    while (*t == Space || *t == Tab)
        ++t;

    if (*t == Zero)
        return 0;

    n0 = t;

    while (*t && *t != Space && *t != Tab && *t != NewLine)
        ++t;

    if (*t == Zero)
        return 0;

    n1 = t - 1;
    while (*t == Space || *t == Tab)
        ++t;

    if (*t == Zero)
        return 0;

    v0 = t;
    int32 brace_lev = 0;
    while (*t && (*t != Space || brace_lev > 0) && *t != Tab && *t != NewLine)
    {
        if (*t == '(')
            ++brace_lev;
        else if (*t == ')')
            --brace_lev;

        ++t;
    }

    if (*t == Zero)
        return 0;

    v1 = t - 1;

    *name = n0;
    *value = v0;

    if (*t != Zero)
    {
        if (*t != NewLine)
        {
            *t = Zero;
            ++t;
        }

        while (*t != NewLine)
        {
            if (*t == 0)
                return 0;
            ++t;
        }
        *t = Zero;
        *end = t;

        *(n1 + 1) = Zero;
        *(v1 + 1) = Zero;

        return 1;
    }
    else
    {
        *end = t;
        *(n1 + 1) = Zero;
        *(v1 + 1) = Zero;
        return -1;
    }
}

//------------------------------------------------------------------------------

bool PreProc::ProcessBuffer(char* inputText, LineVector& lines)
{
    uint32 inputTextLength = static_cast<uint32>(strlen(inputText));
    char* filteredText = AllocBuffer(inputTextLength + 1);
    char* filteredTextPtr = filteredText;
    for (char* s = inputText; *s != Zero; ++s)
    {
        char* begin = s;

        bool unterminatedComment = false;
        s = SkipCommentBlock(s, unterminatedComment);

        if (unterminatedComment)
        {
            Logger::Error("Unterminated comment, starting at:\n%s", begin);
            return false;
        }

        s = SkipCommentLine(s);

        char currentChar = *s;
        if (currentChar != CarriageReturn)
        {
            *filteredTextPtr++ = currentChar;
            if (currentChar == '#')
                s = SkipWhitespace(s + 1) - 1;
        }
    }

    bool success = true;
    char* ln = filteredText;
    bool ln_external = false;
    char* last_s = nullptr;
    uint32 line_n = 1;
    uint32 src_line_n = 1;

    struct condition_t
    {
        bool original_condition;
        bool effective_condition;
        bool do_skip_lines;
    };

    std::vector<condition_t> pending_elif;
    bool dcheck_pending = true;

    pending_elif.emplace_back();
    pending_elif.back().do_skip_lines = false;

    for (char* s = filteredText; *s; ++s)
    {
        int32 skipping_line = false;

        for (int32 i = int(pending_elif.size()) - 1; i >= 0; --i)
        {
            if (pending_elif[i].do_skip_lines)
            {
                skipping_line = true;
                break;
            }
        }

        if (skipping_line)
        {
            bool do_skip = true;

            if (dcheck_pending)
            {
                char* ns1 = s;

                while (*ns1 && (*ns1 == Space || *ns1 == Tab))
                    ++ns1;

                if (*ns1 == Zero)
                {
                    success = false;
                    break;
                }

                if (*ns1 == '#')
                    do_skip = false;
            }

            if (do_skip)
            {
                while (*s && *s != NewLine)
                    ++s;

                if (*s == Zero)
                    break;

                ln = s = s + 1;
                ++src_line_n;
                dcheck_pending = true;
            }
        }

        bool expand_macros = false;

        if (*s == NewLine)
        {
            *s = Zero;
            lines.emplace_back(ln, line_n);

            if (ln_external)
            {
                s = last_s;
                ln_external = false;
            }

            ln = s + 1;
            ++line_n;
            ++src_line_n;
            dcheck_pending = true;
        }
        else if (dcheck_pending)
        {
            char* ns1 = SkipWhitespace(s);

            if (*ns1 == Zero)
            {
                success = false;
                break;
            }

            if (*ns1 == '#')
            {
                s = ns1;
                if (!skipping_line && strncmp(s + 1, "include", 7) == 0)
                {
                    char* t = s;
                    char* f0 = nullptr;
                    char* f1 = nullptr;

                    while (*t != DoubleQuotes)
                    {
                        if (*t == 0)
                            return false;
                        ++t;
                    }
                    f0 = t + 1;
                    ++t;
                    while (*t != DoubleQuotes)
                    {
                        if (*t == 0)
                            return false;
                        ++t;
                    }
                    f1 = t - 1;

                    char fname[256] = {};
                    strncpy(fname, f0, f1 - f0 + 1);

                    if (!ProcessInclude(fname, lines))
                        return false;

                    while (*t != NewLine)
                    {
                        if (*t == Zero)
                            return false;
                        ++t;
                    }

                    if (*t == Zero)
                        break;

                    s = t;
                    ln = t + 1;
                }
                else if (!skipping_line && strncmp(s + 1, "define", 6) == 0)
                {
                    char* name = nullptr;
                    char* value = nullptr;
                    int32 nv = GetNameAndValue(s + 1 + 6, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == Zero)
                        {
                            Logger::Error("#define without value not allowed (%s)", name);
                            success = false;
                            break;
                        }

                        if (!ProcessDefine(name, value))
                        {
                            success = false;
                            break;
                        }

                        if (nv != -1)
                        {
                            *s = NewLine; // since it was null'ed in GetNameAndValue
                            ln = s + 1;
                        }
                        else
                        {
                            ln = s;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else if (!skipping_line && strncmp(s + 1, "ensuredefined", 13) == 0)
                {
                    char* name = nullptr;
                    char* value = nullptr;
                    int32 nv = GetNameAndValue(s + 1 + 13, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == Zero)
                        {
                            Logger::Error("#define without value not allowed (%s)", name);
                            success = false;
                            break;
                        }

                        if (!evaluator.HasVariable(name))
                            evaluator.SetVariable(name, float(atof(value)));

                        if (nv != -1)
                        {
                            *s = NewLine; // since it was null'ed in GetNameAndValue
                            ln = s + 1;
                        }
                        else
                        {
                            ln = s;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else if (!skipping_line && strncmp(s + 1, "undef", 5) == 0)
                {
                    char* name = GetIdentifier(s + 1 + 5, &s);
                    if (!name)
                        break;

                    Undefine(name);

                    *s = NewLine; // since it was null'ed in GetIdentifier
                    ln = s + 1;
                }
                else if (strncmp(s + 1, "ifdef", 5) == 0)
                {
                    char* name = GetIdentifier(s + 1 + 5, &s);
                    if (!name)
                        break;
                    bool condition = evaluator.HasVariable(name);
                    condition_t p;

                    p.original_condition = condition;
                    p.effective_condition = condition;
                    p.do_skip_lines = !condition;
                    pending_elif.push_back(p);

                    *s = NewLine; // since it was null'ed in GetIdentifier
                    ln = s + 1;
                }
                else if (strncmp(s + 1, "ifndef", 6) == 0)
                {
                    char* name = GetIdentifier(s + 1 + 6, &s);
                    if (!name)
                        break;
                    bool condition = !evaluator.HasVariable(name);
                    condition_t p;

                    p.original_condition = condition;
                    p.effective_condition = condition;
                    p.do_skip_lines = !condition;
                    pending_elif.push_back(p);

                    *s = NewLine; // since it was null'ed in GetIdentifier
                    ln = s + 1;
                }
                else if (strncmp(s + 1, "if", 2) == 0)
                {
                    char* e = GetExpression(s + 1 + 2, &s);
                    float v = 0;

                    if (e == nullptr)
                        return false;

                    if (!evaluator.Evaluate(e, &v))
                    {
                        ReportExprEvalError(src_line_n);
                        return false;
                    }

                    *s = NewLine; // since it was null'ed in GetExpression
                    ln = s + 1;

                    bool condition = (v != 0.0f);

                    condition_t p;
                    p.original_condition = condition;
                    p.effective_condition = condition;
                    p.do_skip_lines = !condition;
                    pending_elif.push_back(p);
                }
                else if (strncmp(s + 1, "elif", 4) == 0)
                {
                    char* e = GetExpression(s + 1 + 4, &s);

                    if (e == nullptr)
                        return false;

                    float v = 0;
                    if (!evaluator.Evaluate(e, &v))
                    {
                        ReportExprEvalError(src_line_n);
                        return false;
                    }

                    *s = NewLine; // since it was null'ed in GetExpression
                    ln = s + 1;

                    bool condition = static_cast<int32>(v) != 0;

                    if (pending_elif.back().original_condition)
                        pending_elif.back().do_skip_lines = true;
                    else
                        pending_elif.back().do_skip_lines = !condition;

                    pending_elif.back().effective_condition = pending_elif.back().effective_condition || condition;

                    if (*s == Zero)
                    {
                        ln[0] = Zero;
                        break;
                    }
                }
                else if (strncmp(s + 1, "else", 4) == 0)
                {
                    pending_elif.back().do_skip_lines = pending_elif.back().effective_condition;

                    while (*s && *s != NewLine)
                        ++s;

                    if (*s == Zero)
                    {
                        ln[0] = Zero;
                        break;
                    }

                    ln = s + 1;
                }
                else if (strncmp(s + 1, "endif", 5) == 0)
                {
                    DVASSERT(pending_elif.size());
                    pending_elif.pop_back();

                    while (*s && *s != NewLine)
                        ++s;

                    if (*s == Zero)
                    {
                        ln[0] = Zero;
                        break;
                    }

                    ln = s + 1;
                }
                else if (!skipping_line)
                {
                    Logger::Error("Unknown preprocessor directive \"%s\"", s + 1);
                    break;
                }

                dcheck_pending = true;
            }
            else
            {
                if (*ns1 == NewLine)
                    s = ns1 - 1;

                dcheck_pending = false;
                expand_macros = (*ns1 != NewLine) && (!skipping_line);
            }
        }
        else
        {
            expand_macros = true;
        }

        if (expand_macros)
        {
            ln = ExpandMacroInLine(s);
            s = SeekToLineEnding(s) - 1; /* -1 added to compensate increment in loop ¯\_(ツ)_/¯ */
        }
    }

    if (ln[0] != Zero)
        lines.emplace_back(ln, line_n);

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessInplaceInternal(char* src_text, TextBuffer* output)
{
    bool success = false;

    LineVector lines;
    if (ProcessBuffer(src_text, lines))
    {
        GenerateOutput(output, lines);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessInclude(const char* file_name, LineVector& line_)
{
    bool success = false;

    if (fileCB->Open(file_name))
    {
        uint32 text_sz = fileCB->Size();
        char* text = AllocBuffer(text_sz + 1);
        fileCB->Read(text_sz, text);
        fileCB->Close();

        const char* prev_file_name = curFileName;

        curFileName = file_name;
        ProcessBuffer(text, line_);
        curFileName = prev_file_name;
        success = true;
    }
    else
    {
        Logger::Error("Failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessDefine(const char* name, const char* value)
{
    bool name_valid = IsValidAlphaChar(name[0]);

    for (const char* n = name; *n; ++n)
    {
        if (!IsValidAlphaNumericChar(*n))
        {
            name_valid = false;
            break;
        }
    }

    if (!name_valid)
    {
        Logger::Error("Invalid identifier \"%s\"", name);
        return false;
    }

    float val;

    if (evaluator.Evaluate(value, &val))
    {
        variable.emplace_back();
        strcpy(variable.back().name, name);
        variable.back().val = int(val);
        evaluator.SetVariable(name, val);
    }

    char value2[1024] = {};
    size_t value2_sz;

    strcpy(value2, value);
    value2_sz = strlen(value2) + 1;

    bool expanded = false;
    do
    {
        expanded = false;
        for (const Macro& m : macro)
        {
            char* t = strstr(value2, m.name);
            if (t)
            {
                size_t name_l = m.name_len;
                size_t val_l = m.value_len;
                memmove(t + val_l, t + name_l, value2_sz - (t - value2) - name_l);
                memcpy(t, m.value, val_l);
                value2_sz += val_l - name_l;
                expanded = true;
                break;
            }
        }
    }
    while (expanded);

    uint32 name_len = static_cast<uint32>(strlen(name));
    uint32 value_len = static_cast<uint32>(strlen(value2));
    macro.emplace(name, name_len, value2, value_len);
    minMacroLength = std::min(minMacroLength, value_len);

    return true;
}

char* PreProc::GetToken(char* str, ptrdiff_t strSize, const char* m, ptrdiff_t tokenSize)
{
    char* result = nullptr;
    char* position = strstr(str, m);
    if ((position != nullptr) && ((position - str) < strSize))
    {
        char* l = position;
        char* r = position;

        while ((l > str) && IsValidAlphaNumericChar(*(l - 1)))
            --l;

        while ((r - str < strSize) && IsValidAlphaNumericChar(*r))
            ++r;

        char ending = *r;

        *r = 0;
        if ((l >= str) && ((r - l) == tokenSize) && (strcmp(m, l) == 0))
            result = l;

        *r = ending;
    }
    return result;
}

char* PreProc::ExpandMacroInLine(char* txt)
{
    char* lineEnding = SeekToLineEnding(txt);
    ptrdiff_t sourceLength = lineEnding - txt;

    char* result = txt;

    bool expanded = false;
    do
    {
        expanded = false;
        for (const Macro& m : macro)
        {
            char* position = GetToken(result, sourceLength, m.name, m.name_len);
            if (position != nullptr)
            {
                uint32 requiredLength = static_cast<uint32>(sourceLength + m.value_len + 1);
                char* buffer = AllocBuffer(requiredLength);

                size_t offset = position - result;
                strncpy(buffer, result, offset);
                strncpy(buffer + offset, m.value, m.value_len);
                strncpy(buffer + offset + m.value_len, position + m.name_len, sourceLength - offset - m.name_len);

                result = buffer;
                sourceLength = requiredLength - 1;
                expanded = true;
            }
        }
    } while (expanded);

    return result;
}

//------------------------------------------------------------------------------

void PreProc::Undefine(const char* name)
{
    evaluator.RemoveVariable(name);
    for (auto m = macro.begin(), m_end = macro.end(); m != m_end; ++m)
    {
        if (strcmp(m->name, name) == 0)
        {
            macro.erase(m);
            break;
        }
    }
}

//------------------------------------------------------------------------------

void PreProc::GenerateOutput(TextBuffer* output, LineVector& lines)
{
    static const char* endl = "\r\n";
    static const int32 endl_sz = 2;

    output->clear();
    for (auto l = lines.begin(), l_end = lines.end(); l != l_end; ++l)
    {
        uint32 sz = uint32(strlen(l->text));

        output->insert(output->end(), l->text, l->text + sz);
        output->insert(output->end(), endl, endl + endl_sz);
    }
}

//------------------------------------------------------------------------------

void PreProc::ReportExprEvalError(uint32 line_n)
{
    char err[256] = {};
    evaluator.GetLastError(err, countof(err));
    Logger::Error("%s  : %u  %s", curFileName, line_n, err);
}
}
