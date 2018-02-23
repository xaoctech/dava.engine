#pragma once

#include "NetworkCore/UDPTransport/Private/ENetGuard.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "NetworkCore/UDPTransport/Private/TrafficLogger.h"
#include "NetworkCore/NetworkTypes.h"

#include <Base/FastName.h>
#include <Functional/Function.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>

#include <enet/enet.h>

namespace DAVA
{
using OnClientReceiveCb = Function<void(const uint8*, size_t, uint8, uint32)>;
using OnClientErrorCb = Function<void(NetworkErrors)>;
using OnClientConnectCb = Function<void()>;
using OnClientDisconnectCb = Function<void()>;

class IClient
{
public:
    virtual bool Update(uint32 timeout = 0) = 0;
    virtual bool IsConnected() const = 0;
    virtual bool Send(const uint8* data, size_t size, const PacketParams& param) const = 0;

    virtual void SubscribeOnConnect(const OnClientConnectCb& callback) = 0;
    virtual void SubscribeOnDisconnect(const OnClientDisconnectCb& callback) = 0;
    virtual void SubscribeOnError(const OnClientErrorCb& callback) = 0;
    virtual void SubscribeOnReceive(uint8 channel, const OnClientReceiveCb& callback) = 0;

    virtual const FastName& GetAuthToken() const = 0;
    /* on error return std::numeric_limits<uint32>::max() */
    virtual uint32 GetPing() const = 0;
    virtual float32 GetPacketLoss() const = 0;

    virtual ~IClient(){};
};

class UDPClient : public IClient
{
    DAVA_REFLECTION(UDPClient);

public:
    UDPClient(const DAVA::String& hostName, uint16 port, const FastName& token, size_t peerCount, int8 connAttempts = -1);
    ~UDPClient() override;

    bool Update(uint32 timeout = 0) override;
    bool IsConnected() const override;
    bool Send(const uint8* data, size_t size, const PacketParams& param) const override;

    void SubscribeOnConnect(const OnClientConnectCb& callback) override;
    void SubscribeOnDisconnect(const OnClientDisconnectCb& callback) override;
    void SubscribeOnError(const OnClientErrorCb& callback) override;
    void SubscribeOnReceive(uint8 channel, const OnClientReceiveCb& callback) override;

    const FastName& GetAuthToken() const override;
    /* on error return std::numeric_limits<uint32>::max() */
    uint32 GetPing() const override;
    float32 GetPacketLoss() const override;

private:
    void Connect();

    ENetGuard guard;
    ENetAddress address;
    ENetHost* client;
    ENetPeer* peer;
    int8 maxConnAttempts;
    int8 connAttempts;
    bool wasConnected;
    uint8 channelCount;
    FastName token;

    Signal<> connectSignal;
    Signal<> disconnectSignal;

    Signal<NetworkErrors> errorSignal;
    UnorderedMap<uint8, Vector<OnClientReceiveCb>> receiveSubscrs;

    uint8* rcvBuffer;
    uint8* sndBuffer;
    mutable TrafficLogger trafficLogger;
    mutable float32 packetLosses = 0.f;
};
} // namespace DAVA
