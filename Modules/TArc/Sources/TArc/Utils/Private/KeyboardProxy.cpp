#include "TArc/Utils/KeyboardProxy.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/InputSystem.h>
#include <Input/KeyboardDevice.h>

namespace Utils
{
bool IsKeyPressed(DAVA::eModifierKeys modifier)
{
    using namespace DAVA;
    const KeyboardDevice& keyboard = GetEngineContext()->inputSystem->GetKeyboard();
    switch (modifier)
    {
    case eModifierKeys::ALT:
        return keyboard.IsKeyPressed(Key::LALT) || keyboard.IsKeyPressed(Key::RALT);
    case eModifierKeys::CONTROL:
#ifdef __DAVAENGINE_WINDOWS__
        return keyboard.IsKeyPressed(Key::LCTRL) || keyboard.IsKeyPressed(Key::RCTRL);
#elif defined __DAVAENGINE_MACOS__
        return keyboard.IsKeyPressed(Key::LCMD) || keyboard.IsKeyPressed(Key::LCMD);
#else
#error "non supported platform";
#endif //platform
    case eModifierKeys::SHIFT:
        return keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT);
    default:
        DVASSERT(false, "unsupported key");
        return false;
    }
}
}
