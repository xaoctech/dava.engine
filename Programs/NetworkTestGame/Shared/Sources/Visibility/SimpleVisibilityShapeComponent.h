#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class SimpleVisibilityShapeComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(SimpleVisibilityShapeComponent, DAVA::Component);

    SimpleVisibilityShapeComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
