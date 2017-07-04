#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Engine/EngineTypes.h"
#include "Functional/Function.h"

namespace DAVA
{
/**
    \defgroup input Input System
*/

class Engine;
class UIEvent;
class KeyboardDevice;
class GamepadDevice;
namespace Private
{
class EngineBackend;
struct MainDispatcherEvent;
}

class InputSystem final
{
    friend class Window;
    friend class Private::EngineBackend;
    friend class GamepadDevice;

public:
    // Temporal method for backward compatibility
    // TODO: remove InputSystem::Instance() method
    static InputSystem* Instance();

    uint32 AddHandler(eInputDevices inputDeviceMask, const Function<bool(UIEvent*)>& callback);
    void ChangeHandlerDeviceMask(uint32 token, eInputDevices newInputDeviceMask);
    void RemoveHandler(uint32 token);

    KeyboardDevice& GetKeyboard();
    GamepadDevice& GetGamepadDevice();

private:
    InputSystem(Engine* engine);
    ~InputSystem();

    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    void Update(float32 frameDelta);
    void OnAfterUpdate();
    void HandleInputEvent(UIEvent* uie);
    void HandleGamepadMotion(const Private::MainDispatcherEvent& e);
    void HandleGamepadButton(const Private::MainDispatcherEvent& e);

    void HandleGamepadAdded(const Private::MainDispatcherEvent& e);
    void HandleGamepadRemoved(const Private::MainDispatcherEvent& e);

private:
    RefPtr<KeyboardDevice> keyboard;
    RefPtr<GamepadDevice> gamepad;

    struct InputHandler
    {
        InputHandler(uint32 token_, eInputDevices inputDeviceMask_, const Function<bool(UIEvent*)>& callback_);

        uint32 token;
        eInputDevices inputDeviceMask;
        Function<bool(UIEvent*)> callback;
    };

    Vector<InputHandler> handlers;
    uint32 nextHandlerToken = 1;
    bool pendingHandlerRemoval = false;
};

inline InputSystem::InputHandler::InputHandler(uint32 token_, eInputDevices inputDeviceMask_, const Function<bool(UIEvent*)>& callback_)
    : token(token_)
    , inputDeviceMask(inputDeviceMask_)
    , callback(callback_)
{
}

inline KeyboardDevice& InputSystem::GetKeyboard()
{
    return *keyboard;
}

inline GamepadDevice& InputSystem::GetGamepadDevice()
{
    return *gamepad;
}

} // namespace DAVA
