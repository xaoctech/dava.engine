#include "NetworkClientConnectionSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "Logger/Logger.h"
#include "Base/BitReader.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkClientConnectionSingleComponent)
{
    ReflectionRegistrator<NetworkClientConnectionSingleComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

NetworkClientConnectionSingleComponent::NetworkClientConnectionSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::FixedProcesses)
    , recvPacketsByChannels(PacketParams::CHANNELS_COUNT)
{
    for (int32 i = 0; i < PacketParams::CHANNELS_COUNT; ++i)
    {
        recvPacketsByChannels.emplace_back();
    }
}

bool NetworkClientConnectionSingleComponent::IsConnected() const
{
    return isConnected;
}

void NetworkClientConnectionSingleComponent::SetIsConnected(bool value)
{
    if (isConnected && value)
    {
        SetIsJustDisconnected(true);
    }
    isConnected = value;
}

bool NetworkClientConnectionSingleComponent::IsJustConnected() const
{
    return isJustConnected;
}

void NetworkClientConnectionSingleComponent::SetIsJustConnected(bool value)
{
    isJustConnected = value;
}

bool NetworkClientConnectionSingleComponent::IsJustDisconnected() const
{
    return isJustDisconnected;
}

void NetworkClientConnectionSingleComponent::SetIsJustDisconnected(bool value)
{
    isJustDisconnected = value;
}

void NetworkClientConnectionSingleComponent::StoreRecvPacket(uint8 channel, const uint8* data, size_t size)
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    ClientRecvPacket packet(size);
    packet.reserve(size + BitReader::AccumulatorTypeByteSize);
    Memcpy(packet.data(), data, size);
    recvPacketsByChannels[channel].push_back(std::move(packet));
}

const Vector<NetworkClientConnectionSingleComponent::ClientRecvPacket>& NetworkClientConnectionSingleComponent::GetRecvPackets(uint8 channel) const
{
    DVASSERT(channel < PacketParams::CHANNELS_COUNT);
    return recvPacketsByChannels[channel];
}

void NetworkClientConnectionSingleComponent::Clear()
{
    isJustConnected = false;
    isJustDisconnected = false;

    for (auto& recvPackets : recvPacketsByChannels)
    {
        recvPackets.clear();
    }
}
}
