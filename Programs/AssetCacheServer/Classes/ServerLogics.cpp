#include "ServerLogics.h"
#include "ServerCacheEntry.h"

#include <Concurrency/LockGuard.h>
#include <Logger/Logger.h>
#include <Utils/StringFormat.h>

namespace ServerLogicsDetails
{
using namespace DAVA;

const uint32 CHUNK_SIZE_IN_BYTES = 1 * 1024 * 1024;

uint32 GetNumberOfChunks(uint64 overallSize)
{
    uint32 res = overallSize / CHUNK_SIZE_IN_BYTES;
    if (overallSize % CHUNK_SIZE_IN_BYTES)
    {
        ++res;
    }
    return res;
}

Vector<uint8> GetChunk(const DynamicMemoryFile* data, uint32 chunkNumber)
{
    const Vector<uint8>& dataVector = data->GetDataVector();
    uint64 firstByte = chunkNumber * CHUNK_SIZE_IN_BYTES;
    if (firstByte < dataVector.size())
    {
        uint64 beyondLastByte = std::min(dataVector.size(), firstByte + CHUNK_SIZE_IN_BYTES);
        return Vector<uint8>(dataVector.begin() + firstByte, dataVector.begin() + beyondLastByte);
    }
    else
    {
        return Vector<uint8>();
    }
}
}

void ServerLogics::Init(DAVA::AssetCache::ServerNetProxy* server_, const DAVA::String& serverName_, DAVA::AssetCache::ClientNetProxy* client_, CacheDB* dataBase_)
{
    serverProxy = server_;
    serverName = serverName_;
    clientProxy = client_;
    dataBase = dataBase_;
}

void ServerLogics::OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::AssetCache::CachedItemValue&& value)
{
    using namespace DAVA;

    if ((nullptr != serverProxy) && (nullptr != channel))
    {
        AssetCache::CachedItemValue::Description description = value.GetDescription();
        description.addingChain += "/" + serverName;
        value.SetDescription(description);

        bool isValid = value.IsValid();
        if (isValid && value.GetSize() > dataBase->GetStorageSize())
        {
            isValid = false;
            Logger::Warning("[%s] Inserted size %u is bigger than max storage size %u", __FUNCTION__, value.GetSize(), dataBase->GetStorageSize());
        }

        serverProxy->SendAddedToCache(channel, key, isValid);

        if (isValid)
        {
            dataBase->Insert(key, value);

            //add task for lazy sending of files;
            //CreateAddTask(key, std::forward<AssetCache::CachedItemValue>(value), );
        }
        else
        {
            Logger::Error("[%s] Received invalid data from %s at %s", __FUNCTION__, description.machineName.c_str(), description.creationDate.c_str());
        }
    }
}

ServerLogics::GetTasksMap::iterator ServerLogics::GetOrCreateGetTask(const DAVA::AssetCache::CacheItemKey& key)
{
    using namespace DAVA;

    GetTasksMap::iterator taskIter = getDataTasks.find(key);
    if (taskIter == getDataTasks.end())
    {
        ServerCacheEntry* entry = dataBase->Get(key);
        if (nullptr != entry)
        { // Found in db.
            Logger::Debug("create get task using local data");
            taskIter = getDataTasks.emplace(key, GetDataTask()).first;
            GetDataTask& task = taskIter->second;
            task.serializedData = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);

            DAVA::AssetCache::CachedItemValue& value = entry->GetValue();
            DAVA::AssetCache::CachedItemValue::Description description = value.GetDescription();
            description.receivingChain += "/" + serverName;
            value.SetDescription(description);
            value.Serialize(task.serializedData);
            task.dataStatus = GetDataTask::READY;
            task.bytesOverall = task.bytesReady = task.serializedData->GetSize();
            task.chunksOverall = task.chunksReady = ServerLogicsDetails::GetNumberOfChunks(task.bytesOverall);
        }
        else if (IsRemoteServerConnected() && clientProxy->RequestData(key))
        { // Not found in db. Ask from remote cache.
            Logger::Debug("create get task. Requesting data from remote");
            taskIter = getDataTasks.emplace(key, GetDataTask()).first;
            GetDataTask& task = taskIter->second;
            task.serializedData = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);
            task.dataStatus = GetDataTask::WAITING_DATA_INFO;
        }
    }

    return taskIter;
}

