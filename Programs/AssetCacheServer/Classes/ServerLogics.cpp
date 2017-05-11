#include "ServerLogics.h"
#include "ServerCacheEntry.h"

#include <Tools/AssetCache/ChunkSplitter.h>

#include <Concurrency/LockGuard.h>
#include <Logger/Logger.h>
#include <Utils/StringFormat.h>

void ServerLogics::Init(DAVA::AssetCache::ServerNetProxy* server_, const DAVA::String& serverName_, DAVA::AssetCache::ClientNetProxy* client_, CacheDB* dataBase_)
{
    serverProxy = server_;
    serverName = serverName_;
    clientProxy = client_;
    dataBase = dataBase_;
}

void ServerLogics::OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint64 dataSize, DAVA::uint32 numOfChunks)
{
    DAVA::List<AddDataTask>::iterator it = GetOrCreateAddTask(channel, key);
    AddDataTask& task = *it;

    auto DiscardTask = [&]()
    {
        serverProxy->SendAddedToCache(channel, key, false);
        addDataTasks.erase(it);
    };

    auto Error = [&](const char* err)
    {
        DAVA::Logger::Error("Wrong add request: %s. Client %p, key %s", err, channel, key.ToString().c_str());
        DiscardTask();
    };

    if (task.chunksOverall != 0 || task.bytesOverall != 0)
    {
        Error("add data info was already received with given key and channel");
        return;
    }

    if (dataSize == 0 || numOfChunks == 0)
    {
        Error("both data size and number of chunks are zero");
        return;
    }

    if (dataSize > dataBase->GetStorageSize())
    {
        DAVA::Logger::Warning("Inserted data size %u is bigger than max storage size %u", dataSize, dataBase->GetStorageSize());
        DiscardTask();
        return;
    }

    task.bytesOverall = dataSize;
    task.chunksOverall = numOfChunks;

    serverProxy->SendAddedToCache(channel, key, true);
}

void ServerLogics::OnAddChunkToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunkData)
{
    using namespace DAVA;

    DAVA::List<AddDataTask>::iterator it = GetOrCreateAddTask(channel, key);
    AddDataTask& task = *it;

    auto DiscardTask = [&]()
    {
        serverProxy->SendAddedToCache(channel, key, false);
        addDataTasks.erase(it);
    };

    auto Error = [&](const char* err)
    {
        Logger::Error("Wrong add chunk request: %s. Client %p, key %s chunk#%u", err, channel, key.ToString().c_str(), chunkNumber);
        DiscardTask();
    };

    if (task.chunksReceived != chunkNumber)
    {
        Error(Format("chunk #u was expected", task.chunksReceived).c_str());
        return;
    }

    uint32 chunkSize = static_cast<uint32>(chunkData.size());
    uint32 written = task.receivedData->Write(chunkData.data(), chunkSize);
    if (written != chunkSize)
    {
        Error(Format("can't append %u bytes", chunkSize).c_str());
        return;
    }

    task.bytesReceived += chunkSize;
    ++task.chunksReceived;
    Logger::Debug("Add chunk #%u received: %u bytes. Overall received %u, remaining %u", chunkNumber, chunkData.size(), task.bytesReceived, task.bytesOverall - task.bytesReceived);

    if (task.chunksReceived == task.chunksOverall)
    {
        if (task.bytesReceived != task.bytesOverall)
        {
            Error(Format("unexpected final bytes count: %u (expected %u bytes)", task.bytesReceived, task.bytesOverall).c_str());
            return;
        }

        AssetCache::CachedItemValue value;
        task.receivedData->Seek(0, File::SEEK_FROM_START);
        value.Deserialize(task.receivedData);
        if (value.IsEmpty() || !value.IsValid())
        {
            Error("Received data is empty or invalid");
            return;
        }

        AssetCache::CachedItemValue::Description description = value.GetDescription();
        description.addingChain += "/" + serverName;
        value.SetDescription(description);

        if (value.GetSize() > dataBase->GetStorageSize())
        {
            Logger::Warning("Inserted data size %u is bigger than max storage size %u", value.GetSize(), dataBase->GetStorageSize());
            DiscardTask();
            return;
        }

        dataBase->Insert(key, value);
        addDataTasks.erase(it);
        remoteAddDataTasks.emplace(key, RemoteAddDataTask());
    }

    serverProxy->SendAddedToCache(channel, key, true);
}

