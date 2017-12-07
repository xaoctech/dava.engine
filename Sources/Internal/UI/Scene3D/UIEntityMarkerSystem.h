#pragma once

#include "Base/BaseTypes.h"
#include "Base/Vector.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIComponent;
class UIControl;
class UIEntityMarkerComponent;

/** System for synchronization params between UIControl and Entity. */
class UIEntityMarkerSystem : public UISystem
{
public:
    UIEntityMarkerSystem();
    ~UIEntityMarkerSystem();

protected:
    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(float32 elapsedTime) override;

private:
    Vector<UIEntityMarkerComponent*> components;
};
}