#pragma once

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

namespace DAVA
{

#define PREPROCESSOR_COLLECT_INFO 0

struct PreprocessorTokenSet
{
    void RegisterConditionToken(const char* token)
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        currentNode->conditionTokens.emplace_back(token);
    #endif
    }

    void RegisterConditionToken(const char* token, uint32 tokenSize)
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        currentNode->conditionTokens.emplace_back(token, tokenSize);
    #endif
    }

    void RegisterTextSubstitutionToken(const char* token)
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        currentNode->textSubstitution.emplace_back(token);
    #endif
    }

    void RegisterTextSubstitutionToken(const char* token, uint32 tokenSize)
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        currentNode->textSubstitution.emplace_back(token, tokenSize);
    #endif
    }

    void PushNode()
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        currentNode->children.emplace_back();
        Node* newNode = &currentNode->children.back();
        newNode->parent = currentNode;
        currentNode = newNode;
    #endif
    }

    void PopNode()
    {
    #if (PREPROCESSOR_COLLECT_INFO)
        DVASSERT(currentNode->parent != nullptr);
        currentNode = currentNode->parent;
    #endif
    }

    struct Node
    {
        Node* parent = nullptr;
        Vector<String> conditionTokens;
        Vector<String> textSubstitution;
        Vector<Node> children;
    } root;

    Node* currentNode = &root;

    void PrintNode(const Node& node, const String& ident)
    {
        String tokens;

        Logger::Info("%s{", ident.c_str());
        if (node.textSubstitution.empty() == false)
        {
            tokens = "    #subsitutions:";
            for (const String& t : node.textSubstitution)
                tokens += " " + t;
            Logger::Info("%s%s", ident.c_str(), tokens.c_str());
        }

        if (node.conditionTokens.empty() == false)
        {
            tokens = "    #conditions:";
            for (const String& t : node.conditionTokens)
                tokens += " " + t;
            Logger::Info("%s%s", ident.c_str(), tokens.c_str());
        }

        for (const Node& c : node.children)
            PrintNode(c, ident + "    ");

        Logger::Info("%s}", ident.c_str());
    }

    void PrintInfo()
    {
        for (const Node& c : root.children)
            PrintNode(c, String());
    }
};

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

inline char* SkipCommentBlock(char* s, bool& unterminatedComment)
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

        unterminatedComment = (s[0] == 0) && !blockEndReached;
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

inline char* SeekToCharacter(char* s, char value)
{
    DVASSERT(s != nullptr);

    while ((*s != 0) && (*s != value))
        ++s;

    return s;
}

inline const char* SeekToCharacter(const char* s, char value)
{
    DVASSERT(s != nullptr);

    while ((*s != 0) && (*s != value))
        ++s;

    return s;
}

inline char* SeekToLineEnding(char* s)
{
    return SeekToCharacter(s, '\n');
}

inline const char* SeekToLineEnding(const char* s)
{
    return SeekToCharacter(s, '\n');
}
}