void ServerLogics::OnRequestedFromCache(DAVA::Net::IChannel* clientChannel, const DAVA::AssetCache::CacheItemKey& key)
{
    DAVA::Logger::Debug("%s key %s", __FUNCTION__, key.ToString().c_str());
    GetTasksMap::iterator taskIter = GetOrCreateGetTask(key);
    if (taskIter != getDataTasks.end())
    {
        GetDataTask& task = taskIter->second;
        if (task.dataStatus == GetDataTask::WAITING_DATA_INFO)
        {
            task.clients.emplace(clientChannel, GetDataTask::WAITING_DATA_INFO);
        }
        else
        {
            serverProxy->SendDataInfo(clientChannel, key, task.bytesOverall, task.chunksOverall);
            task.clients.emplace(clientChannel, GetDataTask::READY);
            warmupTasks.emplace_back(WarmupTask(key));
        }
    }
    else
    { // Not found in db. Remote server isn't connected.
        serverProxy->SendDataInfo(clientChannel, key, 0, 0);
    }
}

void ServerLogics::OnChunkRequestedFromCache(DAVA::Net::IChannel* clientChannel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber)
{
    using namespace DAVA;
    Logger::Debug("%s key %s chunk %u", __FUNCTION__, key.ToString().c_str(), chunkNumber);

    auto Error = [&](const char* err)
    {
        Logger::Error("Wrong chunk request: %s. Client %p, key %s, chunk %u", err, clientChannel, key.ToString().c_str(), chunkNumber);
        Vector<uint8> empty;
        serverProxy->SendChunk(clientChannel, key, 0, empty);
    };

    GetTasksMap::iterator taskIter = GetOrCreateGetTask(key);
    if (taskIter != getDataTasks.end())
    {
        GetDataTask& task = taskIter->second;
        GetDataTask::ClientStatus& client = task.clients[clientChannel];

        if (task.chunksReady > chunkNumber) // task has such chunk
        {
            Vector<uint8> chunk = ServerLogicsDetails::GetChunk(task.serializedData, chunkNumber);
            if (chunk.empty())
            {
                Error("can't get valid range for given chunk");
                return;
            }
            serverProxy->SendChunk(clientChannel, key, chunkNumber, chunk);
            client.status = GetDataTask::READY;
        }
        else // task hasn't such chunk yet
        {
            if (task.dataStatus == GetDataTask::READY)
            {
                Error("task status is READY yet not all chunks are available");
                return;
            }
            else
            {
                client.status = GetDataTask::WAITING_NEXT_CHUNK;
                client.waitingChunk = chunkNumber;
            }
        }
    }
    else
    { // Not found in db. Remote server isn't connected.
        Vector<uint8> empty;
        serverProxy->SendChunk(clientChannel, key, 0, empty);
    }
}

void ServerLogics::OnRemoveFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key)
{
    if ((nullptr != serverProxy) && (nullptr != dataBase) && (nullptr != channel))
    {
        bool removed = dataBase->Remove(key);
        serverProxy->SendRemovedFromCache(channel, key, removed);
    }
}

void ServerLogics::OnClearCache(DAVA::Net::IChannel* channel)
{
    dataBase->ClearStorage();
    serverProxy->SendCleared(channel, true);
}

void ServerLogics::OnWarmingUp(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key)
{
    if (nullptr != dataBase)
    {
        dataBase->UpdateAccessTimestamp(key);
    }
}

void ServerLogics::OnStatusRequested(DAVA::Net::IChannel* channel)
{
    serverProxy->SendStatus(channel);
}

void ServerLogics::OnChannelClosed(DAVA::Net::IChannel* channel, const DAVA::char8*)
{
    RemoveClientFromTasks(channel);
}

