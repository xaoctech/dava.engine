#pragma once

#include <Base/BaseTypes.h>
#include <Functional/Signal.h>
#include <UI/UISystem.h>
#include <UI/Layouts/UILayoutSystemListener.h>

/**
This system checks for the changes in control's layout and emit signal just before rendering of that changes.
Basically it's designed to catch time point after control updating by UILayoutSystem and UIScrollSystem
but before UIRenderSystem.
It's useful as we haven't any update notifications from UIScrollSystem.
*/
class RelayoutSignallerSystem final : public DAVA::UISystem, DAVA::UILayoutSystemListener
{
public:
    explicit RelayoutSignallerSystem();
    ~RelayoutSignallerSystem() override;

    DAVA::Signal<> beforeRelayoutedControlRender; // emits for every control which was relayouted in this frame, in the moment before rendering of this control

private:
    // UISystem
    void Process(DAVA::float32 elapsedTime) override;

    // UILayoutSystemListener
    void OnControlLayouted(DAVA::UIControl* control) override;

private:
    bool isDirty = false;
};
