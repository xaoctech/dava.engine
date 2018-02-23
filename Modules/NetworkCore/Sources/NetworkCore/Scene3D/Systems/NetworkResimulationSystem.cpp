#include "NetworkResimulationSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Entity/SystemManager.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <Scene3D/Components/SingleComponents/ChangedSystemsSingleComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkResimulationSystem)
{
    ReflectionRegistrator<NetworkResimulationSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkResimulationSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 11.0f)]
    .Method("Process", &NetworkResimulationSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 11.1f)]
    .End();
}

NetworkResimulationSystem::NetworkResimulationSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent>() | ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    timeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    predictionComp = GetScene()->GetSingletonComponent<NetworkPredictionSingleComponent>();
    ssc = GetScene()->GetSingletonComponent<SnapshotSingleComponent>();
    changedSystemsSingleComponent = scene->GetSingletonComponent<ChangedSystemsSingleComponent>();
}

void NetworkResimulationSystem::Process(float32 timeElapsed)
{
    if (!changedSystemsSingleComponent->addedSystems.empty() ||
        !changedSystemsSingleComponent->removedSystems.empty())
    {
        isCollectedSystems = false;
    }
}

void NetworkResimulationSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::Process");

    if (!isCollectedSystems)
    {
        CollectSystems();
    }

    const uint32 maxFrameId = timeComp->GetFrameId();

    if (predictionComp->mispredictedEntities.size() > 0)
    {
        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#> Resimulate\n");
        for (auto it : predictionComp->mispredictedEntities)
        {
            Entity* entity = it.first;
            uint32 frameId = it.second.frameId;
            NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);

            Snapshot* serverSnapshot = ssc->GetServerSnapshot(frameId);
            DVASSERT(nullptr != serverSnapshot);

            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "##> Resimulate Entity " << entityId << " (" << frameId << " -> " << maxFrameId << ")\n");
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "## Rolling back Entity " << entityId << " to " << frameId << "\n");

            // revert entity to the specified frameId state
            SnapshotUtils::ApplySnapshot(serverSnapshot, entityId, entity);

            Vector<ISimulationSystem*> affectedSystems = {};
            affectedSystems.reserve(systems.size());
            for (auto& r : systems)
            {
                const ComponentMask& requiredComponents = r.system->GetResimulationComponents();
                bool isAffectedSystems = ((requiredComponents & entity->GetAvailableComponentMask()) == requiredComponents);
                if (isAffectedSystems)
                {
                    r.system->ReSimulationStart(entity, frameId);
                    affectedSystems.push_back(r.system);
                }
            }

            const char* entName = entity->GetName().c_str();
            Logger::Debug("[ReSimulation] entity:%lu:%s from %d to %d", static_cast<uint32>(entityId), entName, frameId, maxFrameId);
            ++resimulationsCount;

            for (++frameId; frameId <= maxFrameId; ++frameId)
            {
                LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##> Simulate Entity " << entityId << " frame " << frameId << "\n");

                for (auto& system : affectedSystems)
                {
                    system->Simulate(entity);
                }

                LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##> Simulate Done" << std::endl);
            }

            for (auto& system : affectedSystems)
            {
                system->ReSimulationEnd(entity);
            }
        }

        GetScene()->GetSystem<TransformSystem>()->Process(0);

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#< Resimulate Done\n");
    }
}

void NetworkResimulationSystem::CollectSystems()
{
    systems.clear();

    for (SceneSystem* system : GetScene()->systemsVector)
    {
        ISimulationSystem* isys = dynamic_cast<ISimulationSystem*>(system);

        if (isys != nullptr)
        {
            const ReflectedStructure* structure = ReflectedTypeDB::GetByPointer(system)->GetStructure();

            for (const auto& method : structure->methods)
            {
                if (method->meta != nullptr && method.get()->name != FastName("ProcessServer")) // This is ugly, but the only way right now.
                {
                    const M::SystemProcess* meta = method->meta->GetMeta<M::SystemProcess>();

                    if (meta->type == SP::Type::FIXED)
                    {
                        ResimulationSystem tmp;
                        tmp.system = isys;
                        tmp.order = meta->order;
                        tmp.group = meta->group;

                        systems.push_back(tmp);
                    }
                }
            }
        }
    }

    std::sort(systems.begin(), systems.end(), [](const ResimulationSystem& l, const ResimulationSystem& r) {
        return l.group == r.group ? l.order < r.order : l.group < r.group;
    });

    isCollectedSystems = true;
}

void NetworkResimulationSystem::PrepareForRemove()
{
}

uint32 NetworkResimulationSystem::GetResimulationsCount() const
{
    return resimulationsCount;
}
}
