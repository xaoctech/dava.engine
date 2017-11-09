#ifndef __DAVAENGINE_ASSET_CACHE_SERVER_H__
#define __DAVAENGINE_ASSET_CACHE_SERVER_H__


#include "DavaTools/AssetCache/Connection.h"
#include "DavaTools/AssetCache/CacheItemKey.h"

#include <Base/BaseTypes.h>
#include <Network/IChannel.h>

namespace DAVA
{
namespace AssetCache
{
class CachedItemValue;

class ServerNetProxyListener
{
public:
    virtual ~ServerNetProxyListener() = default;

    virtual void OnAddChunkToCache(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData) = 0;
    virtual void OnChunkRequestedFromCache(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key, uint32 chunkNumber) = 0;
    virtual void OnRemoveFromCache(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key) = 0;
    virtual void OnClearCache(std::shared_ptr<Net::IChannel> channel) = 0;
    virtual void OnWarmingUp(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key) = 0;
    virtual void OnStatusRequested(std::shared_ptr<Net::IChannel> channel) = 0;

    virtual void OnChannelClosed(std::shared_ptr<Net::IChannel> channel, const char8* message){};
};

class ServerNetProxy final : public Net::IChannelListener
{
public:
    ServerNetProxy(Dispatcher<Function<void()>>*);
    ~ServerNetProxy();

    void SetListener(ServerNetProxyListener* delegate);

    void Listen(uint16 port);

    void Disconnect();

    uint16 GetListenPort() const;

    bool SendAddedToCache(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key, bool added);
    bool SendRemovedFromCache(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key, bool removed);
    bool SendCleared(std::shared_ptr<Net::IChannel> channel, bool cleared);
    bool SendChunk(std::shared_ptr<Net::IChannel> channel, const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData);
    bool SendStatus(std::shared_ptr<Net::IChannel> channel);

    //Net::IChannelListener
    // Channel is open (underlying transport has connection) and can receive and send data through IChannel interface
    void OnChannelOpen(std::shared_ptr<Net::IChannel> channel) override{};
    // Channel is closed (underlying transport has disconnected) with reason
    void OnChannelClosed(std::shared_ptr<Net::IChannel> channel, const char8* message) override;
    // Some data arrived into channel
    void OnPacketReceived(std::shared_ptr<Net::IChannel> channel, const void* buffer, size_t length) override;
    // Buffer has been sent and can be reused or freed
    void OnPacketSent(std::shared_ptr<Net::IChannel> channel, const void* buffer, size_t length) override;
    // Data packet with given ID has been delivered to other side
    void OnPacketDelivered(std::shared_ptr<Net::IChannel> channel, uint32 packetId) override{};

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;
    uint16 listenPort = 0;
    std::shared_ptr<Connection> netServer;
    ServerNetProxyListener* listener = nullptr;
};

inline uint16 ServerNetProxy::GetListenPort() const
{
    return listenPort;
}

inline void ServerNetProxy::SetListener(ServerNetProxyListener* listener_)
{
    listener = listener_;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_SERVER_H__
