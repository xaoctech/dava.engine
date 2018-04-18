#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class TEMPLATEComponent : public Component
{
protected:
    virtual ~TEMPLATEComponent();

public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATEComponent, Component);
    TEMPLATEComponent();

    Component* Clone(Entity* toEntity) override;
};
}
