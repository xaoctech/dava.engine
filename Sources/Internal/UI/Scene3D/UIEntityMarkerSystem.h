#pragma once

#include "Base/BaseTypes.h"
#include "Base/Vector.h"
#include "UI/UISystem.h"
#include "Functional/Function.h"

namespace DAVA
{
class UIComponent;
class UIControl;
class UIEntityMarkerComponent;
class UIEntityMarkerPositionComponent;
class UIEntityMarkerScaleComponent;
class UIEntityMarkerVisibilityComponent;

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
    struct Link
    {
        UIControl* control = nullptr;
        UIEntityMarkerComponent* linkComponent = nullptr;
        UIEntityMarkerPositionComponent* positionComponent = nullptr;
        UIEntityMarkerScaleComponent* scaleComponent = nullptr;
        UIEntityMarkerVisibilityComponent* visibilityComponent = nullptr;
    };

    Vector<Link> links;

    Link* Find(UIControl* c);
};
}