#include "TArc/Utils/KeyboardProxy.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include "DeviceManager/DeviceManager.h"
#include <Input/InputSystem.h>
#include <Input/Keyboard.h>

namespace Utils
{
bool IsKeyPressed(DAVA::eModifierKeys modifier)
{
    using namespace DAVA;

    const Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard == nullptr)
    {
        return false;
    }

    switch (modifier)
    {
    case eModifierKeys::ALT:
        return keyboard->GetKeyState(eInputElements::KB_LALT).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RALT).IsPressed();
    case eModifierKeys::CONTROL:
#ifdef __DAVAENGINE_WINDOWS__
        return keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RCTRL).IsPressed();
#elif defined __DAVAENGINE_MACOS__
        return keyboard->GetKeyState(eInputElements::KB_LCMD).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RCMD).IsPressed();
#else
#error "non supported platform";
#endif //platform
    case eModifierKeys::SHIFT:
        return keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RSHIFT).IsPressed();
    default:
        DVASSERT(false, "unsupported key");
        return false;
    }
}
}
