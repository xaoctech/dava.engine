#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

// Responsible for indicating which role an entity has
class ShooterRoleComponent final : public DAVA::Component
{
public:
    enum Role
    {
        Player,
        Car,
        Bullet
    };

    DAVA_VIRTUAL_REFLECTION(ShooterRoleComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetRole(Role value);
    Role GetRole() const;

private:
    int role;
};
