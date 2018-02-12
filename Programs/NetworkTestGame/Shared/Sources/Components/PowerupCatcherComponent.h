#pragma once

#include "Entity/Component.h"

using namespace DAVA;

class PowerupCatcherComponent : public Component
{
public:
    DAVA_VIRTUAL_REFLECTION(PowerupCatcherComponent, Component);

    PowerupCatcherComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
