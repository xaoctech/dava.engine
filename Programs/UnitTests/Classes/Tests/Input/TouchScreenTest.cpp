#include "UnitTests/UnitTests.h"

#include <Engine/Engine.h>
#include <Engine/Private/EngineBackend.h>
#include <Engine/Private/Dispatcher/MainDispatcher.h>
#include <Engine/Private/Dispatcher/MainDispatcherEvent.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/TouchScreen.h>
#include <Logger/Logger.h>

using namespace DAVA;

DAVA_TESTCLASS (TouchScreenTestClass)
{
    DAVA_TEST (TouchScreenSupportedElementsTest)
    {
        // Check touch screen supported elements:
        //   - all touch screen elements are supported
        //   - all non-touch screen elements are not supported

        TouchScreen* touchscreen = GetEngineContext()->deviceManager->GetTouchScreen();
        if (touchscreen == nullptr)
        {
            return;
        }

        for (uint32 i = static_cast<uint32>(eInputElements::FIRST); i <= static_cast<uint32>(eInputElements::LAST); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            TEST_VERIFY(IsTouchInputElement(element) ? touchscreen->IsElementSupported(element) : !touchscreen->IsElementSupported(element));
        }
    }

    DAVA_TEST (TouchScreenDefaultStateTest)
    {
        // Check touch screen default state: equal to released for each click, {0, 0, 0} for each position

        TouchScreen* touchscreen = GetEngineContext()->deviceManager->GetTouchScreen();
        if (touchscreen == nullptr)
        {
            return;
        }

        for (uint32 i = static_cast<uint32>(eInputElements::TOUCH_FIRST_CLICK); i <= static_cast<uint32>(eInputElements::TOUCH_LAST_CLICK); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            DigitalElementState state = touchscreen->GetDigitalElementState(element);
            TEST_VERIFY(state.IsReleased() && !state.IsJustReleased());
        }

        for (uint32 i = static_cast<uint32>(eInputElements::TOUCH_FIRST_POSITION); i <= static_cast<uint32>(eInputElements::TOUCH_LAST_POSITION); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            AnalogElementState state = touchscreen->GetAnalogElementState(element);
            TEST_VERIFY(state.x == 0.0f && state.y == 0.0f && state.z == 0.0f);
        }
    }

    DAVA_TEST (TouchScreenSingleTouchEventHandlingTest)
    {
        // Check event handling by the touch screen, for the first touch:
        //   - Check that initial state is released
        //   - Imitate platform sending TOUCH_DOWN event
        //   - Check that click state has changed to just pressed, position has changed to the correct one
        //   - Imitate platform sending TOUCH_MOVE event
        //   - Check that position has changed to the new one
        //   - Wait for the next frame, check it has changed to pressed
        //   - Imititate platform sending TOUCH_UP event
        //   - Check that state has changed to just released
        //   - Wait for the next frame, check that it has changed to released, position has changed to {0, 0, 0}
        //
        // Handled in Update()

        TouchScreen* touchscreen = GetEngineContext()->deviceManager->GetTouchScreen();
        if (touchscreen == nullptr)
        {
            touchScreenEventHandlingTestFinished = true;
            return;
        }
    }

    DAVA_TEST (TouchScreenMultiTouchEventHandlingTest)
    {
        // TODO
    }

    void CheckSingleState(TouchScreen * touchScreen, eInputElements requiredElement, DigitalElementState requiredState)
    {
        // All elements should be in released state, `requiredElement` must be in `requiredState`
        for (uint32 i = static_cast<uint32>(eInputElements::TOUCH_FIRST_CLICK); i <= static_cast<uint32>(eInputElements::TOUCH_LAST_CLICK); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            DigitalElementState state = touchScreen->GetDigitalElementState(element);

            if (element == requiredElement)
            {
                TEST_VERIFY(state == requiredState);
            }
            else
            {
                TEST_VERIFY(state.IsReleased() && !state.IsJustReleased());
            }
        }
    }

    void Update(float32 timeElapsed, const String& testName) override
    {
        if (testName == "TouchScreenSingleTouchEventHandlingTest" && !touchScreenEventHandlingTestFinished)
        {
            using namespace DAVA::Private;

            TouchScreen* touchscreen = GetEngineContext()->deviceManager->GetTouchScreen();

            if (!waitingForFirstFrame && !waitingForSecondFrame)
            {
                // Check that click is released
                // Send TOUCH_DOWN and check that button is just pressed, position is correct
                // Send TOUCH_MOVE and check that position is correct
                // Wait for the next frame

                Logger::Info("TouchScreenEventHandlingTest: testing element '%s'", GetInputElementInfo(eInputElements::TOUCH_CLICK0).name.c_str());

                DigitalElementState currentElementStateBeforeEvent = touchscreen->GetDigitalElementState(eInputElements::TOUCH_CLICK0);
                CheckSingleState(touchscreen, eInputElements::TOUCH_CLICK0, DigitalElementState::Released());
                AnalogElementState currentElementPositionBeforeEvent = touchscreen->GetAnalogElementState(eInputElements::TOUCH_POSITION0);
                TEST_VERIFY(currentElementPositionBeforeEvent.x == 0.0f && currentElementPositionBeforeEvent.y == 0.0f && currentElementPositionBeforeEvent.z == 0.0f);

                const float32 initialTouchX = 32.0f;
                const float32 initialTouchY = 51.3f;
                Window* primaryWindow = GetPrimaryWindow();
                MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();
                dispatcher->SendEvent(MainDispatcherEvent::CreateWindowTouchEvent(primaryWindow, MainDispatcherEvent::TOUCH_DOWN, static_cast<uint32>(eInputElements::TOUCH_CLICK0), initialTouchX, initialTouchY, eModifierKeys::NONE));

                DigitalElementState currentElementStateAfterEvent = touchscreen->GetDigitalElementState(eInputElements::TOUCH_CLICK0);
                CheckSingleState(touchscreen, eInputElements::TOUCH_CLICK0, DigitalElementState::JustPressed());
                AnalogElementState currentElementPositionAfterEvent = touchscreen->GetAnalogElementState(eInputElements::TOUCH_POSITION0);
                TEST_VERIFY(currentElementPositionAfterEvent.x == initialTouchX && currentElementPositionAfterEvent.y == initialTouchY && currentElementPositionAfterEvent.z == 0.0f);

                const float32 moveTouchX = 390.21f;
                const float32 moveTouchY = 124.03f;
                dispatcher->SendEvent(MainDispatcherEvent::CreateWindowTouchEvent(primaryWindow, MainDispatcherEvent::TOUCH_MOVE, static_cast<uint32>(eInputElements::TOUCH_CLICK0), moveTouchX, moveTouchY, eModifierKeys::NONE));
                currentElementPositionAfterEvent = touchscreen->GetAnalogElementState(eInputElements::TOUCH_POSITION0);
                TEST_VERIFY(currentElementPositionAfterEvent.x == moveTouchX && currentElementPositionAfterEvent.y == moveTouchY && currentElementPositionAfterEvent.z == 0.0f);

                waitingForFirstFrame = true;
            }
            else if (waitingForFirstFrame)
            {
                // Check that click is pressed
                // Send TOUCH_UP and check that click is just released
                // Wait for the next frame

                DigitalElementState currentElementStateBeforeEvent = touchscreen->GetDigitalElementState(eInputElements::TOUCH_CLICK0);
                CheckSingleState(touchscreen, eInputElements::TOUCH_CLICK0, DigitalElementState::Pressed());

                Window* primaryWindow = GetPrimaryWindow();
                MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();
                dispatcher->SendEvent(MainDispatcherEvent::CreateWindowTouchEvent(primaryWindow, MainDispatcherEvent::TOUCH_UP, static_cast<uint32>(eInputElements::TOUCH_CLICK0), 0.0f, 0.0f, eModifierKeys::NONE));

                DigitalElementState currentElementStateAfterEvent = touchscreen->GetDigitalElementState(eInputElements::TOUCH_CLICK0);
                CheckSingleState(touchscreen, eInputElements::TOUCH_CLICK0, DigitalElementState::JustReleased());
                AnalogElementState currentElementPositionAfterEvent = touchscreen->GetAnalogElementState(eInputElements::TOUCH_POSITION0);
                TEST_VERIFY(currentElementPositionAfterEvent.x == 0.0f && currentElementPositionAfterEvent.y == 0.0f && currentElementPositionAfterEvent.z == 0.0f);

                waitingForFirstFrame = false;
                waitingForSecondFrame = true;
            }
            else if (waitingForSecondFrame)
            {
                // Check that click is released
                // Finish

                DigitalElementState currentElementState = touchscreen->GetDigitalElementState(eInputElements::TOUCH_CLICK0);
                CheckSingleState(touchscreen, eInputElements::TOUCH_CLICK0, DigitalElementState::Released());

                touchScreenEventHandlingTestFinished = true;

                waitingForSecondFrame = false;
            }
        }
    }

    bool TestComplete(const String& testName) const override
    {
        if (testName != "TouchScreenSingleTouchEventHandlingTest")
        {
            return true;
        }
        else
        {
            return touchScreenEventHandlingTestFinished;
        }
    }

private:
    // TouchScreenEventHandlingTest variables
    bool waitingForFirstFrame = false; // Button has been pressed, wait for the next frame to check that its status has changed
    bool waitingForSecondFrame = false; // Button has been released, wait for the next frame to check that its status has changed
    bool touchScreenEventHandlingTestFinished = false;
};