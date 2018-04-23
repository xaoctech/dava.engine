#pragma once

#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "Entity/SingleComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class NetworkServerConnectionsSingleComponent : public ClearableSingleComponent, public INetworkEventStorage
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkServerConnectionsSingleComponent, ClearableSingleComponent);

    struct ServerRecvPacket
    {
        FastName token;
        Vector<uint8> data;
    };

    NetworkServerConnectionsSingleComponent();

    const Vector<FastName>& GetConfirmedTokens() const;
    const UnorderedSet<FastName>& GetConnectedTokens() const;
    const Vector<FastName>& GetJustConnectedTokens() const;
    const Vector<FastName>& GetJustDisconnectedTokens() const;
    const Vector<ServerRecvPacket>& GetRecvPackets(const PacketParams::Channels channel) const;

private:
    void AddConnectedToken(const FastName& token) final;
    void StoreRecvPacket(const PacketParams::Channels channel, const FastName& token, const void* data, size_t size) final;
    void RemoveConnectedToken(const FastName& token) final;
    void ConfirmToken(const FastName& token) final;

    Vector<FastName> confirmedTokens;

    Vector<FastName> justConnectedTokens;
    Vector<FastName> justDisconnectedTokens;
    UnorderedSet<FastName> connectedTokens;
    Vector<Vector<ServerRecvPacket>> recvPacketsByChannels;

    void Clear() override;
};
}
