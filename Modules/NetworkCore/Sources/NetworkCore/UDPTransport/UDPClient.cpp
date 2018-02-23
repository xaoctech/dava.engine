#include "UDPClient.h"

#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_REFLECTION_IMPL(UDPClient)
{
    ReflectionRegistrator<UDPClient>::Begin()
    .Field("isConnected", &UDPClient::IsConnected, nullptr)
    .Field("ping", &UDPClient::GetPing, nullptr)
    .End();
}

UDPClient::UDPClient(const DAVA::String& hostName, uint16 port, const FastName& token_, size_t peerCount, int8 connAttempts)
    : maxConnAttempts(connAttempts)
    , connAttempts(connAttempts)
    , wasConnected(false)
    , channelCount(PacketParams::CHANNELS_COUNT)
    , rcvBuffer(new uint8[GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE)])
    , sndBuffer(new uint8[GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE)])
{
    client = enet_host_create(nullptr, peerCount, 0, 0, 0);
    ThrowIfENetError(client, "CLIENT_ENET_HOST_CREATE");
    ThrowIfENetError(enet_address_set_host(&address, hostName.c_str()), "ENET_ADDRESS_SET_HOST");
    address.port = port;
    token = token_;

    Connect();
};

UDPClient::~UDPClient()
{
    enet_host_flush(client);
    enet_peer_disconnect(peer, 0);
    enet_peer_reset(peer);
    enet_host_destroy(client);
    delete[] rcvBuffer;
    delete[] sndBuffer;
};

bool UDPClient::Update(uint32 timeout)
{
    ENetEvent event;
    const int rc = enet_host_service(client, &event, timeout);
    if (0 >= rc)
    {
        if (0 > rc)
        {
            errorSignal.Emit(NetworkErrors::LOOP_ERROR);
        }

        return false;
    }

    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
    {
        Logger::FrameworkDebug("SERVER_CONNECTED: host:%d port:%d", address.host, address.port);
        connAttempts = maxConnAttempts;
        connectSignal.Emit();
        wasConnected = true;

        if (!token.empty())
        {
            PacketParams packetParams = PacketParams::Reliable(PacketParams::TOKEN_CHANNEL_ID);
            TokenPacketHeader header;
            uint32 tokenLength = static_cast<uint32>(strlen(token.c_str()));
            DVASSERT(tokenLength == TokenPacketHeader::TOKEN_LENGTH);
            memcpy(header.token, token.c_str(), tokenLength);
            Send((uint8*)&header, sizeof(TokenPacketHeader), packetParams);
        }
        break;
    }

    case ENET_EVENT_TYPE_DISCONNECT:
    {
        Logger::FrameworkDebug("SERVER_DISCONNECTED: host:%d port:%d", address.host, address.port);
        if (wasConnected)
        {
            disconnectSignal.Emit();
            wasConnected = false;
        }

        if (connAttempts == 0)
        {
            errorSignal.Emit(NetworkErrors::NOT_CONNECTED);
            return false;
        }

        enet_peer_reset(peer);
        Connect();

        if (connAttempts > 0)
        {
            --connAttempts;
        }
        break;
    }

    case ENET_EVENT_TYPE_RECEIVE:
    {
        auto subscrsIt = receiveSubscrs.find(event.channelID);
        if (subscrsIt != receiveSubscrs.end())
        {
            size_t csize = GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE);
            if (DecompressWithLz4(event.packet->data, event.packet->dataLength, rcvBuffer, csize))
            {
                for (const auto& cb : subscrsIt->second)
                {
                    cb(rcvBuffer, csize, event.channelID, event.peer->roundTripTime);
                }
            }
            else
            {
                Logger::Error("Decompression was failed (%d). Data size: %d", csize, event.packet->dataLength);
            }
        }
        trafficLogger.LogReceiving(event.packet->dataLength, event.channelID);
        enet_packet_destroy(event.packet);
        break;
    }

    default:
        break;
    }

    return true;
};

bool UDPClient::IsConnected() const
{
    return peer->state == ENET_PEER_STATE_CONNECTED;
}

bool UDPClient::Send(const uint8* data, size_t size, const PacketParams& param) const
{
    if (!IsConnected())
    {
        return false;
    }

    if (size > PacketParams::MAX_PACKET_SIZE)
    {
        DAVA_THROW(DAVA::Exception, "Packet size > MAX_PACKET_SIZE");
    }

    size_t csize = GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE);
    CompressWithLz4(data, size, sndBuffer, csize);

    if (csize > ENET_HOST_DEFAULT_MTU && !param.isReliable)
    {
        DAVA_THROW(DAVA::Exception, "Packet size > ENET_HOST_DEFAULT_MTU");
    }

    ENetPacket* packet = enet_packet_create(sndBuffer, csize, param.BuildFlags());
    ThrowIfENetError(packet, "ENET_PACKET_CREATE");
    ThrowIfENetError(enet_peer_send(peer, param.channelID, packet), "ENET_PEER_SEND");
    trafficLogger.LogSending(csize, param.channelID);

    return true;
};

void UDPClient::SubscribeOnConnect(const OnClientConnectCb& callback)
{
    connectSignal.Connect(callback);
}

void UDPClient::SubscribeOnDisconnect(const OnClientDisconnectCb& callback)
{
    disconnectSignal.Connect(callback);
}

void UDPClient::SubscribeOnError(const OnClientErrorCb& callback)
{
    errorSignal.Connect(callback);
}

void UDPClient::SubscribeOnReceive(uint8 channel, const OnClientReceiveCb& callback)
{
    auto findIt = receiveSubscrs.find(channel);
    if (findIt == receiveSubscrs.end())
    {
        findIt = receiveSubscrs.emplace(channel, Vector<OnClientReceiveCb>()).first;
    }
    findIt->second.push_back(callback);
}

void UDPClient::Connect()
{
    peer = enet_host_connect(client, &address, channelCount, 0);
    ThrowIfENetError(peer, "ENET_HOST_CONNECT");
#ifdef __DAVAENGINE_DEBUG__
    enet_peer_timeout(peer, 0, 1000, 1500);
#endif
};

const FastName& UDPClient::GetAuthToken() const
{
    return token;
}

uint32 UDPClient::GetPing() const
{
    uint32 result = std::numeric_limits<uint32>::max();
    if (nullptr != peer)
    {
        result = peer->lastRoundTripTime;
    }
    return result;
}

float32 UDPClient::GetPacketLoss() const
{
    return DAVA::GetPacketLoss(peer, packetLosses);
}

} // namespace DAVA
