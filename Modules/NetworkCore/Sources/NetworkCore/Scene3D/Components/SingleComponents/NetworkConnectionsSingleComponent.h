#pragma once

#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "Base/UnordererMap.h"
#include "Entity/SingleComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class NetworkConnectionsSingleComponent : public ClearableSingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkConnectionsSingleComponent, ClearableSingleComponent);

    struct RecvPacket
    {
        FastName token;
        Vector<uint8> data;
    };

    NetworkConnectionsSingleComponent();

    void AddConnectedToken(const FastName& token);
    void RemoveConnectedToken(const FastName& token);

    const UnorderedSet<FastName>& GetConnectedTokens() const;
    const Vector<FastName>& GetJustConnectedTokens() const;
    const Vector<FastName>& GetJustDisconnectedTokens() const;

    void StoreRecvPacket(uint8 channel, const FastName& token, const uint8* data, size_t size);
    const Vector<RecvPacket>& GetRecvPackets(uint8 channel) const;

private:
    Vector<FastName> justConnectedTokens;
    Vector<FastName> justDisconnectedTokens;
    UnorderedSet<FastName> connectedTokens;
    Vector<Vector<RecvPacket>> recvPacketsByChannels;

    void Clear() override;
};
}
