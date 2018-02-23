#include "Scene3D/Systems/DebugRenderSystem.h"

#include "Debug/DVAssert.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DebugRenderSystem)
{
    ReflectionRegistrator<DebugRenderSystem>::Begin()[M::Tags("base", "debug")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &DebugRenderSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 10.0f)]
    .End();
}

DebugRenderSystem::DebugRenderSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<DebugRenderComponent>())
{
    renderSystem = scene->GetRenderSystem();
}

DebugRenderSystem::~DebugRenderSystem()
{
}

void DebugRenderSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_DEBUG_RENDER_SYSTEM);

    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();
    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<DebugRenderComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                // Update new transform pointer, and mark that transform is changed
                const Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldTransformPtr();

                RenderObject* object = entity->GetComponent<DebugRenderComponent>()->GetRenderObject();
                if (nullptr != object)
                {
                    object->SetWorldTransformPtr(worldTransformPointer);
                    entity->GetScene()->renderSystem->MarkForUpdate(object);
                }
            }
        }
    }
}

void DebugRenderSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        RenderObject* renderObject = entity->GetComponent<DebugRenderComponent>()->GetRenderObject();
        if (!renderObject)
            return;
        const Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldTransformPtr();
        renderObject->SetWorldTransformPtr(worldTransformPointer);

        componentRenderObjectMap[component] = renderObject;
        GetScene()->GetRenderSystem()->RenderPermanent(renderObject);
    }
}

void DebugRenderSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        RenderObject* renderObject = componentRenderObjectMap.at(component);
        if (!renderObject)
        {
            return;
        }

        GetScene()->GetRenderSystem()->RemoveFromRender(renderObject);
        componentRenderObjectMap.erase(component);
    }
}

void DebugRenderSystem::PrepareForRemove()
{
    componentRenderObjectMap.clear();
}
}
