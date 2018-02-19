#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

class TEMPLATEComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATEComponent, DAVA::Component);
    TEMPLATEComponent();
    virtual ~TEMPLATEComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
