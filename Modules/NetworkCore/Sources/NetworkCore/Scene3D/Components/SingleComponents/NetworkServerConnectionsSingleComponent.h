#pragma once

#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "Base/UnordererMap.h"
#include "Entity/SingleComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class NetworkServerConnectionsSingleComponent : public ClearableSingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkServerConnectionsSingleComponent, ClearableSingleComponent);

    struct ServerRecvPacket
    {
        FastName token;
        Vector<uint8> data;
    };

    NetworkServerConnectionsSingleComponent();

    void AddConnectedToken(const FastName& token);
    void RemoveConnectedToken(const FastName& token);

    const UnorderedSet<FastName>& GetConnectedTokens() const;
    const Vector<FastName>& GetJustConnectedTokens() const;
    const Vector<FastName>& GetJustDisconnectedTokens() const;

    void StoreRecvPacket(uint8 channel, const FastName& token, const uint8* data, size_t size);
    const Vector<ServerRecvPacket>& GetRecvPackets(uint8 channel) const;

private:
    Vector<FastName> justConnectedTokens;
    Vector<FastName> justDisconnectedTokens;
    UnorderedSet<FastName> connectedTokens;
    Vector<Vector<ServerRecvPacket>> recvPacketsByChannels;

    void Clear() override;
};
}
