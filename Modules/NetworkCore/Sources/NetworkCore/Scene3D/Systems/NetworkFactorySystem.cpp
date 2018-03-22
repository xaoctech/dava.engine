#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkFactoryComponent.h"
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
{
}

void NetworkFactorySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkFactorySystem::Process");
    for (NetworkFactoryComponent* fc : factoryGroupPending->components)
    {
        Entity* entity = fc->GetEntity();
        const EntityCfg& entityCfg = EntityCfg::LoadFromYaml(fc->name);
        const uint8 domainMask = GetDomainMask(entity, fc->playerId);
        if ((domainMask & entityCfg.domainMask) == 0)
        {
            continue;
        }

        Entity* model = entityCfg.model->Clone();
        entity->AddNode(model);
        model->Release();

        if (fc->initialTransformPtr)
        {
            TransformComponent* transComp = entity->GetComponent<TransformComponent>();
            DVASSERT(transComp);
            if (transComp)
            {
                const NetworkFactoryComponent::InitialTransform& src = *fc->initialTransformPtr;
                transComp->SetLocalTransform(src.position, src.rotation, transComp->GetScale());
            }
        }

        Vector<const Type*> predictComponentTypes;
        for (const auto& componentIt : entityCfg.components)
        {
            const ReflectedType* reflectedType = componentIt.first;
            const EntityCfg::ComponentCfg& componentCfg = componentIt.second;
            if ((domainMask & componentCfg.domainMask) == 0)
            {
                continue;
            }

            if (entity->GetComponent(reflectedType->GetType()))
            {
                Logger::Error("Duplicate component:%s", reflectedType->GetPermanentName().c_str());
                continue;
            }

            Component* component = ComponentUtils::Create(reflectedType->GetType());
            Reflection refComp = Reflection::Create(ReflectedObject(component));

            for (const auto& f : componentCfg.fields)
            {
                Reflection refField = refComp.GetField(f.name);
                refField.SetValueWithCast(f.value);
            }

            if (componentCfg.predictDomainMask & domainMask)
            {
                predictComponentTypes.push_back(reflectedType->GetType());
            }

            entity->AddComponent(component);

            auto findIt = fc->componentTypeToOverrideData.find(reflectedType->GetType());
            if (findIt != fc->componentTypeToOverrideData.end())
            {
                NetworkFactoryComponent::OverrideFieldData& overrideFieldData = findIt->second;

                for (const NetworkFactoryComponent::FieldValue& fieldValue : overrideFieldData.fieldValues)
                {
                    refComp.GetField(fieldValue.name).SetValueWithCast(fieldValue.value);
                }

                if (overrideFieldData.callbackHolder)
                {
                    overrideFieldData.callbackHolder->Invoke(component);
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

uint8 NetworkFactorySystem::GetDomainMask(Entity* entity, NetworkPlayerID playerId)
{
    using D = EntityCfg::Domain;
    uint8 domainMask = 0;
    if (IsServer(this))
    {
        domainMask |= D::Server;
    }

    if (IsClient(this))
    {
        domainMask |= D::Client;

        const NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
        DVASSERT(netGameModeComp);
        if (netGameModeComp->GetNetworkPlayerID() == playerId)
        {
            domainMask |= D::ClientOwner;
        }
        else
        {
            domainMask |= D::ClientNotOwner;
        }
    }

    return domainMask;
};
}
