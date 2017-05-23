#include "PreProcessor.h"

#include "Logger/Logger.h"
#include "Base/BaseTypes.h"
#include "rhi_Utils.h"

PreProc::DefaultFileCallback PreProc::DefFileCallback;

//------------------------------------------------------------------------------

PreProc::PreProc(FileCallback* fc)
    : fileCB((fc) ? fc : &DefFileCallback)
    ,
    curFileName("<buffer>")
    ,
    minMacroLength(DAVA::InvalidIndex)
{
}

//------------------------------------------------------------------------------

PreProc::~PreProc()
{
    Clear();
}

//------------------------------------------------------------------------------

bool PreProc::ProcessFile(const char* file_name, TextBuf* output)
{
    bool success = false;

    if (fileCB->Open(file_name))
    {
        Reset();

        unsigned text_sz = fileCB->Size();
        char* text = AllocBuffer(text_sz + 1);

        fileCB->Read(text_sz, text);
        text[text_sz] = 0;
        fileCB->Close();

        curFileName = "<buffer>";
        if (ProcessBuffer(text, &line))
        {
            GenerateOutput(output);
            success = true;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessInplace(char* src_text, TextBuf* output)
{
    Reset();
    return ProcessInplaceInternal(src_text, output);
}

//------------------------------------------------------------------------------

bool PreProc::Process(const char* src_text, TextBuf* output)
{
    Reset();

    bool success = false;
    char* text = AllocBuffer(unsigned(strlen(src_text)) + 1);

    strcpy(text, src_text);

    if (ProcessInplaceInternal(text, output))
    {
        GenerateOutput(output);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

void PreProc::Clear()
{
    Reset();
    minMacroLength = DAVA::InvalidIndex;
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
    for (unsigned i = 0; i != line.size(); ++i)
    {
        DAVA::Logger::Info("%04u | %s", line[i].line_n, line[i].text);
    }
}

//------------------------------------------------------------------------------

void PreProc::Reset()
{
    line.clear();

    for (unsigned b = 0; b != buffer.size(); ++b)
        ::free(buffer[b].mem);
    buffer.clear();

    curFileName = "<buffer>";
}

//------------------------------------------------------------------------------

char* PreProc::AllocBuffer(unsigned sz)
{
    Buffer buf;

    buf.mem = ::malloc(sz);

    buffer.push_back(buf);
    return reinterpret_cast<char*>(buf.mem);
}

//------------------------------------------------------------------------------

inline char* PreProc::GetExpression(char* txt, char** end) const
{
    char* e = txt;
    char* s = txt;
    bool cmt = false;

    while (*s != '\n' && *s != '\r')
    {
        if (s[0] == '/')
        {
            if (s[1] == '/')
            {
                *s = 0;
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
            s[0] = ' ';
            s[1] = ' ';
        }

        if (cmt)
            *s = ' ';
        ++s;
    }
    DVASSERT(*s);
    if (*s == '\r')
        *s = '\0';

    while (*s != '\n')
        ++s;

    *end = s;
    return e;
}

//------------------------------------------------------------------------------

char* PreProc::GetIdentifier(char* txt, char** end) const
{
    char* t = txt;
    char* n = nullptr;
    bool cmt = false;

    while (!cmt && !(isalnum(*t) || *t == '_'))
    {
        if (t[0] == '/' && t[1] == '*')
            cmt = true;
        else if (t[0] == '*' && t[1] == '/')
            cmt = false;

        ++t;
    }
    if (*t == '\0')
        return nullptr;
    n = t;

    while (isalnum(*t) || *t == '_')
        ++t;
    if (*t == '\0')
        return nullptr;
    if (*t != '\n')
        *t = '\0';

    while (*t != '\n')
        ++t;

    *t = '\0';
    *end = t;
    return n;
}

//------------------------------------------------------------------------------

int PreProc::GetNameAndValue(char* txt, char** name, char** value, char** end) const
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

    while (*t == ' ' || *t == '\t')
        ++t;
    if (*t == '\0')
        return 0;
    n0 = t;
    while (*t != ' ' && *t != '\t' && *t != '\n' && *t != '\r')
    {
        if (t[0] == '/' && t[1] == '/')
            return 0;

        ++t;
    }
    if (*t == '\0')
        return 0;
    n1 = t - 1;

    while (*t == ' ' || *t == '\t')
    {
        if (t[0] == '/' && t[1] == '/')
            return 0;

        ++t;
    }
    if (*t == '\0')
        return 0;
    v0 = t;
    while (*t != ' ' && *t != '\t' && *t != '\n' && *t != '\r')
    {
        if (t[0] == '/' && t[1] == '/')
        {
            t[0] = '\0';
            ++t;
            break;
        }

        ++t;
    }
    if (*t == '\0')
        return 0;
    v1 = t - 1;

    *name = n0;
    *value = v0;

    if (*t != '\0')
    {
        if (*t != '\n')
            *t = '\0';

        while (*t != '\n')
            ++t;
        *t = '\0';
        *end = t;

        *(n1 + 1) = '\0';
        *(v1 + 1) = '\0';

        return 1;
    }
    else
    {
        *end = t;
        *(n1 + 1) = '\0';
        *(v1 + 1) = '\0';
        return -1;
    }
}

//------------------------------------------------------------------------------

bool PreProc::ProcessBuffer(char* text, std::vector<Line>* line_)
{
    bool success = true;
    char* ln = text;
    bool ln_external = false;
    char* last_s = nullptr;
    unsigned line_n = 1;
    unsigned src_line_n = 1;

    struct
    condition_t
    {
        bool original_condition;
        bool effective_condition;
        bool do_skip_lines;
    };

    std::vector<condition_t> pending_elif;
    bool dcheck_pending = true;
    bool cmt_block = false;

    pending_elif.resize(1);
    pending_elif.back().do_skip_lines = false;

    for (char* s = text; *s; ++s)
    {
        if (*s == '\r')
        {
            if (s[1] == '\n')
                *s++ = 0;
            else
                *s = ' ';
        }

        if (s[0] == '/' && s[1] == '*')
            cmt_block = true;
        else if (s[0] == '*' && s[1] == '/')
            cmt_block = false;

        if (s[0] == '/' && s[1] == '/')
        {
            char* end = s;

            while (*s && *s != '\n')
                ++s;
            if (*s == 0)
                return success;

            *end = 0;
        }

        int skipping_line = false;

        for (int i = int(pending_elif.size()) - 1; i >= 0; --i)
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

                while (*ns1 && (*ns1 == ' ' || *ns1 == '\t'))
                    ++ns1;
                if (*ns1 == 0)
                {
                    success = false;
                    break;
                }

                if (*ns1 == '#')
                    do_skip = false;
            }

            if (do_skip)
            {
                while (*s != '\n')
                    ++s;

                if (*s == 0)
                    break;
                else
                    ln = s = s + 1;

                ++src_line_n;
                dcheck_pending = true;
            }
        }

        if (*s == '\n')
        {
            *s = 0;
            line_->push_back(Line(ln, line_n));

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
        else if (dcheck_pending && !cmt_block)
        {
            char* ns1 = s;

            while (*ns1 && (*ns1 == ' ' || *ns1 == '\t'))
                ++ns1;
            if (*ns1 == 0)
            {
                success = false;
                break;
            }

            if (*ns1 == '#')
            {
                s = ns1;

                DVASSERT(s[1]);
                if (!skipping_line && strncmp(s + 1, "include", 7) == 0)
                {
                    char* t = s;
                    char* f0 = nullptr;
                    char* f1 = nullptr;

                    while (*t != '\"')
                        ++t;
                    DVASSERT(*t);
                    f0 = t + 1;
                    ++t;
                    while (*t != '\"')
                        ++t;
                    DVASSERT(*t);
                    f1 = t - 1;

                    char fname[256];

                    strncpy(fname, f0, f1 - f0 + 1);
                    fname[f1 - f0 + 1] = 0;
                    if (!ProcessInclude(fname, line_))
                    {
                        return false;
                    }

                    while (*t != '\n')
                        ++t;
                    if (*t == 0)
                    {
                        break;
                    }
                    else
                    {
                        s = t;
                        ln = t + 1;
                    }
                }
                else if (!skipping_line && strncmp(s + 1, "define", 6) == 0)
                {
                    char* name = nullptr;
                    char* value = nullptr;
                    int nv = GetNameAndValue(s + 1 + 6, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == '\0')
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
                            *s = '\n'; // since it was null'ed in _get_name_and_value
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
                    int nv = GetNameAndValue(s + 1 + 13, &name, &value, &s);

                    if (nv)
                    {
                        if (value[0] == '\0')
                        {
                            DAVA::Logger::Error("#define without value not allowed (%s)", name);
                            success = false;
                            break;
                        }

                        if (!evaluator.HasVariable(name))
                            evaluator.SetVariable(name, float(atof(value)));

                        if (nv != -1)
                        {
                            *s = '\n'; // since it was null'ed in _get_name_and_value
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

                    *s = '\n'; // since it was null'ed in _get_identifier
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

                    *s = '\n'; // since it was null'ed in _get_identifier
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

                    *s = '\n'; // since it was null'ed in _get_identifier
                    ln = s + 1;
                }
                else if (strncmp(s + 1, "if", 2) == 0)
                {
                    char* e = GetExpression(s + 1 + 2, &s);
                    float v = 0;

                    if (!evaluator.Evaluate(e, &v))
                    {
                        ReportExprEvalError(src_line_n);
                        return false;
                    }

                    ln = s + 1;

                    bool condition = (int(v)) ? true : false;
                    condition_t p;
                    p.original_condition = condition;
                    p.effective_condition = condition;
                    p.do_skip_lines = !condition;
                    pending_elif.push_back(p);
                }
                else if (strncmp(s + 1, "elif", 4) == 0)
                {
                    char* e = GetExpression(s + 1 + 4, &s);
                    float v = 0;

                    if (!evaluator.Evaluate(e, &v))
                    {
                        ReportExprEvalError(src_line_n);
                        return false;
                    }

                    ln = s + 1;

                    bool condition = (int(v)) ? true : false;
                    if (pending_elif.back().original_condition)
                        pending_elif.back().do_skip_lines = true;
                    else
                        pending_elif.back().do_skip_lines = !condition;
                    pending_elif.back().effective_condition = pending_elif.back().effective_condition || condition;

                    if (*s == 0)
                    {
                        ln[0] = 0;
                        break;
                    }
                }
                else if (strncmp(s + 1, "else", 4) == 0)
                {
                    pending_elif.back().do_skip_lines = pending_elif.back().effective_condition;

                    while (*s != '\n')
                        ++s;
                    if (*s == 0)
                    {
                        ln[0] = 0;
                        break;
                    }
                    else
                        ln = s + 1;
                }
                else if (strncmp(s + 1, "endif", 5) == 0)
                {
                    pending_elif.pop_back();
                    DVASSERT(pending_elif.size());

                    while (*s && *s != '\n')
                        ++s;
                    if (*s == 0)
                    {
                        ln[0] = 0;
                        break;
                    }
                    else
                        ln = s + 1;
                }
                else
                {
                    if (!skipping_line)
                    {
                        char* ns1 = s;

                        while (*ns1 && *ns1 != ' ')
                            ++ns1;

                        *ns1 = 0;
                        DAVA::Logger::Error("unknown pre-processor directive \"%s\"", s + 1);
                        break;
                    }
                }

                dcheck_pending = true;
            }
            else
            {
                if (*ns1 == '\n')
                    s = ns1 - 1;

                dcheck_pending = false;
            }
        }
        else
        {
            // expand macros, if any
            char* ln_end = s;
            bool macro_expanded = false;
            bool restore_nl = false;

            while (*ln_end && *ln_end != '\n')
                ++ln_end;

            if (*ln_end == '\n')
            {
                *ln_end = '\0';
                restore_nl = true;
            }

            for (unsigned m = 0; m != macro.size(); ++m)
            {
                char* t = strstr(s, macro[m].name);

                if (t)
                {
                    size_t sz = ln_end - ln;
                    char* l = AllocBuffer(unsigned(sz) + macro[m].value_len + 1);

                    size_t l1 = t - ln;

                    strncpy(l, ln, l1);
                    strncpy(l + l1, macro[m].value, macro[m].value_len);
                    strncpy(l + l1 + macro[m].value_len, t + macro[m].name_len, sz - (l1 + macro[m].name_len));
                    l[l1 + macro[m].value_len + sz - (l1 + macro[m].name_len)] = '\n';
                    l[l1 + macro[m].value_len + sz - (l1 + macro[m].name_len) + 1] = '\0';

                    ln = l;
                    s = l + l1 + macro[m].value_len;

                    if (!ln_external)
                    {
                        last_s = ln_end;
                        ln_external = true;
                    }

                    macro_expanded = true;
                    break;
                }
            }

            if (restore_nl)
                *ln_end = '\n';

            if (!macro_expanded
                && minMacroLength != DAVA::InvalidIndex
                && s + minMacroLength < ln_end
                )
            {
                s += minMacroLength - 1;
            }
        }
    }

    if (ln[0])
        line_->push_back(Line(ln, line_n));

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessInplaceInternal(char* src_text, TextBuf* output)
{
    bool success = false;

    if (ProcessBuffer(src_text, &line))
    {
        GenerateOutput(output);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessInclude(const char* file_name, std::vector<PreProc::Line>* line_)
{
    bool success = false;

    if (fileCB->Open(file_name))
    {
        unsigned text_sz = fileCB->Size();
        char* text = AllocBuffer(text_sz + 1);

        fileCB->Read(text_sz, text);
        text[text_sz] = 0;
        fileCB->Close();

        const char* prev_file_name = curFileName;

        curFileName = file_name;
        ProcessBuffer(text, &line);
        curFileName = prev_file_name;
        success = true;
    }
    else
    {
        DAVA::Logger::Error("failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool PreProc::ProcessDefine(const char* name, const char* value)
{
    bool name_valid = true;

    if (!(isalpha(name[0]) || name[0] == '_'))
        name_valid = false;
    for (const char* n = name; *n; ++n)
    {
        if (!(isalnum(name[0]) || name[0] == '_'))
        {
            name_valid = false;
            break;
        }
    }

    if (!name_valid)
    {
        DAVA::Logger::Error("invalid identifier \"%s\"", name);
        return false;
    }

    float val;

    if (evaluator.Evaluate(value, &val))
    {
        variable.push_back(Var());
        strcpy(variable.back().name, name);
        variable.back().val = int(val);

        evaluator.SetVariable(name, val);
    }
    else
    {
        //        _report_expr_eval_error(0);
    }

    macro.resize(macro.size() + 1);
    strncpy(macro.back().name, name, countof(macro.back().name));
    strncpy(macro.back().value, value, countof(macro.back().value));
    macro.back().name_len = unsigned(strlen(name));
    macro.back().value_len = unsigned(strlen(value));
    if (macro.back().value_len < minMacroLength)
        minMacroLength = macro.back().value_len;

    return true;
}

//------------------------------------------------------------------------------

void PreProc::Undefine(const char* name)
{
    evaluator.RemoveVariable(name);
    for (std::vector<macro_t>::iterator m = macro.begin(), m_end = macro.end(); m != m_end; ++m)
    {
        if (strcmp(m->name, name) == 0)
        {
            macro.erase(m);
            break;
        }
    }
}

//------------------------------------------------------------------------------

void PreProc::GenerateOutput(TextBuf* output)
{
    #if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    static const char* endl = "\n";
    #else
    static const char* endl = "\r\n";
    #endif

    output->clear();
    for (std::vector<Line>::const_iterator l = line.begin(), l_end = line.end(); l != l_end; ++l)
    {
        unsigned sz = unsigned(strlen(l->text));

        output->insert(output->end(), l->text, l->text + sz);
        output->insert(output->end(), endl, endl + 2);
    }
}

//------------------------------------------------------------------------------

void PreProc::ReportExprEvalError(unsigned line_n)
{
    char err[256];

    evaluator.GetLastError(err, countof(err));
    DAVA::Logger::Error("%s  : %u  %s", curFileName, line_n, err);
}
