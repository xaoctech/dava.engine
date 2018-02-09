#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class ShootCooldownComponent : public DAVA::Component
{
protected:
    virtual ~ShootCooldownComponent();

public:
    DAVA_VIRTUAL_REFLECTION(ShootCooldownComponent, DAVA::Component);
    ShootCooldownComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::uint32 GetLastShootFrameId() const;
    void SetLastShootFrameId(DAVA::uint32 frameId);

protected:
    DAVA::uint32 lastShootFrameId = 0;
};
