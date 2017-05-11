#pragma once

#include "CacheDB.h"

#include <Tools/AssetCache/AssetCache.h>

#include <FileSystem/DynamicMemoryFile.h>

class ServerLogics : public DAVA::AssetCache::ServerNetProxyListener,
                     public DAVA::AssetCache::ClientNetProxyListener
{
public:
    void Init(DAVA::AssetCache::ServerNetProxy* server, const DAVA::String& serverName, DAVA::AssetCache::ClientNetProxy* client, CacheDB* dataBase);
    void Update();

    void OnRemoteDisconnecting();

    //ServerNetProxyListener
    void OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint64 dataSize, DAVA::uint32 numOfChunks) override;
    void OnAddChunkToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunkData) override;
    void OnRequestedFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnChunkRequestedFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber) override;
    void OnRemoveFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnClearCache(DAVA::Net::IChannel* channel) override;
    void OnWarmingUp(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnChannelClosed(DAVA::Net::IChannel* channel, const DAVA::char8* message) override;
    void OnStatusRequested(DAVA::Net::IChannel* channel) override;

    //ClientNetProxyListener
    void OnClientProxyStateChanged() override;
    void OnAddedToCache(const DAVA::AssetCache::CacheItemKey& key, bool added);
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, DAVA::uint64 dataSize, DAVA::uint32 numOfChunks) override;
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunkData) override;

private:
    struct GetDataTask
    {
        enum DataRequestStatus
        {
            READY,
            WAITING_DATA_INFO,
            WAITING_NEXT_CHUNK
        };

        struct ClientStatus
        {
            ClientStatus(DataRequestStatus status = DataRequestStatus::READY)
                : status(status)
            {
            }
            DataRequestStatus status = DataRequestStatus::READY;
            DAVA::uint32 waitingChunk = 0;
        };

        DAVA::UnorderedMap<DAVA::Net::IChannel*, ClientStatus> clients;
        DAVA::ScopedPtr<DAVA::DynamicMemoryFile> serializedData;
        DataRequestStatus dataStatus = READY;

        DAVA::uint64 bytesReady = 0;
        DAVA::uint64 bytesOverall = 0;
        DAVA::uint32 chunksReady = 0;
        DAVA::uint32 chunksOverall = 0;
    };
    using GetTasksMap = DAVA::UnorderedMap<DAVA::AssetCache::CacheItemKey, GetDataTask>;

    struct AddDataTask
    {
        DAVA::AssetCache::CacheItemKey key;
        DAVA::Net::IChannel* channel;
        DAVA::ScopedPtr<DAVA::DynamicMemoryFile> receivedData;

        size_t bytesReceived = 0;
        size_t bytesOverall = 0;
        DAVA::uint32 chunksReceived = 0;
        DAVA::uint32 chunksOverall = 0;
    };

    struct RemoteAddDataTask
    {
        DAVA::ScopedPtr<DAVA::DynamicMemoryFile> serializedData;
        DAVA::uint32 chunksSent = 0;
        DAVA::uint32 chunksOverall = 0;
    };
    using RemoteAddTasksMap = DAVA::UnorderedMap<DAVA::AssetCache::CacheItemKey, RemoteAddDataTask>;

    struct WarmupTask
    {
        WarmupTask(const DAVA::AssetCache::CacheItemKey& key)
            : key(key)
        {
        }
        DAVA::AssetCache::CacheItemKey key;
    };

private:
    bool IsRemoteServerConnected() const;

    DAVA::List<AddDataTask>::iterator GetOrCreateAddTask(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key);
    GetTasksMap::iterator GetOrCreateGetTask(const DAVA::AssetCache::CacheItemKey& key);
    void RequestNextChunk(GetTasksMap::iterator it);
    void SendInfoToClients(GetTasksMap::iterator taskIt);
    void SendChunkToClients(GetTasksMap::iterator taskIt, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunk);
    bool SendAddInfoToRemote(RemoteAddTasksMap::iterator taskIt);
    bool SendAddChunkToRemote(RemoteAddTasksMap::iterator taskIt);
    void CancelGetTask(GetTasksMap::iterator it);
    void CancelRemoteTasks();
    void RemoveClientFromTasks(DAVA::Net::IChannel* clientChannel);

    void ProcessLazyTasks();

private:
    DAVA::AssetCache::ServerNetProxy* serverProxy = nullptr;
    DAVA::AssetCache::ClientNetProxy* clientProxy = nullptr;
    CacheDB* dataBase = nullptr;

    GetTasksMap getDataTasks;
    DAVA::List<AddDataTask> addDataTasks;
    DAVA::List<WarmupTask> warmupTasks;
    RemoteAddTasksMap remoteAddDataTasks;
    DAVA::String serverName;
};
