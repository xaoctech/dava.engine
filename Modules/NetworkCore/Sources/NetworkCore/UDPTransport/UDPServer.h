#pragma once

#include <memory>

#include "Private/ENetGuard.h"
#include "Private/ENetUtils.h"
#include "Private/TrafficLogger.h"
#include "enet/enet.h" // <- should be hidden in *.cpp as implementation details in future
#include "Functional/Signal.h"
#include "Base/FastName.h"

// Future vision: of integration Replay System with other parts of programm
// I want you to add more info about global sheme of networking in DAVA.
//                        +
//                        |            +--------------+
//       Client           |            |    Server    |
//                        |            +--------------+
//+----------------------------------------------------------------+
//                        |            +--------------+
//                        |            |    Scene     |                 +-----------------------------+
//                        |            +--^--------+--+                 |In: Timings, Users Input     |
//                        |               |        |                    |Out: Scene Replication Diffs |
//                        |            In |     Out|                    |                             |
//                        |      +--------+--------v--------+           +-----------------------------+
//                        |      | Dava Scene Sync protocol |
//                        |      +--------^--------+--------+           +-----------------------------+
//                        |               |        |                    |In: frame_id?, packets?      |
//                        |            In |     Out|                    |Out: Replication diffs? Full?|
//                        |       +-------+--------v----------+         +-----------------------------+
//                        |       | Dava Auth, Time Sync prot |
//                        |       +-------^--------+----------+         +-----------------------------+
//                        |               |        |                    |In: EConn,EDis,ERecieve(all) |
//                        |            In |     Out|                    |Out: Full Diff, Replic Diff  |
//                        |           +---+--------v----+               +-----------------------------+
//                        |           |  ENet  v1.3.13  |
//                        |           +-----------------+
//                        |
//                        |
//                        |
//                        +
//
//
//
//                             +-----------------v-------------+
//                             | Dava Scene Sync protocol v0.1 |
//                             +----+---^------+---------+-----+
//                                  |          |         |
//                                  |          |         |
//                     +------------v---+      |   +-----v----------+
//                     |  Process       |      |   | Record, Replay,|
//                     |  FixedProcess  |      |   | System         |
//                     +----------------+      |   +----------------+
//                                             |
//                                             |
//                                  +----------v--------+
//                                  |  Replication Diffs|
//                                  |  System           |
//                                  |                   |
//                                  +-------------------+
//

namespace DAVA
{
class Responder
{
public:
    using AckCallback = Function<void()>;
    virtual void Send(const uint8* data, size_t size, const PacketParams& param, const AckCallback& callback = {}) const = 0;
    virtual uint32 GetRtt() const = 0;
    virtual float32 GetPacketLoss() const = 0;
    virtual const FastName& GetToken() const = 0;
    virtual void SetToken(const FastName& token_) = 0;
    virtual bool IsValid() const = 0;
    virtual void SetIsValid(bool value) = 0;
    virtual const uint8 GetTeamID() const = 0;
    virtual void SetTeamID(uint8 teamID_) = 0;
    virtual void SaveRtt() = 0;
    virtual bool RttIsBetter() const = 0;
    virtual ~Responder() = default;
};

class UDPResponder : public Responder
{
public:
    UDPResponder(ENetPeer* peer, TrafficLogger* trafficLogger);
    UDPResponder(const UDPResponder& responder) = delete;
    UDPResponder(UDPResponder&&) = delete;
    UDPResponder& operator=(const UDPResponder& other) = delete;
    UDPResponder& operator=(UDPResponder&& other) = delete;
    ~UDPResponder() override;

    void Send(const uint8* data, size_t size, const PacketParams& param, const AckCallback& callback) const override;
    uint32 GetRtt() const override;
    float32 GetPacketLoss() const override;
    const FastName& GetToken() const override;
    void SetToken(const FastName& token_) override;
    bool IsValid() const override;
    void SetIsValid(bool value) override;
    const uint8 GetTeamID() const override;
    void SetTeamID(uint8 teamID_) override;
    ENetPeer* GetPeer() const;
    void SaveRtt() override;
    bool RttIsBetter() const override;

private:
    static void OnFreeCallback(ENetPacket* packet);
    mutable UnorderedMap<ENetPacket*, AckCallback> packetToAckCallback;

