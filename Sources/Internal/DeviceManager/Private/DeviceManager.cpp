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
    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &DeviceManager::HandleEvent));
}

DeviceManager::~DeviceManager() = default;

void DeviceManager::UpdateDisplayConfig()
{
    impl->UpdateDisplayConfig();
}

bool DeviceManager::HandleEvent(const Private::MainDispatcherEvent& e)
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
        return true;
    }
    return false;
}

void DeviceManager::OnEngineInited()
{
// TODO: keep track of devices, implement id constants for kb, mouse, touch devices

#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    keyboard = new KeyboardInputDevice(1);
    inputDevices.push_back(keyboard);

    mouse = new MouseDevice(2);
    inputDevices.push_back(mouse);
#endif

#if defined(__DAVAENGINE_WIN_UAP__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    touch = new TouchDevice(3);
    inputDevices.push_back(touch);
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

    return nullptr;
}

KeyboardInputDevice* DeviceManager::GetKeyboard()
{
    return keyboard;
}

MouseDevice* DeviceManager::GetMouse()
{
    return mouse;
}

TouchDevice* DeviceManager::GetTouch()
{
    return touch;
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
