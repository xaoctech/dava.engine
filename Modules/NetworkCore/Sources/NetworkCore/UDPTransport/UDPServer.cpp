#include "Logger/Logger.h"
#include "Time/SystemTimer.h"
#include "Private/ENetUtils.h"
#include "UDPServer.h"
#include "NetworkCore/NetworkTypes.h"

#include <Debug/ProfilerCPU.h>
#include "Concurrency/Thread.h"

namespace DAVA
{
UDPResponder::UDPResponder(ENetPeer* peer, TrafficLogger* trafficLogger)
    : peer(peer)
    , trafficLogger(trafficLogger)
    , buffer(new uint8[GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE)])
{
#ifdef __DAVAENGINE_DEBUG__
    enet_peer_timeout(peer, 0, 1000, 1500);
#endif
}

UDPResponder::UDPResponder(const UDPResponder& responder)
    : peer(responder.peer)
    , trafficLogger(responder.trafficLogger)
    , token(responder.token)
    , teamID(responder.teamID)
    , packetLosses(responder.packetLosses)
    , isValid(responder.isValid)
    , prevRtt(responder.prevRtt)
    , buffer(new uint8[GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE)])
{
}

void UDPResponder::Send(const uint8* data, size_t size, const PacketParams& param) const
{
    if (size > PacketParams::MAX_PACKET_SIZE)
    {
        DAVA_THROW(DAVA::Exception, "Packet size > MAX_PACKET_SIZE");
    }

    size_t csize = GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE);
    CompressWithLz4(data, size, buffer, csize);

    if (csize > ENET_HOST_DEFAULT_MTU && !param.isReliable)
    {
        DAVA_THROW(DAVA::Exception, "Packet size > ENET_HOST_DEFAULT_MTU");
    }

    ENetPacket* packet = enet_packet_create(buffer, csize, param.BuildFlags());
    ThrowIfENetError(enet_peer_send(peer, param.channelID, packet), "ENET_PEER_SEND");
    trafficLogger->LogSending(csize, param.channelID);
}

uint32 UDPResponder::GetRtt() const
{
    return peer->roundTripTime;
}

const FastName& UDPResponder::GetToken() const
{
    return token;
}

void UDPResponder::SetToken(const FastName& token_)
{
    token = token_;
}

bool UDPResponder::IsValid() const
{
    return isValid;
}

void UDPResponder::SetIsValid(bool value)
{
    isValid = value;
}

const uint8 UDPResponder::GetTeamID() const
{
    return teamID;
}

void UDPResponder::SetTeamID(uint8 teamID_)
{
    teamID = teamID_;
}

ENetPeer* UDPResponder::GetPeer() const
{
    return peer;
}

void UDPResponder::SaveRtt()
{
    prevRtt = GetRtt();
}

bool UDPResponder::RttIsBetter() const
{
    return prevRtt > GetRtt();
}

float32 UDPResponder::GetPacketLoss() const
{
    return DAVA::GetPacketLoss(peer, packetLosses);
}

UDPResponder::~UDPResponder()
{
    delete[] buffer;
}

UDPServer::UDPServer(uint32 host, uint16 port, size_t peerCount)
    : maxRtt(0)
    , trafficLogger(new TrafficLogger())
    , buffer(new uint8[GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE)])
{
    address.host = host;
    address.port = port;

    server = enet_host_create(&address, peerCount, 0, 0, 0);
    ThrowIfENetError(server, "SERVER_ENET_HOST_CREATE");

    /*
     SERVER_COMPLETE
     Think about ability about unsubscribing from signals from UDPServer, UDPClient.
     Here I thought to unsubscribe after token recv, but it's not possible with current API
     */
    SubscribeOnReceive(PacketParams::TOKEN_CHANNEL_ID,
                       OnServerReceiveCb(this, &UDPServer::OnReceiveToken));
}

UDPServer::~UDPServer()
{
    enet_host_destroy(server);
    delete[] buffer;
}

