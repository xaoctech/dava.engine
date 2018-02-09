#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class TEMPLATEComponent : public DAVA::Component
{
protected:
    virtual ~TEMPLATEComponent();

public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATEComponent, DAVA::Component);
    TEMPLATEComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