DAVA::List<ServerLogics::AddDataTask>::iterator ServerLogics::GetOrCreateAddTask(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key)
{
    using namespace DAVA;
    List<ServerLogics::AddDataTask>::iterator it = std::find_if(addDataTasks.begin(), addDataTasks.end(), [&](const AddDataTask& task)
                                                                {
                                                                    return (task.channel == channel && task.key == key);
                                                                });

    if (it == addDataTasks.end())
    {
        it = addDataTasks.emplace(addDataTasks.end(), AddDataTask());
        it->channel = channel;
        it->key = key;
        it->receivedData = DynamicMemoryFile::Create(File::CREATE | File::WRITE | File::READ);
    }

    return it;
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

            AssetCache::CachedItemValue& value = entry->GetValue();
            AssetCache::CachedItemValue::Description description = value.GetDescription();
            description.receivingChain += "/" + serverName;
            value.SetDescription(description);
            value.Serialize(task.serializedData);
            task.dataStatus = GetDataTask::READY;
            task.bytesOverall = task.bytesReady = task.serializedData->GetSize();
            task.chunksOverall = task.chunksReady = AssetCache::ChunkSplitter::GetNumberOfChunks(task.bytesOverall);
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
    DAVA::Logger::Debug("Requested data with key %s", key.ToString().c_str());
    GetTasksMap::iterator taskIter = GetOrCreateGetTask(key);
    if (taskIter != getDataTasks.end())
    {
        GetDataTask& task = taskIter->second;
        if (task.dataStatus == GetDataTask::WAITING_DATA_INFO)
        {
            DAVA::Logger::Debug("task is in waiting_data state");
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
    Logger::Debug("Chunk #%u is requested, key %s", chunkNumber, key.ToString().c_str());

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
            Vector<uint8> chunk = AssetCache::ChunkSplitter::GetChunk(task.serializedData->GetDataVector(), chunkNumber);
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
        Error("data was not requested");
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
    else if (dataSize > dataBase->GetStorageSize())
    {
        DAVA::Logger::Warning("Inserted data size %u is bigger than max storage size %u", dataSize, dataBase->GetStorageSize());
        CancelGetTask(taskIter);
        return;
    }
    else
    {
        task.bytesOverall = dataSize;
        task.chunksOverall = numOfChunks;
        SendInfoToClients(taskIter);
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
        Error("data was not requested", taskIter);
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
        Logger::Debug("Empty chunk is received. GetData task will be canceled for all clients");
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
        task.serializedData->Seek(0, File::SEEK_FROM_START);
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

void ServerLogics::OnAddedToCache(const DAVA::AssetCache::CacheItemKey& key, bool added)
{
    RemoteAddTasksMap::iterator itTask = remoteAddDataTasks.find(key);
    if (itTask != remoteAddDataTasks.end())
    {
        RemoteAddDataTask& task = itTask->second;
        if (++task.chunksSent == task.chunksOverall)
        {
            DAVA::Logger::Debug("All chunks are sent. Removing remote add task");
            remoteAddDataTasks.erase(itTask);
            return;
        }

        if (added)
        {
            bool sentOk = SendAddChunkToRemote(itTask);
            if (!sentOk)
            {
                remoteAddDataTasks.erase(itTask);
                return;
            }
        }
        else
        {
            DAVA::Logger::Debug("Chunk was not added to remote cache. Removing task");
            remoteAddDataTasks.erase(itTask);
            return;
        }
    }
    else
    {
        DAVA::Logger::Error("Answer for unknown remote add task is received. Key %s", key.ToString().c_str());
    }
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

void ServerLogics::SendInfoToClients(ServerLogics::GetTasksMap::iterator taskIt)
{
    DVASSERT(taskIt != getDataTasks.end());

    const DAVA::AssetCache::CacheItemKey& key = taskIt->first;
    GetDataTask& task = taskIt->second;

    for (std::pair<DAVA::Net::IChannel* const, GetDataTask::ClientStatus>& client : task.clients)
    {
        if (client.second.status == GetDataTask::WAITING_DATA_INFO)
        {
            DAVA::Logger::Debug("sending info: %u bytes, %u chunks", task.bytesOverall, task.chunksOverall);
            serverProxy->SendDataInfo(client.first, key, task.bytesOverall, task.chunksOverall);
            client.second.status = GetDataTask::READY;
        }
    }
}

void ServerLogics::SendChunkToClients(ServerLogics::GetTasksMap::iterator taskIt, DAVA::uint32 chunkNumber, const DAVA::Vector<DAVA::uint8>& chunk)
{
    DAVA::Logger::Debug("%s", __FUNCTION__);
    DVASSERT(taskIt != getDataTasks.end());

    const DAVA::AssetCache::CacheItemKey& key = taskIt->first;
    GetDataTask& task = taskIt->second;

    for (std::pair<DAVA::Net::IChannel* const, GetDataTask::ClientStatus>& client : task.clients)
    {
        if (client.second.status == GetDataTask::WAITING_NEXT_CHUNK && client.second.waitingChunk == chunkNumber)
        {
            DAVA::Logger::Debug("sending chunk #%u size %u", chunkNumber, chunk.size());
            serverProxy->SendChunk(client.first, key, chunkNumber, chunk);
            client.second.status = GetDataTask::READY;
        }
    }
}

bool ServerLogics::SendAddInfoToRemote(RemoteAddTasksMap::iterator taskIt)
{
    using namespace DAVA;

    DVASSERT(taskIt != remoteAddDataTasks.end());

    const AssetCache::CacheItemKey& key = taskIt->first;
    RemoteAddDataTask& task = taskIt->second;

    ServerCacheEntry* entry = dataBase->Get(key);
    if (entry)
    {
        Logger::Debug("Create remote add task using local data");
        task.serializedData = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);
        AssetCache::CachedItemValue& value = entry->GetValue();
        value.Serialize(task.serializedData);
        uint64 dataSize = task.serializedData->GetSize();
        task.chunksSent = 0;
        task.chunksOverall = AssetCache::ChunkSplitter::GetNumberOfChunks(dataSize);
        return clientProxy->RequestAddData(key, dataSize, task.chunksOverall);
    }
    else
    {
        Logger::Warning("Data with key %s is not found in DB", key.ToString().c_str());
        return false;
    }
}

bool ServerLogics::SendAddChunkToRemote(RemoteAddTasksMap::iterator taskIt)
{
    using namespace DAVA;

    DVASSERT(taskIt != remoteAddDataTasks.end());

    const AssetCache::CacheItemKey& key = taskIt->first;
    RemoteAddDataTask& task = taskIt->second;

    Vector<uint8> chunk = AssetCache::ChunkSplitter::GetChunk(task.serializedData->GetDataVector(), task.chunksSent);
    return clientProxy->RequestAddNextChunk(key, task.chunksSent, chunk);
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

    addDataTasks.remove_if([clientChannel](const AddDataTask& task)
                           {
                               return task.channel == clientChannel;
                           });
}

void ServerLogics::CancelRemoteTasks()
{
    for (auto it = getDataTasks.begin(); it != getDataTasks.end();)
    {
        GetDataTask& task = it->second;
        if (task.dataStatus != GetDataTask::READY)
        {
            DAVA::Logger::Debug("cancel remote get task, key %s", it->first.ToString().c_str());
            auto itDel = it++;
            CancelGetTask(itDel);
        }
        else
        {
            ++it;
        }
    }

    remoteAddDataTasks.clear();
}

void ServerLogics::ProcessLazyTasks()
{
    if (IsRemoteServerConnected())
    {
        for (WarmupTask& task : warmupTasks)
        {
            DAVA::Logger::Debug("process lazy warmup task, key %s", task.key.ToString().c_str());
            clientProxy->RequestWarmingUp(task.key);
        }
        warmupTasks.clear();

        for (RemoteAddTasksMap::iterator it = remoteAddDataTasks.begin(); it != remoteAddDataTasks.end();)
        {
            if (it->second.chunksOverall == 0)
            {
                DAVA::Logger::Debug("process lazy remote add task, key %s", it->first.ToString().c_str());
                bool sentOk = SendAddInfoToRemote(it);
                if (!sentOk)
                {
                    auto itDel = it++;
                    remoteAddDataTasks.erase(itDel);
                    continue;
                }
            }
            ++it;
        }
    }
    else
    {
        warmupTasks.clear();
        remoteAddDataTasks.clear();
    }
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