bool UDPServer::Update(uint32 timeout)
{
    ENetEvent event;
    const int rc = enet_host_service(server, &event, timeout);
    if (0 >= rc)
    {
        if (0 > rc)
        {
            errorSignal.Emit(NetworkErrors::LOOP_ERROR);
        }
        return false;
    }

    ENetPeer* peer = event.peer;
    ThrowIfENetError(peer, "PEER_NOT_FOUND");
    if (event.peer->roundTripTime > maxRtt)
    {
        maxRtt = event.peer->roundTripTime;
    }

    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
    {
        Logger::FrameworkDebug("CLIENT_CONNECTED: host:%d port:%d", peer->address.host, peer->address.port);
        bool ret = peerStorage.emplace(peer, UDPResponder(peer, trafficLogger.get())).second;
        DVASSERT(ret);
        break;
    }

    case ENET_EVENT_TYPE_DISCONNECT:
    {
        Logger::FrameworkDebug("CLIENT_DISCONNECTED: host:%d port:%d", peer->address.host, peer->address.port);
        auto peerIt = peerStorage.find(peer);
        DVASSERT(peerIt != peerStorage.end());
        disconnectSignal.Emit(peerIt->second.GetToken());
        tokenIndex.erase(peerIt->second.GetToken());
        peerStorage.erase(peerIt);
        break;
    }

    case ENET_EVENT_TYPE_RECEIVE:
    {
        auto peerIt = peerStorage.find(peer);
        DVASSERT(peerIt != peerStorage.end());
        Responder& responder = peerIt->second;
        if (!responder.IsValid() && event.channelID != PacketParams::TOKEN_CHANNEL_ID)
        {
            enet_packet_destroy(event.packet);
            break;
        }
        auto subscrsIt = receiveSubscrs.find(event.channelID);
        if (subscrsIt != receiveSubscrs.end())
        {
            size_t csize = GetMaxCompressedSizeWithLz4(PacketParams::MAX_PACKET_SIZE);
            if (DecompressWithLz4(event.packet->data, event.packet->dataLength, buffer, csize))
            {
                for (const auto& cb : subscrsIt->second)
                {
                    cb(responder, buffer, csize);
                }
            }
            else
            {
                Logger::Error("Decompression was failed (%d). Data size: %d", csize, event.packet->dataLength);
            }
        }
        responder.SaveRtt();
        trafficLogger->LogReceiving(event.packet->dataLength, event.channelID);
        enet_packet_destroy(event.packet);
        break;
    }

    default:
    {
        break;
    }
    }

    return true;
}

void UDPServer::Foreach(const DoForEach& callback) const
{
    for (const auto& peer : peerStorage)
    {
        const Responder& responder = peer.second;
        if (responder.IsValid())
        {
            callback(responder);
        }
    }
}

void UDPServer::Broadcast(const uint8* data, size_t size, const PacketParams& param) const
{
    for (const auto& peer : peerStorage)
    {
        const Responder& responder = peer.second;
        if (responder.IsValid())
        {
            responder.Send(data, size, param);
        }
    }
}

uint32 UDPServer::GetMaxRtt() const
{
    return maxRtt;
}

const Responder& UDPServer::GetResponder(const FastName& token) const
{
    auto peerIt = tokenIndex.find(token);
    DVASSERT(peerIt != tokenIndex.end());
    auto responderIt = peerStorage.find(peerIt->second);
    DVASSERT(responderIt != peerStorage.end());
    return responderIt->second;
}

bool UDPServer::HasResponder(const FastName& token) const
{
    auto peerIt = tokenIndex.find(token);
    if (peerIt == tokenIndex.end())
    {
        return false;
    }
    auto responderIt = peerStorage.find(peerIt->second);
    return (responderIt != peerStorage.end());
}

void UDPServer::SubscribeOnConnect(const OnServerConnectCb& callback)
{
    connectSignal.Connect(callback);
}

void UDPServer::SubscribeOnTokenConfirmation(const OnServerTokenConfirmationCb& callback)
{
    tokenConfirmationSignal.Connect(callback);
}

void UDPServer::SubscribeOnError(const OnServerErrorCb& callback)
{
    errorSignal.Connect(callback);
}

void UDPServer::SubscribeOnReceive(uint8 channel, const OnServerReceiveCb& callback)
{
    auto findIt = receiveSubscrs.find(channel);
    if (findIt == receiveSubscrs.end())
    {
        findIt = receiveSubscrs.emplace(channel, Vector<OnServerReceiveCb>()).first;
    }
    findIt->second.push_back(callback);
}

void UDPServer::SubscribeOnDisconnect(const OnServerDisconnectCb& callback)
{
    disconnectSignal.Connect(callback);
}

void UDPServer::OnReceiveToken(const Responder& responder, const uint8* data, size_t size)
{
    DAVA_PROFILER_CPU_SCOPE("UDPServer::OnReceiveToken");
    const TokenPacketHeader* header = reinterpret_cast<const TokenPacketHeader*>(data);
    String tokenString(header->token, TokenPacketHeader::TOKEN_LENGTH);
    FastName token(tokenString);

    auto findIt = peerStorage.find(responder.GetPeer());
    DVASSERT(findIt != peerStorage.end());
    findIt->second.SetToken(token);
    auto emplaceRet = tokenIndex.emplace(token, responder.GetPeer());
    if (!emplaceRet.second)
    {
        emplaceRet.first->second = responder.GetPeer();
    }
    tokenConfirmationSignal.Emit(responder);
}

void UDPServer::SetValidToken(const FastName& token)
{
    auto peerIt = tokenIndex.find(token);
    DVASSERT(peerIt != tokenIndex.end());
    auto responderIt = peerStorage.find(peerIt->second);
    DVASSERT(responderIt != peerStorage.end());
    Responder& responder = responderIt->second;
    responder.SetIsValid(true);
    connectSignal.Emit(responder);
}

void UDPServer::Disconnect(const FastName& token)
{
    auto peerIt = tokenIndex.find(token);
    DVASSERT(peerIt != tokenIndex.end());
    ENetPeer* peer = peerIt->second;
    enet_peer_disconnect_now(peer, 0);
    peerStorage.erase(peer);
    tokenIndex.erase(peerIt);
    Logger::FrameworkDebug("CLIENT_DISCONNECTED: host:%d port:%d", peer->address.host, peer->address.port);
}

} // namespace DAVA
