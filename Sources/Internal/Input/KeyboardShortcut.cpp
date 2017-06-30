#include "KeyboardShortcut.h"

#include "KeyboardDevice.h"
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

KeyboardShortcut::KeyboardShortcut(Key key_, eModifierKeys modifiers_)
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
            DVASSERT(key == Key::UNKNOWN);
            key = InputSystem::Instance()->GetKeyboard().GetKeyByName(t);
        }
    }
    modifiers = static_cast<eModifierKeys>(modifiersPack) & eModifierKeys::MASK;

    DVASSERT(key != Key::UNKNOWN);
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

Key KeyboardShortcut::GetKey() const
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
    stream << InputSystem::Instance()->GetKeyboard().GetKeyName(key);

    return stream.str();
}
}
