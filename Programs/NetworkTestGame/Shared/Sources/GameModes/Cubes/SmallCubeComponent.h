#pragma once

#include <Entity/Component.h>

namespace DAVA
{
class Entity;
} // namespace DAVA

class SmallCubeComponent final : public DAVA::Component
{
    DAVA_VIRTUAL_REFLECTION(SmallCubeComponent, DAVA::Component);

public:
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};