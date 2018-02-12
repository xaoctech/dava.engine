#include "Scene3D/Systems/ReflectionSystem.h"

#include "Entity/ComponentUtils.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Scene3D/Components/ReflectionComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Logger/Logger.h"

#define UPDATE_PROBES_EACH_FRAME 0
#define UPDATE_PROBES_PERIODICALLY 1
#define UPDATE_PROBES_TIMEOUT 5.0f

namespace DAVA
{
ReflectionSystem::ReflectionSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<ReflectionComponent>())
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
    renderSystem = scene->GetRenderSystem();
}

ReflectionSystem::~ReflectionSystem()
{
}

void ReflectionSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::REFLECTION_COMPONENT_CHANGED)
    {
        Entity* entity = component->GetEntity();
        const Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldTransformPtr();

        ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
        if (nullptr != reflectionComponent)
        {
            ReflectionProbe* probe = reflectionComponent->GetReflectionProbe();
            if (nullptr != probe)
            {
                // GFX_COMPLETE if we do not do set of world transform matrix, we have NULL ptr in world matrix in UpdateProbe here
                Vector3 newPosition = worldTransformPointer->GetTranslationVector();
                probe->SetPosition(newPosition);
                probe->SetWorldTransformPtr(worldTransformPointer);

                probe->SetCapturePosition(reflectionComponent->GetCapturePosition());
                probe->SetCaptureSize(reflectionComponent->GetCaptureSize());

                probe->UpdateProbe();
                renderSystem->GetReflectionRenderer()->UpdateProbe(probe);
            }
            renderSystem->GetReflectionRenderer()->SetDebugDrawEnabled(reflectionComponent->GetDebugDrawEnabled());
        }
    }
}

void ReflectionSystem::AddEntity(Entity* entity)
{
    ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
    if (reflectionComponent)
    {
        ReflectionProbe* reflectionProbe = new ReflectionProbe();
        reflectionProbe->SetReflectionType(reflectionComponent->GetReflectionType());
        renderSystem->GetReflectionRenderer()->RegisterReflectionProbe(reflectionProbe);
        reflectionComponent->SetReflectionProbe(reflectionProbe);

        allComponents.push_back(reflectionComponent);
    }
}

void ReflectionSystem::RemoveEntity(Entity* entity)
{
    ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
    if (reflectionComponent)
    {
        ReflectionProbe* reflectionProbe = reflectionComponent->GetReflectionProbe();
        renderSystem->GetReflectionRenderer()->UnregisterReflectionProbe(reflectionProbe);
        SafeRelease(reflectionProbe);
        FindAndRemoveExchangingWithLast(allComponents, reflectionComponent);
    }
}

void ReflectionSystem::PrepareForRemove()
{
    for (ReflectionComponent* reflectionComponent : allComponents)
    {
        ReflectionProbe* reflectionProbe = reflectionComponent->GetReflectionProbe();
        renderSystem->GetReflectionRenderer()->UnregisterReflectionProbe(reflectionProbe);
        SafeRelease(reflectionProbe);
    }
    allComponents.clear();
}

void ReflectionSystem::AddComponent(Entity* entity, Component* component)
{
    ReflectionComponent* reflectionComponent = dynamic_cast<ReflectionComponent*>(component);
    if (reflectionComponent)
        allComponents.push_back(reflectionComponent);
}

void ReflectionSystem::RemoveComponent(Entity* entity, Component* component)
{
    ReflectionComponent* reflectionComponent = dynamic_cast<ReflectionComponent*>(component);
    if (reflectionComponent)
        FindAndRemoveExchangingWithLast(allComponents, reflectionComponent);
}

void ReflectionSystem::Process(float32 timeElapsed)
{
#if (UPDATE_PROBES_EACH_FRAME)
    for (ReflectionComponent* component : allComponents)
    {
        Entity* entity = component->GetEntity();
        {
            {
#elif (UPDATE_PROBES_PERIODICALLY)

    timeToUpdate -= timeElapsed;
    bool updateProbes = (timeToUpdate <= 0.0f);
    if (updateProbes)
        timeToUpdate = UPDATE_PROBES_TIMEOUT;

    for (ReflectionComponent* component : allComponents)
    {
        Entity* entity = component->GetEntity();
        {
            if (updateProbes)
            {

#else

    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<ReflectionComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
#endif
                ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
                if (nullptr != reflectionComponent)
                {
                    ReflectionProbe* probe = reflectionComponent->GetReflectionProbe();
                    if (nullptr != probe)
                    {
                        const Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldTransformPtr();
                        Vector3 newPosition = worldTransformPointer->GetTranslationVector();

                        probe->SetPosition(newPosition);
                        probe->SetWorldTransformPtr(worldTransformPointer);
                        probe->SetCapturePosition(reflectionComponent->GetCapturePosition());
                        probe->SetCaptureSize(reflectionComponent->GetCaptureSize());

                        // GFX_COMPLETE - think about proper update mechanism
                        probe->UpdateProbe();
                        renderSystem->GetReflectionRenderer()->UpdateProbe(probe);
                    }
                    renderSystem->GetReflectionRenderer()->SetDebugDrawEnabled(reflectionComponent->GetDebugDrawEnabled());
                }
            }
        }
    }
}
}