void ServerLogics::OnRemoteDisconnecting()
{
    CancelRemoteTasks();
}

void ServerLogics::OnClientProxyStateChanged()
{
    DVASSERT(clientProxy);
    if (!clientProxy->ChannelIsOpened())
    {
        OnRemoteDisconnecting();
    }
}

void ServerLogics::OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, DAVA::uint64 dataSize, DAVA::uint32 numOfChunks)
{
    auto Error = [&](const char* err)
    {
        DAVA::Logger::Error("Wrong data info response: %s. Key %s", err, key.ToString().c_str());
    };

    DAVA::Logger::Debug("Received info: %u bytes, %u chunks", dataSize, numOfChunks);

    auto taskIter = getDataTasks.find(key);
    if (taskIter == getDataTasks.end())
    {
        Error("data was not requsted");
        return;
    }

    GetDataTask& task = taskIter->second;
    if (task.dataStatus != GetDataTask::DataRequestStatus::WAITING_DATA_INFO)
    {
        Error("data status is not WAITING_DATA_INFO");
        return;
    }
    DVASSERT(task.bytesReady == 0 && task.chunksReady == 0 && task.serializedData->GetSize() == 0);

    if (dataSize == 0 || numOfChunks == 0)
    {
        CancelGetTask(taskIter);
        return;
    }
    else
    {
        task.bytesOverall = dataSize;
        task.chunksOverall = numOfChunks;
        RequestNextChunk(taskIter);
    }
}

void ServerLogics::OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunkData)
{
    using namespace DAVA;

    auto Error = [&](const char* err, GetTasksMap::iterator taskIter)
    {
        Logger::Error("Wrong chunk response: %s. Key %s, chunk %u", err, key.ToString().c_str(), chunkNumber);
        CancelGetTask(taskIter);
    };

    auto taskIter = getDataTasks.find(key);
    if (taskIter == getDataTasks.end())
    {
        Error("data was not requsted", taskIter);
        return;
    }

    GetDataTask& task = taskIter->second;
    if (task.dataStatus != GetDataTask::DataRequestStatus::WAITING_NEXT_CHUNK)
    {
        Error("data status is not WAITING_NEXT_CHUNK", taskIter);
        return;
    }

    if (chunkData.empty())
    {
        Logger::Debug("Empty chunk is received. GetData task will be cancelled for all clients");
        CancelGetTask(taskIter);
        return;
    }

    if (chunkNumber != task.chunksReady)
    {
        Error(Format("chunk #%u was expected", task.chunksReady).c_str(), taskIter);
        return;
    }

    uint32 chunkSize = static_cast<uint32>(chunkData.size());
    uint32 written = task.serializedData->Write(chunkData.data(), chunkSize);
    if (written != chunkSize)
    {
        Error(Format("can't append %u bytes", chunkSize).c_str(), taskIter);
        return;
    }

    task.bytesReady += chunkSize;
    ++task.chunksReady;
    Logger::Debug("Chunk #%u received: %u bytes. Overall received %u, remaining %u", chunkNumber, chunkData.size(), task.bytesReady, task.bytesOverall - task.bytesReady);

    if (task.chunksReady == task.chunksOverall)
    {
        if (task.bytesReady != task.bytesOverall)
        {
            Error(Format("unexpected final bytes count: %u (expected %u bytes)", task.bytesReady, task.bytesOverall).c_str(), taskIter);
            return;
        }

        task.dataStatus = GetDataTask::READY;

        AssetCache::CachedItemValue value;
        value.Deserialize(task.serializedData);
        if (value.IsEmpty() || !value.IsValid())
        {
            Logger::Debug("Received data is empty or invalid");
            CancelGetTask(taskIter);
            return;
        }
        dataBase->Insert(key, value);
    }
    else
    {
        RequestNextChunk(taskIter);
    }

    SendChunkToClients(taskIter, chunkNumber, chunkData);
}

