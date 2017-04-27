#include "PreProcessor.h"

#include "Logger/Logger.h"
#include "Base/BaseTypes.h"
#include "rhi_Utils.h"

PreProc::DefFileCallback PreProc::_DefFileCallback;

//------------------------------------------------------------------------------

PreProc::PreProc(FileCallback* fc)
    : _file_cb((fc) ? fc : &_DefFileCallback)
    ,
    _cur_file_name("<buffer>")
    ,
    _min_macro_length(DAVA::InvalidIndex)
{
}

//------------------------------------------------------------------------------

PreProc::~PreProc()
{
    clear();
}

//------------------------------------------------------------------------------

bool
PreProc::process_file(const char* file_name, TextBuf* output)
{
    bool success = false;

    if (_file_cb->open(file_name))
    {
        _reset();

        unsigned text_sz = _file_cb->size();
        char* text = _alloc_buffer(text_sz + 1);

        _file_cb->read(text_sz, text);
        text[text_sz] = 0;
        _file_cb->close();

        _cur_file_name = "<buffer>";
        if (_process_buffer(text, &_line))
        {
            _generate_output(output);
            success = true;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

bool
PreProc::process_inplace(char* src_text, TextBuf* output)
{
    _reset();
    return _process_inplace(src_text, output);
}

//------------------------------------------------------------------------------

bool
PreProc::process(const char* src_text, TextBuf* output)
{
    _reset();

    bool success = false;
    char* text = _alloc_buffer(unsigned(strlen(src_text)) + 1);

    strcpy(text, src_text);

    if (_process_inplace(text, output))
    {
        _generate_output(output);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

void
PreProc::clear()
{
    _reset();
    _min_macro_length = DAVA::InvalidIndex;
    _macro.clear();
}

//------------------------------------------------------------------------------

bool
PreProc::add_define(const char* name, const char* value)
{
    return _process_define(name, value);
}

//------------------------------------------------------------------------------

void
PreProc::dump() const
{
    for (unsigned i = 0; i != _line.size(); ++i)
    {
        DAVA::Logger::Info("%04u | %s", _line[i].line_n, _line[i].text);
    }
}

//------------------------------------------------------------------------------

void
PreProc::_reset()
{
    _line.clear();

    for (unsigned b = 0; b != _buf.size(); ++b)
        ::free(_buf[b].mem);
    _buf.clear();

    _cur_file_name = "<buffer>";
}

//------------------------------------------------------------------------------

char*
PreProc::_alloc_buffer(unsigned sz)
{
    Buffer buf;

    buf.mem = ::malloc(sz);

    _buf.push_back(buf);
    return (char*)(buf.mem);
}

//------------------------------------------------------------------------------

inline char*
PreProc::_get_expression(char* txt, char** end) const
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

char*
PreProc::_get_identifier(char* txt, char** end) const
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

int
PreProc::_get_name_and_value(char* txt, char** name, char** value, char** end) const
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
    while (*t != ' ' && *t != '\t')
        ++t;
    if (*t == '\0')
        return 0;
    n1 = t - 1;

    while (*t == ' ' || *t == '\t')
        ++t;
    if (*t == '\0')
        return 0;
    v0 = t;
    while (*t != ' ' && *t != '\t' && *t != '\n' && *t != '\r')
        ++t;
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

bool
PreProc::_process_buffer(char* text, std::vector<Line>* line)
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

        for (int i = pending_elif.size() - 1; i >= 0; --i)
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

                while (*ns1 && *ns1 == ' ' || *ns1 == '\t')
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
            line->push_back(Line(ln, line_n));

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

            while (*ns1 && *ns1 == ' ' || *ns1 == '\t')
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
                    if (!_process_include(fname, line))
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
                    int nv = _get_name_and_value(s + 1 + 6, &name, &value, &s);

                    if (nv)
                    {
                        _process_define(name, value);

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
                    int nv = _get_name_and_value(s + 1 + 13, &name, &value, &s);

                    if (nv)
                    {
                        if (!_eval.has_variable(name))
                            _eval.set_variable(name, float(atof(value)));

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
                else if (strncmp(s + 1, "ifdef", 5) == 0)
                {
                    char* name = _get_identifier(s + 1 + 5, &s);
                    if (!name)
                        break;
                    bool condition = _eval.has_variable(name);
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
                    char* name = _get_identifier(s + 1 + 6, &s);
                    if (!name)
                        break;
                    bool condition = !_eval.has_variable(name);
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
                    char* e = _get_expression(s + 1 + 2, &s);
                    float v = 0;

                    if (!_eval.evaluate(e, &v))
                    {
                        _report_expr_eval_error(src_line_n);
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
                    char* e = _get_expression(s + 1 + 4, &s);
                    float v = 0;

                    if (!_eval.evaluate(e, &v))
                    {
                        _report_expr_eval_error(src_line_n);
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
                        DAVA::Logger::Warning("ignoring unknown pre-processor directive \"%s\"", s + 1);
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

            for (unsigned m = 0; m != _macro.size(); ++m)
            {
                char* t = strstr(s, _macro[m].name);

                if (t)
                {
                    size_t sz = ln_end - ln;
                    char* l = _alloc_buffer(sz + _macro[m].value_len + 1);

                    size_t l1 = t - ln;

                    strncpy(l, ln, l1);
                    strncpy(l + l1, _macro[m].value, _macro[m].value_len);
                    strncpy(l + l1 + _macro[m].value_len, t + _macro[m].name_len, sz - (l1 + _macro[m].name_len));
                    l[l1 + _macro[m].value_len + sz - (l1 + _macro[m].name_len)] = '\n';
                    l[l1 + _macro[m].value_len + sz - (l1 + _macro[m].name_len) + 1] = '\0';

                    ln = l;
                    s = l + l1 + _macro[m].value_len;

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
                && _min_macro_length != DAVA::InvalidIndex
                && s + _min_macro_length < ln_end
                )
            {
                s += _min_macro_length - 1;
            }
        }
    }

    if (ln[0])
        line->push_back(Line(ln, line_n));

    return success;
}

//------------------------------------------------------------------------------

bool
PreProc::_process_inplace(char* src_text, TextBuf* output)
{
    bool success = false;

    if (_process_buffer(src_text, &_line))
    {
        _generate_output(output);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool
PreProc::_process_include(const char* file_name, std::vector<PreProc::Line>* line)
{
    bool success = true;

    if (_file_cb->open(file_name))
    {
        unsigned text_sz = _file_cb->size();
        char* text = _alloc_buffer(text_sz + 1);

        _file_cb->read(text_sz, text);
        text[text_sz] = 0;
        _file_cb->close();

        const char* prev_file_name = _cur_file_name;

        _cur_file_name = file_name;
        _process_buffer(text, &_line);
        _cur_file_name = prev_file_name;
    }
    else
    {
        DAVA::Logger::Error("failed to open \"%s\"\n", file_name);
    }

    return success;
}

//------------------------------------------------------------------------------

bool
PreProc::_process_define(const char* name, const char* value)
{
    bool success = false;
    float val;

    if (_eval.evaluate(value, &val))
    {
        _var.push_back(Var());
        strcpy(_var.back().name, name);
        _var.back().val = int(val);

        _eval.set_variable(name, val);
    }
    else
    {
        //        _report_expr_eval_error(0);
    }

    _macro.resize(_macro.size() + 1);
    strncpy(_macro.back().name, name, countof(_macro.back().name));
    strncpy(_macro.back().value, value, countof(_macro.back().value));
    _macro.back().name_len = strlen(name);
    _macro.back().value_len = strlen(value);
    if (_macro.back().value_len < _min_macro_length)
        _min_macro_length = _macro.back().value_len;

    return success;
}

//------------------------------------------------------------------------------

void
PreProc::_generate_output(TextBuf* output)
{
    static const char* endl = "\r\n";

    output->clear();
    for (std::vector<Line>::const_iterator l = _line.begin(), l_end = _line.end(); l != l_end; ++l)
    {
        unsigned sz = unsigned(strlen(l->text));

        output->insert(output->end(), l->text, l->text + sz);
        output->insert(output->end(), endl, endl + 2);
    }
}

//------------------------------------------------------------------------------

void
PreProc::_report_expr_eval_error(unsigned line_n)
{
    char err[256];

    _eval.get_last_error(err, countof(err));
    DAVA::Logger::Error("%s  : %u  %s", _cur_file_name, line_n, err);
}
