#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include <enet/enet.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
ElasticBuffer::ElasticBuffer()
    : size(PrimaryBuffSize)
    , buff(std::make_unique<uint8[]>(size))
    , idx(0)
{
}

ElasticBuffer::~ElasticBuffer()
{
    if (idx > 0)
    {
        Logger::Warning("Release fallback buffer level:%lu size:%lu", idx, offset);
    }
}

ElasticBuffer::ElasticBuffer(uint32 size, uint32 idx)
    : size(size)
    , buff(std::make_unique<uint8[]>(size))
    , idx(idx)
{
}

void ElasticBuffer::Reset()
{
    offset = 0;
    fallback.reset();
}

const uint8* ElasticBuffer::Insert(const uint8* srcBuff, uint32 srcSize, uint32 align)
{
    DVASSERT(IsPowerOf2(align));

    uint32 dstSize = srcSize;
    if (0 != ((align - 1) & dstSize))
    {
        dstSize &= ~(align - 1);
        dstSize += align;
    }

    if (dstSize <= size - offset)
    {
        uint8* insertPos = buff.get() + offset;
        Memcpy(insertPos, srcBuff, srcSize);
        offset += dstSize;
        DVASSERT(offset <= size);
        return insertPos;
    }

    if (!fallback)
    {
        if (idx >= ExtPageMaxCount || dstSize > FallbackBuffSize)
        {
            return nullptr;
        }

        fallback.reset(new ElasticBuffer(FallbackBuffSize, idx + 1));
    }

    return fallback->Insert(srcBuff, srcSize);
}

uint32 ElasticBuffer::GetOffset() const
{
    return offset;
}

uint32 ElasticBuffer::GetFallbackCount() const
{
    if (fallback)
    {
        return 1 + fallback->GetFallbackCount();
    }
    return 0;
}

const ElasticBuffer& ElasticBuffer::GetTail() const
{
    if (fallback)
    {
        return fallback->GetTail();
    }
    return *this;
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaReplicationSystemBase)
{
    ReflectionRegistrator<NetworkDeltaReplicationSystemBase>::Begin()
    .End();
}

NetworkDeltaReplicationSystemBase::NetworkDeltaReplicationSystemBase(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
}

} //namespace DAVA
