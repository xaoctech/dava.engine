#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class CharacterVisibilityShapeComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(CharacterVisibilityShapeComponent, DAVA::Component);

    CharacterVisibilityShapeComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::float32 height;
};
