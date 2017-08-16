#include "Input/InputDevice.h"

namespace DAVA
{
InputDevice::InputDevice(uint32 id)
    : id(id)
{
    DAVA::Engine* engine = DAVA::Engine::Instance();

    engine->windowCreated.Connect(this, &InputDevice::OnWindowCreated);
    engine->windowDestroyed.Connect(this, &InputDevice::OnWindowDestroyed);
}

InputDevice::~InputDevice()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();

    engine->windowCreated.Disconnect(this);
    engine->windowDestroyed.Disconnect(this);

    for (DAVA::UnorderedSet<DAVA::Window*>::iterator it = windows.begin(); it != windows.end(); ++it)
    {
        DAVA::Window* window = *it;

        DVASSERT(window != nullptr);

        window->focusChanged.Disconnect(this);
        window->sizeChanged.Disconnect(this);
    }
}

void InputDevice::OnWindowCreated(DAVA::Window* window)
{
    DVASSERT(window != nullptr);

    windows.insert(window);

    window->focusChanged.Connect(this, &InputDevice::OnWindowFocusChanged);
    window->sizeChanged.Connect(this, &InputDevice::OnWindowSizeChanged);
}

void InputDevice::OnWindowDestroyed(DAVA::Window* window)
{
    DVASSERT(window != nullptr);

    DAVA::UnorderedSet<DAVA::Window*>::iterator it = windows.find(window);

    if (it != windows.end())
    {
        window->focusChanged.Disconnect(this);
        window->sizeChanged.Disconnect(this);

        windows.erase(it);
    }
}

void InputDevice::OnWindowFocusChanged(DAVA::Window* window, bool focused)
{
    // Reset device state when window is unfocused
    if (!focused)
    {
        this->ResetState(window);
    }
}

void InputDevice::OnWindowSizeChanged(DAVA::Window* window, DAVA::Size2f, DAVA::Size2f)
{
    // Reset device state when window size changes
    // To workaround cases when input events are not generated while window is changint its size
    // (e.g. when maximizing window in macOS)
    this->ResetState(window);
}

uint32 InputDevice::GetId() const
{
    return id;
}
} // namespace DAVA