#include "DeviceManager/DeviceManager.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

#if defined(__DAVAENGINE_QT__)
#include "DeviceManager/Private/Qt/DeviceManagerImplQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "DeviceManager/Private/Win32/DeviceManagerImplWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "DeviceManager/Private/Win10/DeviceManagerImplWin10.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "DeviceManager/Private/Mac/DeviceManagerImplMac.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "DeviceManager/Private/Ios/DeviceManagerImplIos.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "DeviceManager/Private/Android/DeviceManagerImplAndroid.h"
#else
#error "DeviceManager: unknown platform"
#endif

namespace DAVA
{
DeviceManager::DeviceManager(Private::EngineBackend* engineBackend)
    : impl(new Private::DeviceManagerImpl(this, engineBackend->GetDispatcher()))
{
}

DeviceManager::~DeviceManager() = default;

void DeviceManager::UpdateDisplayConfig()
{
    impl->UpdateDisplayConfig();
}

void DeviceManager::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;
    if (e.type == MainDispatcherEvent::DISPLAY_CONFIG_CHANGED)
    {
        size_t count = e.displayConfigEvent.count;
        DisplayInfo* displayInfo = e.displayConfigEvent.displayInfo;

        displays.resize(count);
        std::move(displayInfo, displayInfo + count, begin(displays));

        delete[] displayInfo;

        displayConfigChanged.Emit();
    }
}

void DeviceManager::OnEngineInited()
{
#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_MACOS__)
    keyboard = new KeyboardInputDevice(0);
    mouse = new MouseInputDevice(1);

    inputDevices.push_back(keyboard);
    inputDevices.push_back(mouse);
#endif
}

InputDevice* DeviceManager::GetInputDevice(uint32 id)
{
    for (InputDevice* device : inputDevices)
    {
        if (device->GetId() == id)
        {
            return device;
        }
    }

    DVASSERT(false);

    return nullptr;
}

KeyboardInputDevice* DeviceManager::GetKeyboard()
{
    return keyboard;
}

MouseInputDevice* DeviceManager::GetMouse()
{
    return mouse;
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
