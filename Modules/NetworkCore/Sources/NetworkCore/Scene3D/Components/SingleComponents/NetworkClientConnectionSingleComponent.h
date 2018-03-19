#pragma once

#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "Base/UnordererMap.h"
#include "Entity/SingleComponent.h"

namespace DAVA
{
class NetworkClientConnectionSingleComponent : public ClearableSingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkClientConnectionSingleComponent, ClearableSingleComponent);

    using ClientRecvPacket = Vector<uint8>;

    NetworkClientConnectionSingleComponent();

    bool IsConnected() const;
    void SetIsConnected(bool value);

    bool IsJustConnected() const;
    void SetIsJustConnected(bool value);

    bool IsJustDisconnected() const;
    void SetIsJustDisconnected(bool value);

    void StoreRecvPacket(uint8 channel, const uint8* data, size_t size);
    const Vector<ClientRecvPacket>& GetRecvPackets(uint8 channel) const;

private:
    bool isConnected = false;
    bool isJustConnected = false;
    bool isJustDisconnected = false;
    Vector<Vector<ClientRecvPacket>> recvPacketsByChannels;

    void Clear() override;
};
}
