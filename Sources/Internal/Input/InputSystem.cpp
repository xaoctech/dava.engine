#include "Input/InputSystem.h"

#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"

namespace DAVA
{
InputSystem* InputSystem::Instance()
{
    return GetEngineContext()->inputSystem;
}

InputSystem::InputSystem(Engine* engine)
    : keyboard(new KeyboardDevice())
    , gamepad(new GamepadDevice(this))
{
    engine->update.Connect(this, &InputSystem::Update);
}

InputSystem::~InputSystem() = default;

uint32 InputSystem::AddHandler(eInputDevices inputDeviceMask, const Function<bool(UIEvent*)>& callback)
{
    DVASSERT(callback != nullptr);

    uint32 token = nextHandlerToken;
    nextHandlerToken += 1;
    handlers.emplace_back(token, inputDeviceMask, callback);
    return token;
}

void InputSystem::ChangeHandlerDeviceMask(uint32 token, eInputDevices newInputDeviceMask)
{
    auto it = std::find_if(begin(handlers), end(handlers), [token](const InputHandler& o) { return o.token == token; });
    if (it != end(handlers))
    {
        it->inputDeviceMask = newInputDeviceMask;
    }
}

void InputSystem::RemoveHandler(uint32 token)
{
    DVASSERT(token != 0);

    auto it = std::find_if(begin(handlers), end(handlers), [token](const InputHandler& o) { return o.token == token; });
    if (it != end(handlers))
    {
        it->token = 0;
        it->inputDeviceMask = eInputDevices::UNKNOWN;
        pendingHandlerRemoval = true;
    }
}

void InputSystem::Update(float32 frameDelta)
{
    gamepad->Update();
}

void InputSystem::OnAfterUpdate()
{
    keyboard->OnFinishFrame();
    if (pendingHandlerRemoval)
    {
        handlers.erase(std::remove_if(begin(handlers), end(handlers), [](const InputHandler& h) { return h.token == 0; }), end(handlers));
        pendingHandlerRemoval = false;
    }
}

void InputSystem::HandleInputEvent(UIEvent* uie)
{
    bool handled = false;
    for (const InputHandler& h : handlers)
    {
        if ((h.inputDeviceMask & uie->device) != eInputDevices::UNKNOWN)
        {
            handled |= h.callback(uie);
            if (handled)
            {
                break;
            }
        }
    }
    if (!handled && uie->window != nullptr)
    {
        UIControlSystem* uiControlSystem = uie->window->GetUIControlSystem();
        uiControlSystem->OnInput(uie);
    }
}

void InputSystem::HandleGamepadMotion(const Private::MainDispatcherEvent& e)
{
    gamepad->HandleGamepadMotion(e);
}

void InputSystem::HandleGamepadButton(const Private::MainDispatcherEvent& e)
{
    gamepad->HandleGamepadButton(e);
}

void InputSystem::HandleGamepadAdded(const Private::MainDispatcherEvent& e)
{
    gamepad->HandleGamepadAdded(e);
}

void InputSystem::HandleGamepadRemoved(const Private::MainDispatcherEvent& e)
{
    gamepad->HandleGamepadRemoved(e);
}

} // namespace DAVA
