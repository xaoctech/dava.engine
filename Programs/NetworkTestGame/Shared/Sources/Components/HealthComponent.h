#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class HealthComponent : public DAVA::Component
{
protected:
    virtual ~HealthComponent();

public:
    DAVA_VIRTUAL_REFLECTION(HealthComponent, DAVA::Component);
    HealthComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void DecHealth(DAVA::uint8 dec, DAVA::uint32 frameId);
    void SetHealth(DAVA::uint8 health);
    DAVA::uint8 GetHealth() const;

    DAVA::uint32 GetLastDamageId() const;

private:
    DAVA::uint32 lastDamageFrameId = 0;
    DAVA::uint8 health = 10;
};
