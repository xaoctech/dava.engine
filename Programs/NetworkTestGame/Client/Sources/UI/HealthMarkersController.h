#pragma once

#include <Scene3D/ComponentGroup.h>

namespace DAVA
{
class Component;
class Scene;
class UIControl;
class UIEntityMarkerComponent;
class UIEntityMarkersContainerComponent;
}

class HealthComponent;

class HealthMarkersController final
{
public:
    HealthMarkersController(DAVA::UIControl* container, DAVA::UIControl* prototype, DAVA::Scene* scene);
    ~HealthMarkersController();

    void Process();

private:
    void OnAdd(HealthComponent* c);
    void OnRemove(HealthComponent* c);
    static void CustomStrategy(DAVA::UIControl* ctrl, DAVA::UIEntityMarkersContainerComponent*, DAVA::UIEntityMarkerComponent* emc);

    DAVA::UIControl* container = nullptr;
    DAVA::UIControl* prototype = nullptr;
    DAVA::ComponentGroup<HealthComponent>* group = nullptr;
    DAVA::Vector<HealthComponent*> componentsOnAdd;
};