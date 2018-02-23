#pragma once
#include <limits>

#include <Entity/SceneSystem.h>
#include <Entity/SingletonComponent.h>
#include <Reflection/Reflection.h>

using namespace DAVA;

class PowerupSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PowerupSystem, SceneSystem);

    PowerupSystem(Scene* scene);
    void PrepareForRemove() override{};

    void ProcessFixed(float32 timeElapsed) override;

    void AddEntity(Entity* bonus) override;
    void RemoveEntity(Entity* bonus) override;

private:
    bool ApplyBonus(Entity* bonus, Entity* target);

    Vector<Entity*> bonuses;

    UnorderedSet<Entity*> triggeredEntities;
    Vector<Entity*> bonusesToRemove;
};