    ENetPeer* peer;
    TrafficLogger* trafficLogger;
    FastName token;
    uint8 teamID = 0;
    mutable float32 packetLosses = 0.f;
    bool isValid = false;
    uint32 prevRtt = 0;
    uint8* buffer = nullptr;
};

// FIXME (draft) in future this interface will be removed (after split Enet/DavaLow/DavaHigh protocol)
// If your system need respond as fast as possible (like NetworkTimeSystem)
// This can be extructed in low-level network interface in future
struct IServerSyncCallback
{
    virtual ~IServerSyncCallback() = default;

    virtual void OnConnectServer(const Responder& responder) = 0;
    virtual void OnReceiveServer(const Responder& responder, const void* data, size_t) = 0;
};

// FIXME (draft) this interface should store all data from network after refactor (this is DavaHi - protocol)
// This can be future hi-level network interface
struct INetworkEventStorage
{
    virtual ~INetworkEventStorage() = default;

    virtual void AddConnectedToken(const FastName& token) = 0;
    virtual void StoreRecvPacket(const PacketParams::Channels channel, const FastName& token, const void* dataPtr, size_t dataSize) = 0;
    virtual void RemoveConnectedToken(const FastName& token) = 0;
    virtual void ConfirmToken(const FastName& token) = 0;
};

class IServer
{
public:
    virtual ~IServer() = default;

    virtual bool Update(uint32 timeout = 0) = 0;
    virtual void Broadcast(const uint8* data, size_t size, const PacketParams& param) const = 0;
    virtual uint32 GetMaxRtt() const = 0;
    virtual const Responder& GetResponder(const FastName& token) const = 0;
    virtual bool HasResponder(const FastName& token) const = 0;
    virtual void SetValidToken(const FastName& token) = 0;
    virtual void Disconnect(const FastName& token) = 0;
    virtual void EmitFakeReconnect(const Responder& responder) = 0;
    // FIXME in future remove it
    virtual void SetNetworkEventStorage(INetworkEventStorage& netEventStore) = 0;
    // FIXME in future remove it
    virtual void SetServerSyncCallback(IServerSyncCallback& syncCallback) = 0;
};

class UDPServer : public IServer
{
public:
    UDPServer(uint32 host, uint16 port, size_t peerCount);
    ~UDPServer();

    bool Update(uint32 timeout = 0) override;
    void Broadcast(const uint8* data, size_t size, const PacketParams& param) const override;
    uint32 GetMaxRtt() const override;
    const Responder& GetResponder(const FastName& token) const override;
    bool HasResponder(const FastName& token) const override;
    void SetValidToken(const FastName& token) override;
    void Disconnect(const FastName& token) override;
    void EmitFakeReconnect(const Responder& responder) override;

    void SetNetworkEventStorage(INetworkEventStorage& netEventStore_) final;
    void SetServerSyncCallback(IServerSyncCallback& syncCallback_) final;

private:
    ENetGuard guard;
    ENetAddress address;
    ENetHost* server;
    UnorderedMap<ENetPeer*, UDPResponder> peerStorage;
    UnorderedMap<FastName, ENetPeer*> tokenIndex;
    UnorderedMap<FastName, ENetPeer*> pendingTokens;

    Signal<const Responder&> connectSignal;
    Signal<const Responder&> tokenConfirmationSignal;

    Signal<NetworkErrors> errorSignal;
    uint32 maxRtt;

    Signal<const FastName&> disconnectSignal;
    std::unique_ptr<TrafficLogger> trafficLogger;

    void OnReceiveToken(const Responder& responder, const uint8* data, size_t size);
    void AddTokenToIndex(const FastName& token, ENetPeer* peer);

    uint8* buffer;

    INetworkEventStorage* networkEventStorage = nullptr;
    IServerSyncCallback* syncCallback = nullptr;
};
} // namespace DAVA
