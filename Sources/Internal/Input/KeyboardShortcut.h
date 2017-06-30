#pragma once

#include "Base/BaseTypes.h"
#include "Engine/EngineTypes.h"

#include "KeyboardDevice.h"

namespace DAVA
{
class KeyboardShortcut final
{
public:
    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut& shortcut);
    KeyboardShortcut(Key key_, eModifierKeys modifiers_ = eModifierKeys::NONE);
    KeyboardShortcut(const String& str);

    ~KeyboardShortcut();

    KeyboardShortcut& operator=(const KeyboardShortcut& shortcut);
    bool operator==(const KeyboardShortcut& other) const;
    bool operator!=(const KeyboardShortcut& other) const;

    Key GetKey() const;
    eModifierKeys GetModifiers() const;
    String ToString() const;

private:
    Key key = Key::UNKNOWN;
    eModifierKeys modifiers = eModifierKeys::NONE;
};
}

namespace std
{
template <>
struct hash<DAVA::KeyboardShortcut>
{
    size_t operator()(const DAVA::KeyboardShortcut& shortcut) const
    {
        return static_cast<size_t>(shortcut.GetKey()) | (static_cast<size_t>(shortcut.GetModifiers()) << 16);
    }
};
}
