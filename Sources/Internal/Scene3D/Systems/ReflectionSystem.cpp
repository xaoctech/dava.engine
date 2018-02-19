#include "Scene3D/Systems/ReflectionSystem.h"

#include "Entity/ComponentUtils.h"
#include "FileSystem/FileSystem.h"
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

void ReflectionSystem::UpdateReflections(ReflectionComponent* reflectionComponent)
{
    DVASSERT(reflectionComponent != nullptr);
    ReflectionProbe* probe = reflectionComponent->GetReflectionProbe();
    DVASSERT(probe != nullptr);

    // GFX_COMPLETE if we do not do set of world transform matrix, we have NULL ptr in world matrix in UpdateProbe here
    const Matrix4* worldTransformPointer = reflectionComponent->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransformPtr();

    probe->SetPosition(worldTransformPointer->GetTranslationVector());
    probe->SetWorldTransformPtr(worldTransformPointer);
    probe->SetCapturePosition(reflectionComponent->GetCapturePosition());
    probe->SetCaptureSize(reflectionComponent->GetCaptureSize());
    probe->UpdateProbe();

    renderSystem->GetReflectionRenderer()->UpdateProbe(probe);

    Vector<ReflectionComponent*> debugDrawComponents;
    debugDrawComponents.reserve(allComponents.size());
    for (ReflectionComponent* component : allComponents)
    {
        if (component->GetDebugDrawEnabled())
            debugDrawComponents.emplace_back(component);
    }

    if (debugDrawComponents.empty())
    {
        renderSystem->GetReflectionRenderer()->SetDebugDrawProbe(nullptr);
    }
    else if (reflectionComponent->GetDebugDrawEnabled())
    {
        for (ReflectionComponent* component : debugDrawComponents)
        {
            if ((component != reflectionComponent))
                component->DisableDebugDraw();
        }
        renderSystem->GetReflectionRenderer()->SetDebugDrawProbe(probe);
    }
}

void ReflectionSystem::UpdateAndRegisterProbe(ReflectionComponent* component, ReflectionProbe::ProbeType targetType)
{
    ReflectionProbe* probe = component->GetReflectionProbe();
    probe->SetReflectionType(targetType);

    if (probe->IsStaticProbe())
    {
        ScopedPtr<Texture> image;
        if (!component->GetReflectionsMap().IsEmpty() && DAVA::FileSystem::Instance()->Exists(component->GetReflectionsMap()))
        {
            image.reset(Texture::CreateFromFile(component->GetReflectionsMap()));
        }
        else
        {
            image.reset(Texture::CreatePink(rhi::TextureType::TEXTURE_TYPE_CUBE));
        }
        probe->SetCurrentTexture(image);
        probe->SetSphericalHarmonics(component->GetSphericalHarmonics());
    }

    renderSystem->GetReflectionRenderer()->RegisterReflectionProbe(probe);
    UpdateReflections(component);
}

void ReflectionSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::REFLECTION_COMPONENT_CHANGED)
    {
        DVASSERT(component->GetType() == ReflectedTypeDB::Get<ReflectionComponent>()->GetType());
        ReflectionComponent* reflectionComponent = static_cast<ReflectionComponent*>(component);
        MakeUniqueGlobalProbe(reflectionComponent);

        ReflectionProbe* probe = reflectionComponent->GetReflectionProbe();
        renderSystem->GetReflectionRenderer()->UnregisterReflectionProbe(probe);

        if (probe->IsStaticProbe())
            probe->SetCurrentTexture(nullptr);

        UpdateAndRegisterProbe(reflectionComponent, reflectionComponent->GetReflectionType());
    }
}

void ReflectionSystem::MakeUniqueGlobalProbe(ReflectionComponent* reflectionComponent)
{
    // make sure there is only one global probe on scene
    if ((reflectionComponent->GetReflectionType() == ReflectionProbe::ProbeType::GLOBAL) ||
        (reflectionComponent->GetReflectionType() == ReflectionProbe::ProbeType::GLOBAL_STATIC))
    {
        for (ReflectionComponent* component : allComponents)
        {
            bool isGlobal = (component->GetReflectionType() == ReflectionProbe::ProbeType::GLOBAL) ||
            (component->GetReflectionType() == ReflectionProbe::ProbeType::GLOBAL_STATIC);

            if (isGlobal && (component != reflectionComponent))
            {
                ReflectionProbe* probe = component->GetReflectionProbe();
                renderSystem->GetReflectionRenderer()->UnregisterReflectionProbe(probe);
                component->SetReflectionTypeSilently(ReflectionProbe::ProbeType::LOCAL);
                UpdateAndRegisterProbe(component, probe->IsStaticProbe() ? ReflectionProbe::ProbeType::LOCAL_STATIC : ReflectionProbe::ProbeType::LOCAL);
            }
        }
    }
}

void ReflectionSystem::AddEntity(Entity* entity)
{
    ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
    DVASSERT(reflectionComponent != nullptr);

    MakeUniqueGlobalProbe(reflectionComponent);

    reflectionComponent->SetReflectionProbe(new ReflectionProbe());
    allComponents.push_back(reflectionComponent);

    UpdateAndRegisterProbe(reflectionComponent, reflectionComponent->GetReflectionType());
}

void ReflectionSystem::RemoveEntity(Entity* entity)
{
    ReflectionComponent* reflectionComponent = entity->GetComponent<ReflectionComponent>();
    DVASSERT(reflectionComponent != nullptr);

    ReflectionProbe* reflectionProbe = reflectionComponent->GetReflectionProbe();
    renderSystem->GetReflectionRenderer()->UnregisterReflectionProbe(reflectionProbe);
    SafeRelease(reflectionProbe);

    FindAndRemoveExchangingWithLast(allComponents, reflectionComponent);
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
                UpdateReflections(entity->GetComponent<ReflectionComponent>());
            }
        }
    }

    for (ReflectionComponent* component : allComponents)
    {
        ReflectionProbe* probe = component->GetReflectionProbe();
        if (probe->ContainsUnprocessedSphericalHarmonics())
        {
            component->SetSphericalHarmonics(probe->GetSphericalHarmonicsArray());
            probe->MarkSphericalHarmonicsAsProcessed();
        }
    }
}
}
