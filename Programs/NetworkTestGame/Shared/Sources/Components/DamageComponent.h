#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class DamageComponent : public DAVA::Component
{
protected:
    virtual ~DamageComponent();

public:
    DAVA_VIRTUAL_REFLECTION(DamageComponent, DAVA::Component);
    DamageComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
    DAVA::uint8 GetDamage() const;

private:
    const DAVA::uint8 damage = 1;
};
