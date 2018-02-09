#include "NetworkDebugPredictDrawSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h>
#include <NetworkCore/Snapshot.h>
#include <NetworkCore/SnapshotUtils.h>

#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Scene3D/Systems/DebugRenderSystem.h>

#include <algorithm>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDebugPredictDrawSystem)
{
    ReflectionRegistrator<NetworkDebugPredictDrawSystem>::Begin()[M::Tags("network", "debugdraw")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkDebugPredictDrawSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 19.0f)]
    .End();
}

NetworkDebugPredictDrawSystem::NetworkDebugPredictDrawSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent>()
                  | ComponentUtils::MakeMask<NetworkDebugDrawComponent>()
                  | ComponentUtils::MakeMask<NetworkReplicationComponent>())
    , netTransCompId(ComponentUtils::GetRuntimeIndex<NetworkTransformComponent>())
{
    tmpNetTransComp.reset(new NetworkTransformComponent);
}

void NetworkDebugPredictDrawSystem::AddEntity(Entity* entity)
{
    entityToLastTransform[entity] = entity->GetLocalTransform();
}

void NetworkDebugPredictDrawSystem::RemoveEntity(Entity* entity)
{
    entityToLastTransform.erase(entity);
}

void NetworkDebugPredictDrawSystem::Process(float32 timeElapsed)
{
    NetworkPredictionSingleComponent* predictionComp = GetScene()->GetSingletonComponent<NetworkPredictionSingleComponent>();
    SnapshotSingleComponent* snapshotComp = GetScene()->GetSingletonComponent<SnapshotSingleComponent>();
    NetworkReplicationSingleComponent* replicationSingleComponent = GetScene()->GetSingletonComponent<NetworkReplicationSingleComponent>();

    const NetworkReplicationSingleComponent::EntityToInfo& entityToReplInfo = replicationSingleComponent->replicationInfo;
    const NetworkPredictionSingleComponent::EntityToMisprediction& entityToMisprediction = predictionComp->mispredictedEntities;

    auto getTmpTransform = [this]() { return tmpNetTransComp->GetOrientation().GetMatrix() * Matrix4::MakeTranslation(tmpNetTransComp->GetPosition()); };

    for (auto& it : entityToLastTransform)
    {
        Entity* entity = it.first;
        Matrix4& transform = it.second;
        const NetworkID networkID = entity->GetComponent<NetworkReplicationComponent>()->GetNetworkID();

        auto findEntityIt = entityToReplInfo.find(networkID);
        if (findEntityIt == entityToReplInfo.end())
        {
            DrawBox(entity, transform, DAVA::Color(0.0f, 1.0f, 0.0f, 0.2f));
        }
        else
        {
            const NetworkReplicationSingleComponent::EntityReplicationInfo& info = findEntityIt->second;
            const uint32 frameId = info.frameIdServer;
            SnapshotComponentKey componentKey(netTransCompId, 0);
            SnapshotUtils::ApplySnapshot(snapshotComp->GetServerSnapshot(frameId), networkID, componentKey, tmpNetTransComp.get());

            transform = std::move(getTmpTransform());
            DrawBox(entity, transform, DAVA::Color(0.0f, 1.0f, 0.0f, 0.2f));

            auto findParamsIt = entityToMisprediction.find(entity);
            if (findParamsIt != entityToMisprediction.end())
            {
                const EntityMisprediction& misprediction = findParamsIt->second;
                const Vector<SnapshotComponentKey>& componentKeys = misprediction.components;
                if (std::find(componentKeys.begin(), componentKeys.end(), componentKey) != componentKeys.end())
                {
                    SnapshotUtils::ApplySnapshot(snapshotComp->GetClientSnapshot(misprediction.frameId), networkID, componentKey, tmpNetTransComp.get());
                    DrawBox(entity, getTmpTransform(), DAVA::Color(1.0f, 0.0f, 0.0f, 0.2f));
                }
            }
        }
    }
}

void NetworkDebugPredictDrawSystem::DrawBox(const Entity* entity, const Matrix4& transform, const Color& color)
{
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    const AABBox3& box = entity->GetComponent<NetworkDebugDrawComponent>()->box;
    drawer->DrawAABoxTransformed(box, transform, color, DAVA::RenderHelper::eDrawType::DRAW_SOLID_DEPTH);
};

} //namespace DAVA
