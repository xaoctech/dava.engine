#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Math/Quaternion.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
class SerializationContext;
class TransformInterpolatedComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(TransformInterpolationComponent, Component);

public:
    bool isActual = false;
    Vector3 translation;
    Quaternion rotation;

    Component* Clone(Entity* toEntity) override;
};
} // namespace DAVA
