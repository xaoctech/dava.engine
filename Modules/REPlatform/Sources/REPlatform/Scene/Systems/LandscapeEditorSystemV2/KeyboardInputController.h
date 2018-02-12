#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushInputController.h"

#include <Functional/Function.h>

namespace DAVA
{
class KeyboardInputController : public DefaultBrushInputController
{
public:
    void OnInput(UIEvent* e) override;
    void RegisterVarCallback(eInputElements element, const Function<void(const Vector2&)>& callback);

private:
    Map<eInputElements, Function<void(const Vector2&)>> callbacks;
    eInputElements pinnedButton = eInputElements::NONE;
};
} // namespace DAVA
