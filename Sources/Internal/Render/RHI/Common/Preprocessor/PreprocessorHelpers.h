#pragma once

namespace PreprocessorHelpers
{
inline bool IsValidAlphaChar(char c)
{
    return (c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

inline bool IsValidDigitChar(char c)
{
    return (c >= '0') && (c <= '9');
}

inline bool IsValidAlphaNumericChar(char c)
{
    return IsValidAlphaChar(c) || IsValidDigitChar(c);
}

inline bool IsSpaceChar(char c)
{
    return (c == ' ') || (c == '\t');
}

inline char* SkipWhitespace(char* s)
{
    DVASSERT(s != nullptr);

    while ((*s != 0) && IsSpaceChar(*s))
        ++s;

    return s;
}

inline char* SkipCommentBlock(char* s)
{
    DVASSERT(s != nullptr);

    if ((s[0] == '/') && (s[1] == '*'))
    {
        s += 2;

        bool blockEndReached = false;
        while (!((s[0] == 0) || (blockEndReached = (s[0] == '*') && (s[1] == '/'))))
            ++s;

        if (blockEndReached)
            s += 2;
    }
    return s;
}

inline char* SkipCommentLine(char* s)
{
    DVASSERT(s != nullptr);

    if ((s[0] == '/') && (s[1] == '/'))
    {
        while (s[0] != '\n')
            ++s;
    }
    return s;
}
}
