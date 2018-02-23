#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/KeyboardInputController.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>
#include <Math/Math2D.h>
#include <Math/Vector.h>

namespace DAVA
{
void KeyboardInputController::OnInput(UIEvent* e)
{
    bool sendToBase = true;
    if (IsInOperation() == false)
    {
        switch (e->phase)
        {
        case UIEvent::Phase::KEY_DOWN:
            if (pinnedButton == eInputElements::NONE)
            {
                if (callbacks.count(e->key) > 0)
                {
                    pinnedButton = e->key;
                    GetPrimaryWindow()->SetCursorCapture(eCursorCapture::PINNING);
                    sendToBase = false;
                }
            }
            break;
        case UIEvent::Phase::KEY_UP:
            if (pinnedButton == e->key)
            {
                GetPrimaryWindow()->SetCursorCapture(eCursorCapture::OFF);
                pinnedButton = eInputElements::NONE;
                sendToBase = false;
            }
            break;
        case UIEvent::Phase::MOVE:
            if (pinnedButton != eInputElements::NONE)
            {
                Vector2 delta = e->point;
                Window* window = GetPrimaryWindow();
                Size2f windowSize = window->GetSize();

                DVASSERT(callbacks.count(pinnedButton) > 0);
                callbacks[pinnedButton](Vector2(delta.x / windowSize.dx, delta.y / windowSize.dy));
                sendToBase = false;
            }
            break;
        default:
            break;
        }
    }

    if (sendToBase == true)
    {
        DefaultBrushInputController::OnInput(e);
    }
}

void KeyboardInputController::RegisterVarCallback(eInputElements element, const Function<void(const Vector2&)>& callback)
{
    callbacks.emplace(element, callback);
}
} // namespace DAVA
