
#include "EditorSystems/EditorCanvas.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"
#include "Modules/DocumentsModule/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Core/FieldBinder.h>

#include <UI/UIEvent.h>
#include <UI/UIControl.h>

EditorCanvas::EditorCanvas(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
{
    wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorCanvasData>());
    InitFieldBinder();
}

bool EditorCanvas::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    using namespace DAVA;
    if (wrapper.HasData() == false)
    {
        return false;
    }
    if ((currentInput->device & eInputDevices::CLASS_POINTER) == eInputDevices::UNKNOWN)
    {
        return false;
    }
    if (currentInput->phase == UIEvent::Phase::WHEEL || currentInput->phase == UIEvent::Phase::GESTURE)
    {
        return true;
    }
    return (systemsManager->GetDragState() == EditorSystemsManager::DragScreen &&
            (currentInput->mouseButton == eMouseButtons::LEFT || currentInput->mouseButton == eMouseButtons::MIDDLE));
}

void EditorCanvas::ProcessInput(DAVA::UIEvent* currentInput)
{
    using namespace DAVA;
    DVASSERT(wrapper.HasData());
    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    EditorCanvasData* canvasData = activeContext->GetData<EditorCanvasData>();

    if (accessor->GetActiveContext() == nullptr)
    {
        return;
    }
    if (currentInput->device == eInputDevices::TOUCH_PAD)
    {
        if (currentInput->phase == UIEvent::Phase::GESTURE)
        {
            const UIEvent::Gesture& gesture = currentInput->gesture;
            if (gesture.dx != 0.0f || gesture.dy != 0.0f)
            {
                Vector2 position = canvasData->GetPosition();
                Vector2 newPosition = position - Vector2(gesture.dx, gesture.dy);
                wrapper.SetFieldValue(EditorCanvasData::positionPropertyName, newPosition);
            }
            else if (gesture.magnification != 0.0f)
            {
                wrapper.SetFieldValue(EditorCanvasData::referencePointPropertyName, currentInput->physPoint);
                float32 newScale = canvasData->GetScale() + gesture.magnification;
                wrapper.SetFieldValue(EditorCanvasData::scalePropertyName, newScale);
            }
        }
    }
    else if (currentInput->device == eInputDevices::MOUSE)
    {
        if (currentInput->phase == UIEvent::Phase::WHEEL)
        {
            if ((currentInput->modifiers & (eModifierKeys::CONTROL | eModifierKeys::COMMAND)) != eModifierKeys::NONE)
            {
                wrapper.SetFieldValue(EditorCanvasData::referencePointPropertyName, currentInput->physPoint);

                int32 ticksCount = static_cast<int32>(currentInput->wheelDelta.y);
                float newScale = GetScaleFromWheelEvent(ticksCount);
                wrapper.SetFieldValue(EditorCanvasData::scalePropertyName, newScale);
            }
            else
            {
                Vector2 position = canvasData->GetPosition();
                Vector2 additionalPos(currentInput->wheelDelta.x, currentInput->wheelDelta.y);

                CentralWidgetData* centralWidgetData = accessor->GetGlobalContext()->GetData<CentralWidgetData>();

                additionalPos *= centralWidgetData->GetViewSize();
//custom delimiter to scroll widget by little chunks of visible area
#if defined(__DAVAENGINE_MACOS__)
                //on the OS X platform wheelDelta depend on scrolling speed
                static const float wheelDelta = 0.002f;
#elif defined(__DAVAENGINE_WIN32__)
                static const float wheelDelta = 0.1f;
#endif //platform
                Vector2 newPosition = position - additionalPos * wheelDelta;
                wrapper.SetFieldValue(EditorCanvasData::positionPropertyName, newPosition);
            }
        }
        else
        {
            Vector2 position = canvasData->GetPosition();
            Vector2 delta = systemsManager->GetMouseDelta();
            wrapper.SetFieldValue(EditorCanvasData::positionPropertyName, position - delta);
        }
    }
}

EditorSystemsManager::eDragState EditorCanvas::RequireNewState(DAVA::UIEvent* currentInput)
{
    using namespace DAVA;
    Function<void(UIEvent*, bool)> setMouseButtonOnInput = [this](const UIEvent* currentInput, bool value) {
        if (currentInput->mouseButton == eMouseButtons::MIDDLE)
        {
            isMouseMidButtonPressed = value;
        }
    };
    if (currentInput->device == eInputDevices::MOUSE)
    {
        if (currentInput->phase == UIEvent::Phase::BEGAN)
        {
            setMouseButtonOnInput(currentInput, true);
        }
        else if (currentInput->phase == UIEvent::Phase::ENDED)
        {
            setMouseButtonOnInput(currentInput, false);
        }
    }
    else if (currentInput->device == eInputDevices::KEYBOARD && currentInput->key == Key::SPACE)
    {
        //we cant use isKeyPressed here, because DAVA update keyboard state after sending Key_Up event
        //if we will press key on dock widget and hold it -> DAVA will receive only KEY_REPEAT event without KEY_DOWN
        if (currentInput->phase == UIEvent::Phase::KEY_DOWN || currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
        {
            isSpacePressed = true;
        }
        else if (currentInput->phase == UIEvent::Phase::KEY_UP)
        {
            isSpacePressed = false;
        }
    }
    bool inDragScreenState = isMouseMidButtonPressed || isSpacePressed;
    return inDragScreenState ? EditorSystemsManager::DragScreen : EditorSystemsManager::NoDrag;
}

void EditorCanvas::InitFieldBinder()
{
    using namespace DAVA;
    using namespace TArc;
    fieldBinder.reset(new FieldBinder(accessor));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = FastName(EditorCanvasData::movableControlPositionPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &EditorCanvas::OnMovableControlPositionChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = FastName(EditorCanvasData::scalePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &EditorCanvas::OnScaleChanged));
    }
}

DAVA::float32 EditorCanvas::GetScaleFromWheelEvent(DAVA::int32 ticksCount) const
{
    using namespace DAVA;
    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    EditorCanvasData* canvasData = activeContext->GetData<EditorCanvasData>();
    if (ticksCount > 0)
    {
        return canvasData->GetNextScale(ticksCount);
    }
    else if (ticksCount < 0)
    {
        return canvasData->GetPreviousScale(ticksCount);
    }
    return canvasData->GetScale();
}

void EditorCanvas::OnMovableControlPositionChanged(const DAVA::Any& movableControlPosition)
{
    using namespace DAVA;

    //right now we scale and move the same UIControl
    //because there is no reason to have another UIControl for moving only
    UIControl* movableControl = systemsManager->GetScalableControl();
    if (movableControlPosition.CanGet<Vector2>())
    {
        Vector2 position = movableControlPosition.Get<Vector2>();
        movableControl->SetPosition(position);
    }
    else
    {
        movableControl->SetPosition(Vector2(0.0f, 0.0f));
    }
}

void EditorCanvas::OnScaleChanged(const DAVA::Any& scaleValue)
{
    using namespace DAVA;
    UIControl* scalableControl = systemsManager->GetScalableControl();
    if (scaleValue.CanGet<float32>())
    {
        float32 scale = scaleValue.Get<float32>();
        scalableControl->SetScale(Vector2(scale, scale));
    }
    else
    {
        scalableControl->SetScale(Vector2(1.0f, 1.0f));
    }
}
