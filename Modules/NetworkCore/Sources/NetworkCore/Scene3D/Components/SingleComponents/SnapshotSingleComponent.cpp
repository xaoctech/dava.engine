#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>

struct ServerDiffCacheKey
{
    DAVA::NetworkID entityId;
    DAVA::uint32 frameIdBase;

    bool operator==(const ServerDiffCacheKey& k) const
    {
        return entityId == k.entityId && frameIdBase == k.frameIdBase;
    }
};

namespace std
{
template <>
struct hash<ServerDiffCacheKey>
{
    std::size_t operator()(const ServerDiffCacheKey& k) const
    {
#if 1
        // leave high 8 bits for frameId and fill others with entityId
        return (k.frameIdBase << 24 | static_cast<DAVA::uint32>(k.entityId));
#else
        // using Cantor pairing function
        // see https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
        return ((k.frameIdBase + k.entityId) * (k.frameIdBase + k.entityId + 1) >> 1) + k.frameIdBase
#endif
    }
};
} // namespace std

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSingleComponent)
{
    ReflectionRegistrator<SnapshotSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace SnapshotSingleComponentDetails
{
class ServerDiffCache
{
public:
    struct CachedDiff
    {
        void* buff = nullptr;
        size_t size = 0;
    };

    CachedDiff GetCache(const SnapshotSingleComponent::CreateDiffParams& params)
    {
        if (params.privacy == M::Privacy::PUBLIC && params.frameId == cacheFrameId)
        {
            auto it = cache.find({ params.entityId, params.frameIdBase });
            if (it != cache.end())
            {
                cacheHitCount++;
                return it->second;
            }
        }

        return CachedDiff();
    }

    void SetCache(const SnapshotSingleComponent::CreateDiffParams& params)
    {
        if (params.frameId != cacheFrameId)
        {
            cacheFrameId = params.frameId;
            cache.clear();
            memPoolPos = 0;
        }

        if (params.privacy == M::Privacy::PUBLIC)
        {
            DVASSERT(params.entityId != NetworkID::INVALID);
            DVASSERT(params.buffSize > 0);

            size_t spaceLeft = memPool.size() - memPoolPos;

            if (params.outDiffSize < spaceLeft)
            {
                void* cacheBuff = &memPool[memPoolPos];
                ::memcpy(cacheBuff, params.buff, params.outDiffSize);
                memPoolPos += params.outDiffSize;

                ServerDiffCacheKey key{ params.entityId, params.outFrameIdBase };
                CachedDiff val{ cacheBuff, static_cast<uint32>(params.outDiffSize) };

                cache[key] = val;
            }
        }
    }

private:
    uint32 cacheFrameId = 0;
    size_t cacheHitCount = 0;
    size_t memPoolPos = 0;
    std::array<uint8, 1024 * 1024 * 4> memPool; // 4 mb

    UnorderedMap<ServerDiffCacheKey, CachedDiff> cache;
};

Snapshot* GetSnapshot(SnapshotSingleComponent::SnapshotHistory& history, size_t& pos, uint32 frameId, bool create)
{
    Snapshot* ret = nullptr;

    DVASSERT(pos < history.size());
    uint32 curFrameId = history[pos].frameId;
    uint32 historySize = history.size();

    // requested current frame
    if (frameId == curFrameId)
    {
        ret = &history[pos];
    }
    // requested past frame
    else if (frameId < curFrameId)
    {
        uint32 offset = curFrameId - frameId;
        if (offset < historySize)
        {
            uint32 p = 0;
            if (offset <= pos)
            {
                p = pos - offset;
            }
            else
            {
                p = historySize - (offset - pos);
            }

            // when creation requested, check if pos is free
            // and initialize it for specified frameId
            if (create && history[p].frameId == 0)
            {
                history[p].Init(frameId);
            }

            if (history[p].frameId == frameId)
            {
                ret = &history[p];
            }
        }
        else
        {
            // Can't create past frame, that is out of history bounds
            ret = nullptr;
        }
    }
    // requested future frame
    else
    {
        // can be get only if `create` flag is true
        if (create)
        {
            uint32 offset = frameId - curFrameId;
            if (offset < historySize)
            {
                for (size_t i = 0; i < offset; ++i)
                {
                    pos++; // override incoming `pos`
                    if (pos >= historySize)
                    {
                        pos = 0;
                    }

                    history[pos].Clear();
                    DVASSERT(history[pos].frameId == 0);
                }

                history[pos].Init(frameId);
                ret = &history[pos];
            }
            else
            {
                for (size_t i = 0; i < historySize; ++i)
                {
                    history[i].Clear();
                    DVASSERT(history[i].frameId == 0);
                }

                pos = historySize - 1; // override incoming `pos`

                history[pos].Init(frameId);
                ret = &history[pos];
            }
        }
    }

    return ret;
}
}

SnapshotSingleComponent::SnapshotSingleComponent()
{
    serverHistory.resize(128);
    clientHistory.resize(64);

    ResetServerHistory();
    ResetClientHistory();

    serverDiffCache.reset(new SnapshotSingleComponentDetails::ServerDiffCache());
}

Snapshot* SnapshotSingleComponent::GetServerSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(serverHistory, serverHistoryPos, frameId, false);
}

