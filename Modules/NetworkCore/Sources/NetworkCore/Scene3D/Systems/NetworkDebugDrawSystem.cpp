#include "NetworkDebugDrawSystem.h"

#include <Entity/ComponentUtils.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Snapshot.h>

#include <Render/Highlevel/RenderSystem.h>
#include <Scene3D/Systems/DebugRenderSystem.h>
#include <Logger/Logger.h>

#include <algorithm>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDebugDrawSystem)
{
    ReflectionRegistrator<NetworkDebugDrawSystem>::Begin()[M::Tags("network", "debugdraw")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkDebugDrawSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 20.0f)]
    .End();
}

NetworkDebugDrawSystem::NetworkDebugDrawSystem(Scene* scene)
    : SceneSystem(scene,
                  ComponentUtils::MakeMask<NetworkDebugDrawComponent>()
                  | ComponentUtils::MakeMask<NetworkTransformComponent>()
                  | ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
}

void NetworkDebugDrawSystem::AddEntity(Entity* entity)
{
    if (!entity->GetComponent<NetworkPredictComponent>())
    {
        entityToTransforms[entity];
    }
}

void NetworkDebugDrawSystem::RegisterComponent(Entity* entity, Component* component)
{
    SceneSystem::RegisterComponent(entity, component);
    if (component->GetType()->Is<NetworkPredictComponent>())
    {
        entityToTransforms.erase(entity);
    }
}

void NetworkDebugDrawSystem::RemoveEntity(Entity* entity)
{
    entityToTransforms.erase(entity);
}

void NetworkDebugDrawSystem::Process(DAVA::float32 timeElapsed)
{
    NetworkReplicationSingleComponent* replicationSingleComponent = GetScene()->GetSingletonComponent<NetworkReplicationSingleComponent>();
    //const NetworkReplicationSingleComponent::EntityToFrames& entityToFrames = replicationSingleComponent->receivedOnCurrentFrame;
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

//    DVASSERT(false);

#if 0
    for (auto& entityToTransformIt : entityToTransforms)
    {
        Entity* entity = entityToTransformIt.first;
        FrameToTransform& frameToTransform = entityToTransformIt.second;
        const NetworkTransformComponent* netTransComp = entity->GetComponent<NetworkTransformComponent>();

        const NetworkID networkID = entity->GetComponent<NetworkReplicationComponent>()->GetNetworkID();

        auto findEntityIt = entityToFrames.find(networkID);
        if (findEntityIt != entityToFrames.end())
        {
            const Set<uint32>& frames = findEntityIt->second;
            DVASSERT(!frames.empty());
            const uint32 frameId = *frames.rbegin();

            if (!frameToTransform.empty())
            {
                auto transformIt = frameToTransform.rbegin();
                const uint32 transformFrameId = transformIt->first;
                Transform& transform = transformIt->second;

                const uint32 deltaFrames = std::min(static_cast<uint32>(32), frameId - transformFrameId);
                if (deltaFrames > MAX_LOST_FRAMES)
                {
                    const float32 tDiff = 1.0f / deltaFrames;

                    Transform lateTransform;
                    lateTransform.numberOfLost = deltaFrames;

                    for (size_t i = 1; i < deltaFrames; ++i)
                    {
                        const uint32 lateFrameId = transformFrameId + i;
                        if (frameToTransform.find(lateFrameId) == frameToTransform.end())
                        {
                            const float32 t = tDiff * i;
                            lateTransform.position.Lerp(netTransComp->GetPosition(), transform.position, t);
                            lateTransform.orientation.Slerp(netTransComp->GetOrientation(), transform.orientation, t);
                            frameToTransform.emplace(lateFrameId, lateTransform);
                        }
                    }
                }
            }

            Transform realTransform;
            realTransform.orientation = netTransComp->GetOrientation();
            realTransform.position = netTransComp->GetPosition();
            frameToTransform.emplace(frameId, realTransform);
        }

        for (auto frameToTransformIt = frameToTransform.begin(); frameToTransformIt != frameToTransform.end();)
        {
            uint32 transformFrameId = frameToTransformIt->first;
            Transform& transform = frameToTransformIt->second;
            if (transform.ttl > 0)
            {
                --transform.ttl;
                ++frameToTransformIt;

                if (!SHOW_ERROR_ONLY || transform.numberOfLost)
                {
                    const Matrix4 matrix = transform.orientation.GetMatrix() * Matrix4::MakeTranslation(transform.position);
                    const AABBox3& box = entity->GetComponent<NetworkDebugDrawComponent>()->box;
                    Color color = REAL_COLOR;
                    if (transform.numberOfLost)
                    {
                        color = LATE_COLOR;
                        color.a = 0.1f * transform.numberOfLost;
                    }
                    drawer->DrawAABoxTransformed(box, matrix, color, DAVA::RenderHelper::eDrawType::DRAW_SOLID_DEPTH);
                }
            }
            else
            {
                frameToTransformIt = frameToTransform.erase(frameToTransformIt);
            }
        }
    }
#endif
}

} //namespace DAVA
