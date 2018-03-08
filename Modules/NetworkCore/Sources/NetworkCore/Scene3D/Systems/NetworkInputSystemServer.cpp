#ifdef SERVER

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkStatisticsSingleComponent.h"
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkInputSystem)
{
    ReflectionRegistrator<NetworkInputSystem>::Begin()[M::Tags("network", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 18.0f)]
    .End();
}

NetworkInputSystem::NetworkInputSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkInputComponent>())
{
    IServer* server = scene->GetSingleComponentForRead<NetworkServerSingleComponent>(this)->GetServer();

    DVASSERT(server != nullptr);

    server->SubscribeOnReceive(PacketParams::INPUT_CHANNEL_ID, OnServerReceiveCb(this, &NetworkInputSystem::OnReceive));
    server->SubscribeOnConnect(OnServerConnectCb(this, &NetworkInputSystem::OnConnect));
}

void NetworkInputSystem::AddEntity(Entity* entity)
{
    const NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    entitiesToBuffers.emplace(entity, NetworkInputBuffer(NetworkTimeSingleComponent::FrequencyHz, netTimeComp->GetFrameId()));
}

void NetworkInputSystem::RemoveEntity(Entity* entity)
{
    entitiesToBuffers.erase(entity);
}

void NetworkInputSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkInputSystem::ProcessFixed");
    NetworkStatisticsSingleComponent* statsComp = GetScene()->GetSingleComponentForWrite<NetworkStatisticsSingleComponent>(this);
    if (statsComp)
    {
        statsComp->UpdateFrameTimestamps();
    }

    const NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    for (auto& entityToBuffer : entitiesToBuffers)
    {
        Entity* entity = entityToBuffer.first;
        NetworkInputBuffer& inputBuffer = entityToBuffer.second;
        auto popRes = inputBuffer.Front();
        int32 frameDiff = 0;
        if (popRes.frameId > 0)
        {
            frameDiff = netTimeComp->GetFrameId() - popRes.frameId;
        }
        for (int32 i = 0; i <= frameDiff; ++i)
        {
            auto popRes = inputBuffer.Pop();
            ActionsSingleComponent::Actions actions;
            if (popRes.value)
            {
                actions.digitalActions = UnpackDigitalActions(popRes.value->actions, GetScene());
                actions.analogActions = UnpackAnalogActions(popRes.value->actions, popRes.value->analogStates, GetScene());
#ifdef DISABLE_LOSSY_PACK
                actions.cameraDelta = popRes.value->cameraDelta.data;
#else
                actions.cameraDelta.Unpack(popRes.value->cameraDelta.GetData());
#endif
                actions.clientFrameId = popRes.frameId;

                if (statsComp)
                {
                    const NetworkReplicationComponent* netReplicationComponent = entity->GetComponent<NetworkReplicationComponent>();
                    const NetworkPlayerID playerID = netReplicationComponent->GetNetworkPlayerID();
                    uint64 tsKey = NetStatTimestamps::GetKey(popRes.frameId, playerID);
                    NetStatTimestamps* timestamps = statsComp->GetTimestamps(tsKey);
                    if (timestamps)
                    {
                        timestamps->server.getFromBuf = SystemTimer::GetMs();
                    }
                }
            }

            AddActionsForClient(this, entity, std::move(actions));
        }
    }
}

void NetworkInputSystem::OnConnect(const Responder& responder)
{
    auto tokenIt = tokensToEntities.find(responder.GetToken());
    if (tokenIt != tokensToEntities.end())
    {
        UnorderedSet<Entity*>& entities = tokenIt->second;
        for (auto entityIt = entities.begin(); entityIt != entities.end();)
        {
            auto bufferIt = entitiesToBuffers.find(*entityIt);
            if (bufferIt != entitiesToBuffers.end())
            {
                bufferIt->second.Clear();
                ++entityIt;
            }
            else
            {
                entities.erase(entityIt++);
            }
        }
    }
}

void NetworkInputSystem::OnReceive(const Responder& responder, const uint8* data, size_t size)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkInputSystem::OnReceive");
    const NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingleComponentForRead<NetworkEntitiesSingleComponent>(this);

    using NetInputData = NetworkInputComponent::Data;
    const InputPacketHeader* header = reinterpret_cast<const InputPacketHeader*>(data);
    uint32 offset = INPUT_PACKET_HEADER_SIZE;
    Vector<NetInputData> inputData;

    for (uint32 i = 0; i < header->framesCount; ++i)
    {
        if (i > 0 && data[offset] == DUPLICATE_INPUT_MARK)
        {
            inputData.push_back(inputData.back());
            offset += 1;
            continue;
        }
        const NetInputData* netInputData = reinterpret_cast<const NetInputData*>(data + offset);
        inputData.push_back(*netInputData);
        offset += sizeof(NetInputData);
    }

    Entity* entity = networkEntities->FindByID(header->entityId);
    auto entityIt = entitiesToBuffers.find(entity);
    if (entityIt == entitiesToBuffers.end())
    {
        Logger::Error("Received the input for the unknown entity %d", header->entityId);
        return;
    }
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingleComponentForWrite<NetworkTimeSingleComponent>(this);
    uint32 frameId = netTimeComp->GetFrameId();
    netTimeComp->SetClientOutrunning(responder.GetToken(), header->frameId - frameId);
    entityIt->second.Update(inputData, header->frameId);
    auto tokenIt = tokensToEntities.find(responder.GetToken());
    if (tokenIt == tokensToEntities.end())
    {
        tokenIt = tokensToEntities.emplace(responder.GetToken(), UnorderedSet<Entity*>()).first;
    }
    tokenIt->second.insert(entity);

    NetworkStatisticsSingleComponent* statsComp = GetScene()->GetSingleComponentForWrite<NetworkStatisticsSingleComponent>(this);
    if (header->hasNetStat)
    {
        const NetStatTimestamps* netStatData = reinterpret_cast<const NetStatTimestamps*>(data + offset);
        std::unique_ptr<NetStatTimestamps> timestamps(new NetStatTimestamps(*netStatData));
        timestamps->server.recvFromNet = SystemTimer::GetMs();
        const NetworkPlayerID playerID = GetScene()->GetSingleComponentForRead<NetworkGameModeSingleComponent>(this)->GetNetworkPlayerID(responder.GetToken());
        uint64 tsKey = NetStatTimestamps::GetKey(header->frameId, playerID);

        statsComp->AddTimestamps(tsKey, std::move(timestamps));
    }

    // statistics
    int16 frameDiff = header->frameId - frameId;
    statsComp->FlushFrameOffsetMeasurement(frameId, frameDiff);
}
}
#endif