void ServerLogics::RequestNextChunk(ServerLogics::GetTasksMap::iterator it)
{
    DAVA::Logger::Debug("%s", __FUNCTION__);
    DVASSERT(it != getDataTasks.end());

    const DAVA::AssetCache::CacheItemKey& key = it->first;
    GetDataTask& task = it->second;
    DVASSERT(task.dataStatus != GetDataTask::READY);
    DVASSERT(task.chunksReady < task.chunksOverall);

    clientProxy->RequestGetNextChunk(key, task.chunksReady);
    task.dataStatus = GetDataTask::WAITING_NEXT_CHUNK;
}

void ServerLogics::SendChunkToClients(GetTasksMap::iterator& taskIt, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunk)
{
    DAVA::Logger::Debug("%s", __FUNCTION__);
    DVASSERT(taskIt != getDataTasks.end());

    const DAVA::AssetCache::CacheItemKey& key = taskIt->first;
    GetDataTask& task = taskIt->second;

    for (std::pair<DAVA::Net::IChannel* const, GetDataTask::ClientStatus>& client : task.clients)
    {
        if (client.second.status == GetDataTask::WAITING_NEXT_CHUNK && client.second.waitingChunk == chunkNumber)
        {
            serverProxy->SendChunk(client.first, key, chunkNumber, chunk);
            client.second.status = GetDataTask::READY;
        }
    }
}

void ServerLogics::CancelGetTask(ServerLogics::GetTasksMap::iterator it)
{
    using namespace DAVA;

    if (it != getDataTasks.end())
    {
        DAVA::Logger::Debug("%s", __FUNCTION__);
        const DAVA::AssetCache::CacheItemKey& key = it->first;
        GetDataTask& task = it->second;

        for (const std::pair<DAVA::Net::IChannel* const, GetDataTask::ClientStatus>& client : task.clients)
        {
            switch (client.second.status)
            {
            case GetDataTask::READY:
                break;
            case GetDataTask::WAITING_DATA_INFO:
                serverProxy->SendDataInfo(client.first, key, 0, 0);
                break;
            case GetDataTask::WAITING_NEXT_CHUNK:
                serverProxy->SendChunk(client.first, key, 0, Vector<uint8>());
                break;
            default:
                DVASSERT(false, Format("Incorrect data status: %u", task.dataStatus).c_str());
                break;
            }
        }

        getDataTasks.erase(it);
    }
}

void ServerLogics::RemoveClientFromTasks(DAVA::Net::IChannel* clientChannel)
{
    for (auto it = getDataTasks.begin(); it != getDataTasks.end();)
    {
        GetDataTask& task = it->second;
        task.clients.erase(clientChannel);
        if (task.clients.empty() && task.dataStatus == GetDataTask::READY)
        {
            auto itDel = it++;
            DAVA::Logger::Debug("removing get task, no one more needs it");
            getDataTasks.erase(itDel);
        }
        else
        {
            ++it;
        }
    }
}

void ServerLogics::CancelRemoteTasks()
{
    for (auto it = getDataTasks.begin(); it != getDataTasks.end();)
    {
        GetDataTask& task = it->second;
        DAVA::Logger::Debug("cancel remote task, key %s", it->first.ToString().c_str());
        if (task.dataStatus != GetDataTask::READY)
        {
            auto itDel = it++;
            CancelGetTask(itDel);
        }
        else
        {
            ++it;
        }
    }
}

void ServerLogics::ProcessLazyTasks()
{
    if (IsRemoteServerConnected())
    {
        for (WarmupTask& task : warmupTasks)
        {
            DAVA::Logger::Debug("process lazy warmup, key %s", task.key.ToString().c_str());
            clientProxy->RequestWarmingUp(task.key);
        }
    }
    warmupTasks.clear();
}

void ServerLogics::Update()
{
    ProcessLazyTasks();

    if (dataBase)
    {
        dataBase->Update();
    }
}

bool ServerLogics::IsRemoteServerConnected() const
{
    return (clientProxy && clientProxy->ChannelIsOpened());
}
