#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/RefPtr.h"
#include "Engine/EngineTypes.h"
#include "Functional/Function.h"
#include "Input/InputEvent.h"

namespace DAVA
{
class Engine;
class UIEvent;
namespace Private
{
class EngineBackend;
}

/**
    \defgroup input Input

    Input system is a part of the engine which is responsible for:
    - Managing input devices (keyboards, mouses, gamepads etc.)
    - Handling input events sent by a platform
    - Storing each input device's state
*/
class InputSystem final
{
    friend class Window;
    friend class Private::EngineBackend;

public:
    // Temporal method for backward compatibility
    // TODO: remove InputSystem::Instance() method
    static InputSystem* Instance();

    uint32 AddHandler(eInputDeviceTypes inputDeviceMask, const Function<bool(const InputEvent&)>& handler);
    void ChangeHandlerDeviceMask(uint32 token, eInputDeviceTypes newInputDeviceMask);
    void RemoveHandler(uint32 token);

    void DispatchInputEvent(const InputEvent& inputEvent);

private:
    InputSystem(Engine* engine);
    ~InputSystem();

    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    void EndFrame();
    void HandleInputEvent(UIEvent* uie);

private:
    struct InputHandler
    {
        InputHandler(uint32 aToken, eInputDeviceTypes inputDeviceMask, const Function<bool(UIEvent*)>& handler);
        InputHandler(uint32 aToken, eInputDeviceTypes inputDeviceMask, const Function<bool(const InputEvent&)>& handler);

        uint32 token;
        bool useRawInputCallback;
        eInputDeviceTypes deviceMask;
        Function<bool(UIEvent*)> uiEventHandler;
        Function<bool(const InputEvent&)> rawInputHandler;
    };

    Vector<InputHandler> handlers;
    uint32 nextHandlerToken = 1;
    bool pendingHandlerRemoval = false;
};

inline InputSystem::InputHandler::InputHandler(uint32 aToken, eInputDeviceTypes inputDeviceMask, const Function<bool(UIEvent*)>& handler)
    : token(aToken)
    , useRawInputCallback(false)
    , deviceMask(inputDeviceMask)
    , uiEventHandler(handler)
{
}

inline InputSystem::InputHandler::InputHandler(uint32 aToken, eInputDeviceTypes inputDeviceMask, const Function<bool(const InputEvent&)>& handler)
    : token(aToken)
    , useRawInputCallback(true)
    , deviceMask(inputDeviceMask)
    , rawInputHandler(handler)
{
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "InputCallback.h"
#include "Input/MouseDevice.h"

namespace DAVA
{
class KeyboardDevice;
class GamepadDevice;

class InputSystem : public Singleton<InputSystem>
{
public:
    enum eInputDevice
    {
        INPUT_DEVICE_TOUCH = 1,
        INPUT_DEVICE_KEYBOARD = 1 << 1,
        INPUT_DEVICE_JOYSTICK = 1 << 2
    };

#if defined(__DAVAENGINE_COREV2__)
    friend class Private::EngineBackend;
#else
    friend void Core::CreateSingletons();
#endif

protected:
    ~InputSystem();
    /**
     \brief Don't call this constructor!
     */
    InputSystem();

public:
    void ProcessInputEvent(UIEvent* event);

    void AddInputCallback(const InputCallback& inputCallback);
    bool RemoveInputCallback(const InputCallback& inputCallback);
    void RemoveAllInputCallbacks();

    void OnBeforeUpdate();
    void OnAfterUpdate();

    inline KeyboardDevice& GetKeyboard();
    inline GamepadDevice& GetGamepadDevice();
    inline MouseDevice& GetMouseDevice();

    inline void EnableMultitouch(bool enabled);
    inline bool GetMultitouchEnabled() const;

protected:
    KeyboardDevice* keyboard;
    GamepadDevice* gamepad;
    MouseDevice* mouse;

    Vector<InputCallback> callbacks;
    bool pinCursor;

    bool isMultitouchEnabled;
};

inline KeyboardDevice& InputSystem::GetKeyboard()
{
    return *keyboard;
}

inline GamepadDevice& InputSystem::GetGamepadDevice()
{
    return *gamepad;
}

inline MouseDevice& InputSystem::GetMouseDevice()
{
    return *mouse;
}

inline void InputSystem::EnableMultitouch(bool enabled)
{
    isMultitouchEnabled = enabled;
}

inline bool InputSystem::GetMultitouchEnabled() const
{
    return isMultitouchEnabled;
}
};

#endif // !__DAVAENGINE_COREV2__
