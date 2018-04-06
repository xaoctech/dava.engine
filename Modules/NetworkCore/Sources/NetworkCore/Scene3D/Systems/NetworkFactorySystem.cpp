#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkFactoryComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkFactorySystem.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/NetworkFactoryUtils.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/TransformComponent.h"

#include "Debug/DVAssert.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkFactorySystem)
{
    ReflectionRegistrator<NetworkFactorySystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkFactorySystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 8.1f)]
    .End();
}

NetworkFactorySystem::NetworkFactorySystem(Scene* scene)
    : DAVA::SceneSystem(scene, ComponentUtils::MakeMask<NetworkFactoryComponent>())
    , factoryGroup(scene->AquireComponentGroup<NetworkFactoryComponent>())
    , factoryGroupPending(scene->AquireComponentGroupOnAdd(factoryGroup, this))
    , entityConfigManager(new EntityConfigManager())
{
}

void NetworkFactorySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkFactorySystem::ProcessFixed");
    for (NetworkFactoryComponent* fc : factoryGroupPending->components)
    {
        Entity* entity = fc->GetEntity();
        const String& name = fc->name;
        entity->SetName(name.c_str());
        const EntityCfg* entityCfg = entityConfigManager->GetEntityCfg(name);
        DVASSERT(entityCfg);

        NetworkReplicationComponent* replicationComponent = entity->GetComponent<NetworkReplicationComponent>();
        if (!replicationComponent)
        {
            DVASSERT(fc->replicationId.IsValid());
            const M::Privacy privacy = entityConfigManager->GetPrivacyByDomain(entityCfg->domainMask);
            replicationComponent = new NetworkReplicationComponent(fc->replicationId);
            replicationComponent->SetForReplication<NetworkFactoryComponent>(privacy);
            for (const auto& componentIt : entityCfg->components)
            {
                const ReflectedType* reflectedType = componentIt.first;
                const EntityCfg::ComponentCfg& componentCfg = componentIt.second;
                if (componentCfg.replicationPrivacy != M::Privacy::SERVER_ONLY)
                {
                    replicationComponent->SetForReplication(reflectedType->GetType(), componentCfg.replicationPrivacy);
                }
            }

            entity->AddComponent(replicationComponent);
        }

        const uint8 currentDomain = GetCurrentDomain(replicationComponent->GetNetworkID().GetPlayerId());
        if ((currentDomain & entityCfg->domainMask) == 0)
        {
            continue;
        }

        for (auto& modelDomain : entityCfg->models)
        {
            if (modelDomain.domainMask & currentDomain)
            {
                Entity* model = modelDomain.model->Clone();
                entity->AddNode(model);
                model->Release();
                break;
            }
        }

        TransformComponent* transComp = entity->GetComponent<TransformComponent>();
        DVASSERT(transComp);
        NetworkFactoryComponent::InitialTransform initTrans = { transComp->GetPosition(), transComp->GetRotation() };
        if (fc->initialTransformPtr)
        {
            initTrans = *fc->initialTransformPtr;
        }
        transComp->SetLocalTransform(initTrans.position, initTrans.rotation, transComp->GetScale() * fc->scale);

        Vector<const Type*> predictComponentTypes;
        for (const auto& componentIt : entityCfg->components)
        {
            const ReflectedType* reflectedType = componentIt.first;
            const EntityCfg::ComponentCfg& componentCfg = componentIt.second;
            const Type* type = reflectedType->GetType();
            if ((currentDomain & componentCfg.domainMask) == 0)
            {
                continue;
            }

            Component* component = entity->GetComponent(type);
            if (!component)
            {
                component = ComponentUtils::Create(type);
            }

            Reflection refComp = Reflection::Create(ReflectedObject(component));

            for (const auto& f : componentCfg.fields)
            {
                Reflection refField = refComp.GetField(f.name);
                refField.SetValueWithCast(f.value);
            }

            if (componentCfg.predictDomainMask & currentDomain)
            {
                predictComponentTypes.push_back(type);
            }

            entity->AddComponent(component);

            auto findIt = fc->componentTypeToOverrideData.find(type);
            if (findIt != fc->componentTypeToOverrideData.end())
            {
                NetworkFactoryComponent::OverrideFieldData& overrideFieldData = findIt->second;

                for (const NetworkFactoryComponent::FieldValue& fieldValue : overrideFieldData.fieldValues)
                {
                    refComp.GetField(fieldValue.name).SetValueWithCast(fieldValue.value);
                }

                if (overrideFieldData.callbackWithCast)
                {
                    overrideFieldData.callbackWithCast(component);
                }
            }
        }

        if (fc->overrideEntityDataCallback)
        {
            fc->overrideEntityDataCallback(entity);
        }

        if (!predictComponentTypes.empty())
        {
            ComponentMask predictionMask;
            for (const Type* type : predictComponentTypes)
            {
                predictionMask.Set(type);
            }

            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionMask);
            entity->AddComponent(networkPredictComponent);
        }
    }

    factoryGroupPending->components.clear();
}

uint8 NetworkFactorySystem::GetCurrentDomain(NetworkPlayerID playerId)
{
    using D = EntityCfg::Domain;
    if (IsServer(this))
    {
        return D::Server;
    }

    if (IsClient(this))
    {
        const NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
        DVASSERT(netGameModeComp);
        if (netGameModeComp->GetNetworkPlayerID() == playerId)
        {
            return D::ClientOwner;
        }

        return D::ClientNotOwner;
    }
};
}