Snapshot* SnapshotSingleComponent::GetClientSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(clientHistory, clientHistoryPos, frameId, false);
}

Snapshot* SnapshotSingleComponent::CreateServerSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(serverHistory, serverHistoryPos, frameId, true);
}

Snapshot* SnapshotSingleComponent::CreateClientSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(clientHistory, clientHistoryPos, frameId, true);
}

void SnapshotSingleComponent::ResetServerHistory()
{
    size_t sz = serverHistory.size();
    for (size_t i = 0; i < sz; ++i)
    {
        serverHistory[i].Clear();
    }
    serverHistoryPos = 0;
}

void SnapshotSingleComponent::ResetClientHistory()
{
    size_t sz = clientHistory.size();
    for (size_t i = 0; i < sz; ++i)
    {
        clientHistory[i].Clear();
    }
    clientHistoryPos = 0;
}

bool SnapshotSingleComponent::GetServerDiff(CreateDiffParams& params)
{
    DVASSERT(params.buff != nullptr);
    DVASSERT(params.buffSize > 0);
    DVASSERT(params.frameIdBase <= params.frameId);

    Snapshot* snapshotBase = GetServerSnapshot(params.frameIdBase);

    if (nullptr == snapshotBase)
    {
        params.outFrameIdBase = 0;
    }
    else
    {
        params.outFrameIdBase = params.frameIdBase;
    }

    // WARNING: returning cache depends on params.outFrameIdBase,
    // so make sure that its filled before calling cache getter

    SnapshotSingleComponentDetails::ServerDiffCache::CachedDiff diff = serverDiffCache->GetCache(params);
    if (nullptr != diff.buff)
    {
        if (diff.size <= params.buffSize)
        {
            ::memcpy(params.buff, diff.buff, diff.size);
            params.outDiffSize = diff.size;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        Snapshot* snapshot = GetServerSnapshot(params.frameId);

        DVASSERT(snapshot != nullptr);
        DVASSERT(snapshot != snapshotBase);

        size_t diffSz = SnapshotUtils::CreateSnapshotDiff(snapshotBase, snapshot, params.entityId, params.privacy, params.buff, params.buffSize);

        if (diffSz > 0)
        {
            params.outDiffSize = diffSz;
            serverDiffCache->SetCache(params);

            return true;
        }
        else
        {
            return false;
        }
    }
}

bool SnapshotSingleComponent::ApplyServerDiff(ApplyDiffParams& params, SnapshotApplyCallback cb)
{
    static Snapshot dummySnapshot;
    static uint32 lastFrameId;

    DVASSERT(params.buff != nullptr);
    DVASSERT(params.buffSize > 0);
    DVASSERT(params.frameIdBase <= params.frameId);

    Snapshot* snapshot = CreateServerSnapshot(params.frameId);
    Snapshot* snapshotBase = GetServerSnapshot(params.frameIdBase);

    // we should reject diff if target snapshot didn't created
    if (nullptr == snapshot)
    {
        return false;
    }

    // we should reject incremental diff (non-zero base frameId)
    // in some cases... so check them
    if (params.frameIdBase != 0)
    {
        // reject if base snapshot isn't found
        if (nullptr == snapshotBase)
        {
            return false;
        }

        // reject if there is no appropriate entity in base snapshot
        if (nullptr == snapshotBase->FindEntity(params.entityId))
        {
            return false;
        }
    }

    params.outDiffSize = SnapshotUtils::ApplySnapshotDiff(snapshotBase, snapshot, params.entityId, params.buff, params.buffSize, cb);
    return (params.outDiffSize > 0);
}
} // namespace DAVA
