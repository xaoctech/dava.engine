#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Components/ReflectionComponent.h"

namespace DAVA
{
class ReflectionSystem : public SceneSystem
{
public:
    ReflectionSystem(Scene* scene);
    virtual ~ReflectionSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;

    const Vector<ReflectionComponent*>& GetAllComponents() const;

private:
    void UpdateReflections(ReflectionComponent* component);
    void MakeUniqueGlobalProbe(ReflectionComponent* component);
    void UpdateAndRegisterProbe(ReflectionComponent* component, ReflectionProbe::ProbeType targetType);

private:
    Vector<ReflectionComponent*> allComponents;
    RenderSystem* renderSystem = nullptr;
    float timeToUpdate = 0.0f;
};

inline const Vector<ReflectionComponent*>& ReflectionSystem::GetAllComponents() const
{
    return allComponents;
}
};
