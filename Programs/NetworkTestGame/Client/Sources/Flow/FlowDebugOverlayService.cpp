#include "FlowDebugOverlayService.h"

#include <Debug/DebugOverlay.h>
#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/Mouse.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FlowDebugOverlayService)
{
    using namespace DAVA;

    ReflectionRegistrator<FlowDebugOverlayService>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](FlowDebugOverlayService* s) { delete s; })
    .Method("toggleOverlay", &FlowDebugOverlayService::ToggleDebugOverlay)
    .End();
}

FlowDebugOverlayService::FlowDebugOverlayService()
{
    DAVA::Logger::Info("Debug overlay service constructed.");
}

void FlowDebugOverlayService::ToggleDebugOverlay()
{
    using namespace DAVA;

    const EngineContext* context = GetEngineContext();
    Window* window = GetPrimaryWindow();

    if (context != nullptr && window != nullptr)
    {
        DebugOverlay* overlay = context->debugOverlay;

        if (overlay != nullptr)
        {
            const Mouse* mouse = context->deviceManager->GetMouse();

            if (!overlay->IsShown())
            {
                if (mouse != nullptr)
                {
                    // Senseless. But what if we can open UI menu by keyboard?
                    cursorWasPinned = window->GetCursorCapture() == eCursorCapture::PINNING;

                    if (cursorWasPinned)
                    {
                        window->SetCursorCapture(eCursorCapture::OFF);
                    }
                }

                overlay->Show();
            }
            else
            {
                if (mouse != nullptr && cursorWasPinned)
                {
                    window->SetCursorCapture(eCursorCapture::PINNING);
                }

                overlay->Hide();
            }
        }
    }
}
