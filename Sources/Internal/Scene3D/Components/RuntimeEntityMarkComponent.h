#pragma once

#include "Entity/Component.h"

namespace DAVA
{
class RuntimeEntityMarkComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

private:
    DAVA_VIRTUAL_REFLECTION(RuntimeEntityMarkComponent, Component);
};
} // namespace DAVA
