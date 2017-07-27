#include "Render/RHI/Common/PreProcessor.h"
#include "Render/RHI/Common/Preprocessor/PreprocessorHelpers.h"
#include "Logger/Logger.h"
#include "FileSystem/File.h"

namespace PreprocessorHelpers
{
struct DefaultFileCallback : public PreProc::FileCallback
{
    DAVA::ScopedPtr<DAVA::File> in;

    bool Open(const char* file_name) override
    {
        in.reset(DAVA::File::Create(file_name, DAVA::File::READ | DAVA::File::OPEN));
        return (in.get() != nullptr);
    }
    void Close() override
    {
        DVASSERT(in.get() != nullptr);
        in.reset(nullptr);
    }
    uint32_t Size() const override
    {
        return (in.get() != nullptr) ? static_cast<uint32_t>(in->GetSize()) : 0;
    }
    uint32_t Read(uint32_t max_sz, void* dst) override
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

        uint32_t text_sz = fileCB->Size();
        char* text = AllocBuffer(text_sz + 1);
        fileCB->Read(text_sz, text);
        fileCB->Close();

        success = ProcessInplaceInternal(text, output);
    }
    else
    {
        DAVA::Logger::Error("Failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::Process(const char* src_text, TextBuffer* output)
{
    Reset();

    char* text = AllocBuffer(uint32_t(strlen(src_text)) + 1);
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
    for (uint32_t i = 0; i != line.size(); ++i)
    {
        DAVA::Logger::Info("%04u | %s", line[i].line_n, line[i].text);
    }
    */
}

//------------------------------------------------------------------------------

void PreProc::Reset()
{
    for (uint32_t b = 0; b != buffer.size(); ++b)
        ::free(buffer[b].mem);
    buffer.clear();

    curFileName = "<buffer>";
}

//------------------------------------------------------------------------------

char* PreProc::AllocBuffer(uint32_t sz)
{
    Buffer buf;
    buf.mem = ::calloc(1, sz);
    buffer.push_back(buf);
    return reinterpret_cast<char*>(buf.mem);
}

//------------------------------------------------------------------------------

inline char* PreProc::GetExpression(char* txt, char** end) const
{
    char* e = txt;
    char* s = txt;
    bool cmt = false;

    while (*s != NewLine && *s != CarriageReturn)
    {
        if (*s == 0)
            return nullptr;

        if (s[0] == '/')
        {
            if (s[1] == '/')
            {
                *s = Zero;
            }
            else if (s[1] == '*')
            {
                DVASSERT(!cmt);
                cmt = true;
            }
        }
        else if (s[0] == '*' && s[1] == '/')
        {
            DVASSERT(cmt);
            cmt = false;
            s[0] = Space;
            s[1] = Space;
        }

        if (cmt)
            *s = Space;
        ++s;
    }
    DVASSERT(*s);
    if (*s == CarriageReturn)
    {
        *s = Zero;
        ++s;
    }

    while (*s != NewLine)
    {
        if (*s == 0)
            return nullptr;

        ++s;
    }

    *end = s;
    return e;
}

//------------------------------------------------------------------------------

char* PreProc::GetIdentifier(char* txt, char** end) const
{
    char* t = txt;
    char* n = nullptr;
    bool cmt = false;

    while (!cmt && !PreprocessorHelpers::IsValidAlphaNumericChar(*t))
    {
        if (*t == Zero)
            return nullptr;

        if (t[0] == '/' && t[1] == '*')
            cmt = true;
        else if (t[0] == '*' && t[1] == '/')
            cmt = false;

        ++t;
    }
    if (*t == Zero)
        return nullptr;

    n = t;

    while (PreprocessorHelpers::IsValidAlphaNumericChar(*t))
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

int32_t PreProc::GetNameAndValue(char* txt, char** name, char** value, char** end) const
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
    while (*t && *t != Space && *t != Tab && *t != NewLine && *t != CarriageReturn)
    {
        if (t[0] == '/' && t[1] == '/')
            return 0;

        ++t;
    }

    if (*t == Zero)
        return 0;

    n1 = t - 1;
    while (*t == Space || *t == Tab)
    {
        if (t[0] == '/' && t[1] == '/')
            return 0;

        ++t;
    }

    if (*t == Zero)
        return 0;

    v0 = t;
    int32_t brace_lev = 0;
    while (*t && (*t != Space || brace_lev > 0) && *t != Tab && *t != NewLine && *t != CarriageReturn)
    {
        if (t[0] == '/' && t[1] == '/')
        {
            t[0] = Zero;
            ++t;
            break;
        }

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
    uint32_t inputTextLength = static_cast<uint32_t>(strlen(inputText));
    char* filteredText = AllocBuffer(inputTextLength + 1);
    char* filteredTextPtr = filteredText;
    for (char* s = inputText; *s != Zero; ++s)
    {
        s = PreprocessorHelpers::SkipCommentBlock(s);
        s = PreprocessorHelpers::SkipCommentLine(s);

        char currentChar = *s;
        if (currentChar != CarriageReturn)
        {
            *filteredTextPtr++ = currentChar;
            if (currentChar == '#')
                s = PreprocessorHelpers::SkipWhitespace(s + 1) - 1;
        }
    }

    bool success = true;
    char* ln = filteredText;
    bool ln_external = false;
    char* last_s = nullptr;
    uint32_t line_n = 1;
    uint32_t src_line_n = 1;

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
        int32_t skipping_line = false;

        for (int32_t i = int(pending_elif.size()) - 1; i >= 0; --i)
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
            char* ns1 = PreprocessorHelpers::SkipWhitespace(s);

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
                    int32_t nv = GetNameAndValue(s + 1 + 6, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == Zero)
                        {
                            DAVA::Logger::Error("#define without value not allowed (%s)", name);
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
                            *s = NewLine; // since it was null'ed in _get_name_and_value
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
                    int32_t nv = GetNameAndValue(s + 1 + 13, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == Zero)
                        {
                            DAVA::Logger::Error("#define without value not allowed (%s)", name);
                            success = false;
                            break;
                        }

                        if (!evaluator.HasVariable(name))
                            evaluator.SetVariable(name, float(atof(value)));

                        if (nv != -1)
                        {
                            *s = NewLine; // since it was null'ed in _get_name_and_value
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

                    *s = NewLine; // since it was null'ed in _get_identifier
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

                    *s = NewLine; // since it was null'ed in _get_identifier
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

                    *s = NewLine; // since it was null'ed in _get_identifier
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

                    ln = s + 1;

                    bool condition = static_cast<int32_t>(v) != 0;

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

                    ln = s + 1;

                    bool condition = static_cast<int32_t>(v) != 0;

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
                    DAVA::Logger::Error("Unknown preprocessor directive \"%s\"", s + 1);
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
            char* ln_end = s;
            bool macro_expanded = false;
            bool restore_nl = false;

            while (*ln_end && *ln_end != NewLine)
                ++ln_end;

            if (*ln_end == NewLine)
            {
                *ln_end = Zero;
                restore_nl = true;
            }

            for (size_t m = 0; m != macro.size(); ++m)
            {
                char* t = strstr(s, macro[m].name);

                if (t != nullptr)
                {
                    size_t sz = ln_end - ln;
                    char* l = AllocBuffer(uint32_t(sz) + macro[m].value_len + 1);

                    size_t l1 = t - ln;
                    size_t l2 = l1 + macro[m].value_len + sz - (l1 + macro[m].name_len);

                    strncpy(l, ln, l1);
                    strncpy(l + l1, macro[m].value, macro[m].value_len);
                    strncpy(l + l1 + macro[m].value_len, t + macro[m].name_len, sz - (l1 + macro[m].name_len));
                    l[l2] = NewLine;
                    l[l2 + 1] = Zero;
                    ln = l;
                    s = l + l1 + macro[m].value_len;

                    if (*s == CarriageReturn)
                    {
                        if (s[1] == NewLine)
                            *s++ = Zero;
                        else
                            *s = Space;
                    }

                    if (!ln_external)
                    {
                        last_s = ln_end;
                        ln_external = true;
                    }

                    macro_expanded = true;
                    --s;
                    break;
                }
            }

            if (restore_nl)
                *ln_end = NewLine;

            if (!macro_expanded && (minMacroLength != InvalidValue) && (s + minMacroLength < ln_end))
            {
                s += minMacroLength - 1;
            }
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
        uint32_t text_sz = fileCB->Size();
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
        DAVA::Logger::Error("Failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessDefine(const char* name, const char* value)
{
    bool name_valid = PreprocessorHelpers::IsValidAlphaChar(name[0]);

    for (const char* n = name; *n; ++n)
    {
        if (!PreprocessorHelpers::IsValidAlphaNumericChar(*n))
        {
            name_valid = false;
            break;
        }
    }

    if (!name_valid)
    {
        DAVA::Logger::Error("Invalid identifier \"%s\"", name);
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

    char value2[1024];
    size_t value2_sz;

    strcpy(value2, value);
    value2_sz = strlen(value2) + 1;

    bool expanded = false;
    do
    {
        expanded = false;
        for (uint32_t m = 0; m != macro.size(); ++m)
        {
            char* t = strstr(value2, macro[m].name);

            if (t)
            {
                size_t name_l = macro[m].name_len;
                size_t val_l = macro[m].value_len;

                memmove(t + val_l, t + name_l, value2_sz - (t - value2) - name_l);
                memcpy(t, macro[m].value, val_l);
                value2_sz += val_l - name_l;
                expanded = true;
                break;
            }
        }
    }
    while (expanded);

    macro.resize(macro.size() + 1);
    strncpy(macro.back().name, name, countof(macro.back().name));
    strncpy(macro.back().value, value2, countof(macro.back().value));
    macro.back().name_len = uint32_t(strlen(name));
    macro.back().value_len = uint32_t(strlen(value2));
    if (macro.back().value_len < minMacroLength)
        minMacroLength = macro.back().value_len;

    return true;
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
    #if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    static const char* endl = "\n";
    static const int32_t endl_sz = 1;
    #else
    static const char* endl = "\r\n";
    static const int32_t endl_sz = 2;
    #endif

    output->clear();
    for (auto l = lines.begin(), l_end = lines.end(); l != l_end; ++l)
    {
        uint32_t sz = uint32_t(strlen(l->text));

        output->insert(output->end(), l->text, l->text + sz);
        output->insert(output->end(), endl, endl + endl_sz);
    }
}

//------------------------------------------------------------------------------

void PreProc::ReportExprEvalError(uint32_t line_n)
{
    char err[256] = {};
    evaluator.GetLastError(err, countof(err));
    DAVA::Logger::Error("%s  : %u  %s", curFileName, line_n, err);
}
