#include "NetworkServerConnectionsSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "Logger/Logger.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkServerConnectionsSingleComponent)
{
    ReflectionRegistrator<NetworkServerConnectionsSingleComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

NetworkServerConnectionsSingleComponent::NetworkServerConnectionsSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::FixedProcesses)
    , recvPacketsByChannels(PacketParams::CHANNELS_COUNT)
{
    for (int32 i = 0; i < PacketParams::CHANNELS_COUNT; ++i)
    {
        recvPacketsByChannels.emplace_back();
    }
}

void NetworkServerConnectionsSingleComponent::AddConnectedToken(const FastName& token)
{
    auto ret = connectedTokens.insert(token);
    if (!ret.second)
    {
        RemoveConnectedToken(token);
    }
    justConnectedTokens.push_back(token);
}

void NetworkServerConnectionsSingleComponent::RemoveConnectedToken(const FastName& token)
{
    if (connectedTokens.erase(token) > 0)
    {
        justDisconnectedTokens.push_back(token);
    }
}

const UnorderedSet<FastName>& NetworkServerConnectionsSingleComponent::GetConnectedTokens() const
{
    return connectedTokens;
}

const Vector<FastName>& NetworkServerConnectionsSingleComponent::GetJustConnectedTokens() const
{
    return justConnectedTokens;
}

const Vector<FastName>& NetworkServerConnectionsSingleComponent::GetJustDisconnectedTokens() const
{
    return justDisconnectedTokens;
}

void NetworkServerConnectionsSingleComponent::StoreRecvPacket(uint8 channel, const FastName& token, const uint8* data, size_t size)
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    ServerRecvPacket packet{ token, Vector<uint8>(size) };
    Memcpy(packet.data.data(), data, size);
    recvPacketsByChannels[channel].push_back(std::move(packet));
}

const Vector<NetworkServerConnectionsSingleComponent::ServerRecvPacket>& NetworkServerConnectionsSingleComponent::GetRecvPackets(uint8 channel) const
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    return recvPacketsByChannels[channel];
}

void NetworkServerConnectionsSingleComponent::Clear()
{
    justConnectedTokens.clear();
    justDisconnectedTokens.clear();

    for (auto& recvPackets : recvPacketsByChannels)
    {
        recvPackets.clear();
    }
}
}
