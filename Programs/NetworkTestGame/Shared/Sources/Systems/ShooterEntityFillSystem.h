#pragma once

#include <Base/Vector.h>
#include <Base/UnordererSet.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
}

class ShooterRoleComponent;

// Responsible for filling the rest of an entity based on its role
class ShooterEntityFillSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterEntityFillSystem, DAVA::SceneSystem);

    ShooterEntityFillSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    void FillPlayerEntity(DAVA::Entity* entity);
    void FillCarEntity(DAVA::Entity* entity);
    void FillBulletEntity(DAVA::Entity* entity);

private:
    DAVA::Vector<ShooterRoleComponent*> newRoles; // List of new roles components since last Process
};