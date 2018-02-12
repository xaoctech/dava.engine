#pragma once

#include <memory>

#include "Private/ENetGuard.h"
#include "Private/ENetUtils.h"
#include "Private/TrafficLogger.h"
#include "enet/enet.h"
#include "Functional/Function.h"
#include "Functional/Signal.h"
#include "Base/FastName.h"
#include "Base/Deque.h"

namespace DAVA
{
class Responder
{
public:
    virtual void Send(const uint8* data, size_t size, const PacketParams& param) const = 0;
    virtual uint32 GetRtt() const = 0;
    virtual float32 GetPacketLoss() const = 0;
    virtual const FastName& GetToken() const = 0;
    virtual void SetToken(const FastName& token_) = 0;
    virtual bool IsValid() const = 0;
    virtual void SetIsValid(bool value) = 0;
    virtual const uint8 GetTeamID() const = 0;
    virtual void SetTeamID(uint8 teamID_) = 0;
    virtual ENetPeer* GetPeer() const = 0;
    virtual void SaveRtt() = 0;
    virtual bool RttIsBetter() const = 0;
    virtual ~Responder()
    {
    }
};

class UDPResponder : public Responder
{
public:
    UDPResponder(ENetPeer* peer, TrafficLogger* trafficLogger);
    UDPResponder(const UDPResponder& responder);
    ~UDPResponder() override;

    void Send(const uint8* data, size_t size, const PacketParams& param) const override;
    uint32 GetRtt() const override;
    float32 GetPacketLoss() const override;
    const FastName& GetToken() const override;
    void SetToken(const FastName& token_) override;
    bool IsValid() const override;
    void SetIsValid(bool value) override;
    const uint8 GetTeamID() const override;
    void SetTeamID(uint8 teamID_) override;
    ENetPeer* GetPeer() const override;
    void SaveRtt() override;
    bool RttIsBetter() const override;

private:
    ENetPeer* peer;
    TrafficLogger* trafficLogger;
    FastName token;
    uint8 teamID = 0;
    mutable float32 packetLosses = 0.f;
    bool isValid = false;
    uint32 prevRtt = 0;
    uint8* buffer = nullptr;
};

using OnServerReceiveCb = Function<void(const Responder&, const uint8*, size_t)>;
using OnServerErrorCb = Function<void(NetworkErrors)>;
using OnServerConnectCb = Function<void(const Responder&)>;
using OnServerTokenConfirmationCb = OnServerConnectCb;
using OnServerDisconnectCb = Function<void(const FastName&)>;
using DoForEach = Function<void(const Responder&)>;

class IServer
{
public:
    virtual bool Update(uint32 timeout = 0) = 0;
    virtual void Broadcast(const uint8* data, size_t size, const PacketParams& param) const = 0;
    virtual void Foreach(const DoForEach& callback) const = 0;
    virtual uint32 GetMaxRtt() const = 0;
    virtual const Responder& GetResponder(const FastName& token) const = 0;
    virtual bool HasResponder(const FastName& token) const = 0;
    virtual void SetValidToken(const FastName& token) = 0;
    virtual void Disconnect(const FastName& token) = 0;

    virtual void SubscribeOnConnect(const OnServerConnectCb& callback) = 0;
    virtual void SubscribeOnError(const OnServerErrorCb& callback) = 0;
    virtual void SubscribeOnReceive(uint8 channel, const OnServerReceiveCb& callback) = 0;
    virtual void SubscribeOnTokenConfirmation(const OnServerTokenConfirmationCb& callback) = 0;
    virtual void SubscribeOnDisconnect(const OnServerDisconnectCb& callback) = 0;
};

class UDPServer : public IServer
{
public:
    UDPServer(uint32 host, uint16 port, size_t peerCount);
    ~UDPServer();

    bool Update(uint32 timeout = 0) override;
    void Broadcast(const uint8* data, size_t size, const PacketParams& param) const override;
    void Foreach(const DoForEach& callback) const override;
    uint32 GetMaxRtt() const override;
    const Responder& GetResponder(const FastName& token) const override;
    bool HasResponder(const FastName& token) const override;
    void SetValidToken(const FastName& token) override;
    void Disconnect(const FastName& token) override;

    void SubscribeOnConnect(const OnServerConnectCb& callback) override;
    void SubscribeOnError(const OnServerErrorCb& callback) override;
    void SubscribeOnReceive(uint8 channel, const OnServerReceiveCb& callback) override;
    void SubscribeOnTokenConfirmation(const OnServerTokenConfirmationCb& callback) override;
    void SubscribeOnDisconnect(const OnServerDisconnectCb& callback) override;

private:
    ENetGuard guard;
    ENetAddress address;
    ENetHost* server;
    UnorderedMap<ENetPeer*, UDPResponder> peerStorage;
    UnorderedMap<FastName, ENetPeer*> tokenIndex;

    Signal<const Responder&> connectSignal;
    Signal<const Responder&> tokenConfirmationSignal;

    Signal<NetworkErrors> errorSignal;
    UnorderedMap<uint8, Vector<OnServerReceiveCb>> receiveSubscrs;
    uint32 maxRtt;

    Signal<const FastName&> disconnectSignal;
    std::unique_ptr<TrafficLogger> trafficLogger;

    void OnReceiveToken(const Responder& responder, const uint8* data, size_t size);

    uint8* buffer;
};
} // namespace DAVA
