#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

class ShooterProjectileComponent final : public DAVA::Component
{
public:
    enum class ProjectileType
    {
        Bullet,
        Rocket
    };

    DAVA_VIRTUAL_REFLECTION(ShooterProjectileComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetProjectileType(ProjectileType value);
    ProjectileType GetProjectileType() const;

private:
    friend class ShooterProjectileSystem;

    ProjectileType type;
};