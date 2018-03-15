#include "NetworkConnectionsSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "Logger/Logger.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkConnectionsSingleComponent)
{
    ReflectionRegistrator<NetworkConnectionsSingleComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

NetworkConnectionsSingleComponent::NetworkConnectionsSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::AllProcesses)
    , recvPacketsByChannels(PacketParams::CHANNELS_COUNT)
{
    for (int32 i = 0; i < PacketParams::CHANNELS_COUNT; ++i)
    {
        recvPacketsByChannels.emplace_back();
    }
}

void NetworkConnectionsSingleComponent::AddConnectedToken(const FastName& token)
{
    auto ret = connectedTokens.insert(token);
    if (!ret.second)
    {
        RemoveConnectedToken(token);
    }
    justConnectedTokens.push_back(token);
}

void NetworkConnectionsSingleComponent::RemoveConnectedToken(const FastName& token)
{
    if (connectedTokens.erase(token) > 0)
    {
        justDisconnectedTokens.push_back(token);
    }
}

const UnorderedSet<FastName>& NetworkConnectionsSingleComponent::GetConnectedTokens() const
{
    return connectedTokens;
}

const Vector<FastName>& NetworkConnectionsSingleComponent::GetJustConnectedTokens() const
{
    return justConnectedTokens;
}

const Vector<FastName>& NetworkConnectionsSingleComponent::GetJustDisconnectedTokens() const
{
    return justDisconnectedTokens;
}

void NetworkConnectionsSingleComponent::StoreRecvPacket(uint8 channel, const FastName& token, const uint8* data, size_t size)
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    RecvPacket packet{ token, Vector<uint8>(size) };
    Memcpy(packet.data.data(), data, size);
    recvPacketsByChannels[channel].push_back(std::move(packet));
}

const Vector<NetworkConnectionsSingleComponent::RecvPacket>& NetworkConnectionsSingleComponent::GetRecvPackets(uint8 channel) const
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    return recvPacketsByChannels[channel];
}

void NetworkConnectionsSingleComponent::Clear()
{
    justConnectedTokens.clear();
    justDisconnectedTokens.clear();

    for (auto& recvPackets : recvPacketsByChannels)
    {
        recvPackets.clear();
    }
}
}
