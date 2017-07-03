#include "KeyboardShortcut.h"

#include "InputSystem.h"
#include "Utils/StringUtils.h"
#include "Utils/Utils.h"

namespace DAVA
{
KeyboardShortcut::KeyboardShortcut()
{
}

KeyboardShortcut::KeyboardShortcut(const KeyboardShortcut& shortcut)
    : key(shortcut.key)
    , modifiers(shortcut.modifiers)
{
}

KeyboardShortcut::KeyboardShortcut(eInputElements key_, eModifierKeys modifiers_)
    : key(key_)
    , modifiers(modifiers_)
{
}

KeyboardShortcut::KeyboardShortcut(const String& str)
{
    Vector<String> tokens;
    Split(str, "+", tokens);

    int modifiersPack = 0;
    for (const String& token : tokens)
    {
        String t = StringUtils::Trim(token);
        int curModifier = 0;
        if (GlobalEnumMap<eModifierKeys>::Instance()->ToValue(token.c_str(), curModifier))
        {
            modifiersPack |= curModifier;
        }
        else
        {
            DVASSERT(key == eInputElements::NONE);
            for (int i = eInputElements::KB_FIRST; i <= eInputElements::KB_LAST; ++i)
            {
                eInputElements currentKey = static_cast<eInputElements>(i);
                InputElementInfo keyInfo = GetInputElementInfo(static_cast<eInputElements>(i));
                if (keyInfo.name == t)
                {
                    key = currentKey;
                }
            }
        }
    }
    modifiers = static_cast<eModifierKeys>(modifiersPack) & eModifierKeys::MASK;

    DVASSERT(key != eInputElements::NONE);
}

KeyboardShortcut::~KeyboardShortcut() = default;

KeyboardShortcut& KeyboardShortcut::operator=(const KeyboardShortcut& shortcut)
{
    key = shortcut.key;
    modifiers = shortcut.modifiers;
    return *this;
}

bool KeyboardShortcut::operator==(const KeyboardShortcut& other) const
{
    return key == other.key && modifiers == other.modifiers;
}

bool KeyboardShortcut::operator!=(const KeyboardShortcut& other) const
{
    return !(operator==(other));
}

eInputElements KeyboardShortcut::GetKey() const
{
    return key;
}

eModifierKeys KeyboardShortcut::GetModifiers() const
{
    return modifiers;
}

String KeyboardShortcut::ToString() const
{
    StringStream stream;

    int test = static_cast<int>(eModifierKeys::FIRST);
    int last = static_cast<int>(eModifierKeys::LAST);
    int modifiersPack = static_cast<int>(modifiers);
    while (test <= last)
    {
        if (test & modifiersPack)
        {
            stream << GlobalEnumMap<eModifierKeys>::Instance()->ToString(test) << "+";
        }
        test <<= 1;
    }
    stream << GetInputElementInfo(key).name;

    return stream.str();
}
}
