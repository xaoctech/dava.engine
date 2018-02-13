#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotStat.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Private/EntitySnapshotReader.h"

#include <Base/BitReader.h>
#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
EntitySnapshotReader::EntitySnapshotReader(BitReader& bitStream)
    : bstream(bitStream)
{
}

EntitySnapshotReader::EntitySnapshotReader(BitReader& bitStream, NetworkID rootEntityId, const Snapshot* snapshot1, Snapshot* snapshot2, SnapshotApplyCallback callback)
    : bstream(bitStream)
    , rootEntityId(rootEntityId)
    , snapshot1(snapshot1)
    , snapshot2(snapshot2)
    , callback(callback)
{
}

size_t EntitySnapshotReader::Read()
{
    DVASSERT(snapshot2 != nullptr);
    DVASSERT(callback != nullptr);

    const bool isFullDiff = bstream.ReadBits(1) != 0;
    const bool isSizePresent = bstream.ReadBits(1) != 0;
#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    clientSnapshotStat->ntotal += 1;
    clientSnapshotStat->nfull += isFullDiff;
    clientSnapshotStat->nempty += !isSizePresent;
#endif
    if (snapshot1 == nullptr && !isFullDiff)
    {
        // incremental diff cannot be applied without base snapshot
        return 0;
    }

    uint32 size = 0;
    SnapshotApplyParam param{};
    SnapshotEntity* entity = nullptr;
    ObjectMarker marker = static_cast<ObjectMarker>(bstream.ReadBits(2));
    switch (marker)
    {
    case ObjectMarker::MARKER_CHANGED:
        param.cmd = SnapshotApplyCommand::ENTITY_TOUCHED;
        param.entityParam.entityId = rootEntityId;
        callback(param);

        if (isSizePresent)
            size = bstream.ReadBits(11);

        entity = CopyEntitySnapshotRecursive(rootEntityId, NetworkID::SCENE_ID);
        ReadComponents(rootEntityId, entity);
        ReadChildren(entity);
        break;
    case ObjectMarker::MARKER_ADDED:
        param.cmd = SnapshotApplyCommand::ENTITY_ADDED;
        param.entityParam.entityId = rootEntityId;
        param.entityParam.parentId = NetworkID::SCENE_ID;
        callback(param);
        DVASSERT(param.entityParam.outEntity != nullptr);

        if (isSizePresent)
            size = bstream.ReadBits(11);

        entity = CreateEntitySnapshot(param.entityParam.outEntity, NetworkID::SCENE_ID);
        ReadComponents(rootEntityId, entity);
        ReadChildren(entity);
        break;
    case ObjectMarker::MARKER_REMOVED:
        param.cmd = SnapshotApplyCommand::ENTITY_REMOVED;
        param.entityParam.entityId = rootEntityId;
        callback(param);
        break;
    case ObjectMarker::MARKER_END:
        param.cmd = SnapshotApplyCommand::ENTITY_TOUCHED;
        param.entityParam.entityId = rootEntityId;
        callback(param);

        entity = CopyEntitySnapshotRecursive(rootEntityId, NetworkID::SCENE_ID);
        break;
    }

    bstream.ReadAlignmentBits();
    size_t n = bstream.GetBytesRead();
    return n;
}

size_t EntitySnapshotReader::GetSize()
{
    uint32 size = 1;
    bstream.ReadBits(1);
    const bool isSizePresent = bstream.ReadBits(1) != 0;
    if (isSizePresent)
    {
        bstream.ReadBits(2);
        size = bstream.ReadBits(11);
    }
    return size;
}

void EntitySnapshotReader::ReadChildren(SnapshotEntity* rootEntity)
{
    SnapshotApplyParam param{};
    ObjectMarker marker = static_cast<ObjectMarker>(bstream.ReadBits(2));
    while (marker != ObjectMarker::MARKER_END)
    {
        NetworkID parentEntityId(bstream.ReadBits(32));
        NetworkID entityId(bstream.ReadBits(32));
        if (marker == ObjectMarker::MARKER_CHANGED)
        {
            param.cmd = SnapshotApplyCommand::ENTITY_TOUCHED;
            param.entityParam.parentId = parentEntityId;
            param.entityParam.entityId = entityId;
            callback(param);

            SnapshotEntity* entity = snapshot2->FindEntity(entityId);
            DVASSERT(entity != nullptr);
            ReadComponents(entityId, entity);
        }
        else if (marker == ObjectMarker::MARKER_ADDED)
        {
            param.cmd = SnapshotApplyCommand::ENTITY_ADDED;
            param.entityParam.parentId = parentEntityId;
            param.entityParam.entityId = entityId;
            callback(param);
            DVASSERT(param.entityParam.outEntity != nullptr);

            SnapshotEntity* entity = CreateEntitySnapshot(param.entityParam.outEntity, parentEntityId);
            ReadComponents(entityId, entity);
        }
        else if (marker == ObjectMarker::MARKER_REMOVED)
        {
            param.cmd = SnapshotApplyCommand::ENTITY_REMOVED;
            param.entityParam.parentId = parentEntityId;
            param.entityParam.entityId = entityId;
            callback(param);

            snapshot2->RemoveEntity(parentEntityId, entityId);
        }

        marker = static_cast<ObjectMarker>(bstream.ReadBits(2));
    }
}

void EntitySnapshotReader::ReadComponents(NetworkID entityId, SnapshotEntity* entity)
{
    SnapshotApplyParam param{};
    ObjectMarker marker = static_cast<ObjectMarker>(bstream.ReadBits(2));
    while (marker != ObjectMarker::MARKER_END)
    {
        SnapshotComponentKey componentKey;
        componentKey.id = CompressionUtils::DecompressVarInt<uint16>(bstream);
        componentKey.index = CompressionUtils::DecompressVarInt<uint16>(bstream);

        if (marker == ObjectMarker::MARKER_CHANGED)
        {
            auto foundAt = entity->components.find(componentKey);
            DVASSERT(foundAt != entity->components.end());

            param.cmd = SnapshotApplyCommand::COMPONENT_CHANGED;
            param.componentParam.entityId = entityId;
            param.componentParam.componentKey = componentKey;
            callback(param);

            Vector<SnapshotField>& fields = foundAt->second.fields;
            ReadFieldsDelta(componentKey, fields);
        }
        else if (marker == ObjectMarker::MARKER_ADDED)
        {
            DVASSERT(entity->components.find(componentKey) == entity->components.end());

            param.cmd = SnapshotApplyCommand::COMPONENT_ADDED;
            param.componentParam.entityId = entityId;
            param.componentParam.componentKey = componentKey;
            callback(param);
            DVASSERT(param.componentParam.outComponent != nullptr);

            SnapshotComponent* component = &entity->components[componentKey];
            component->Fill(param.componentParam.outComponent);

            ReadFields(componentKey, component->fields);
        }
        else if (marker == ObjectMarker::MARKER_REMOVED)
        {
            param.cmd = SnapshotApplyCommand::COMPONENT_REMOVED;
            param.componentParam.entityId = entityId;
            param.componentParam.componentKey = componentKey;
            callback(param);

            DVASSERT(entity->components.find(componentKey) != entity->components.end());
            entity->components.erase(componentKey);
        }

        marker = static_cast<ObjectMarker>(bstream.ReadBits(2));
    }
}

void EntitySnapshotReader::ReadFieldsDelta(SnapshotComponentKey componentKey, Vector<SnapshotField>& fields)
{
#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    ComponentStatItem& componentStat = clientSnapshotStat->componentStat[componentKey.id];
    uint32 bitsBegin = bstream.GetBitsRead();
    uint32 bitsPayload = 0;
#endif

    const size_t nfields = fields.size();
    for (size_t i = 0; i < nfields; ++i)
    {
        bool fieldWasWritten = bstream.ReadBits(1) != 0;
        if (fieldWasWritten)
        {
            Any& value = fields[i].value;
            const CompressionScheme& scheme = fields[i].compression;
            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(value);
            DVASSERT(compressor != nullptr, Format("Compressor is not registered for '%s'", value.GetType()->GetName()).c_str());

#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
            uint32 x = bstream.GetBitsRead();
            compressor->DecompressDelta(value, value, scheme, bstream);
            uint32 n = bstream.GetBitsRead() - x;

            FastName typeName = compressor->GetTypeName();
            TypeStatItem& stat = componentStat.typeStat[typeName];
            stat.count += 1;
            stat.bits += n;
            bitsPayload += n;
#else
            compressor->DecompressDelta(value, value, scheme, bstream);
#endif
        }
    }

#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    uint32 n = bstream.GetBitsRead() - bitsBegin;
    componentStat.count += 1;
    componentStat.bits += n;
    componentStat.bitsPayload += bitsPayload;
#endif
}

void EntitySnapshotReader::ReadFields(SnapshotComponentKey componentKey, Vector<SnapshotField>& fields)
{
#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    ComponentStatItem& componentStat = clientSnapshotStat->componentStat[componentKey.id];
    uint32 bitsBegin = bstream.GetBitsRead();
    uint32 bitsPayload = 0;
#endif

    const size_t nfields = fields.size();
    for (size_t i = 0; i < nfields; ++i)
    {
        bool fieldWasWritten = bstream.ReadBits(1) != 0;
        if (fieldWasWritten)
        {
            const CompressionScheme& scheme = fields[i].compression;
            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(fields[i].value);
            DVASSERT(compressor != nullptr, Format("Compressor is not registered for '%s'", fields[i].value.GetType()->GetName()).c_str());

            Any& value = fields[i].value;
#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
            uint32 x = bstream.GetBitsRead();
            compressor->DecompressFull(value, scheme, bstream);
            uint32 n = bstream.GetBitsRead() - x;

            FastName typeName = compressor->GetTypeName();
            TypeStatItem& stat = componentStat.typeStat[typeName];
            stat.count += 1;
            stat.countNew += 1;
            stat.bits += n;
            bitsPayload += n;
#else
            compressor->DecompressFull(value, scheme, bstream);
#endif
        }
    }

#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    uint32 n = bstream.GetBitsRead() - bitsBegin;
    componentStat.count += 1;
    componentStat.countNew += 1;
    componentStat.bits += n;
    componentStat.bitsPayload += bitsPayload;
#endif
}

SnapshotEntity* EntitySnapshotReader::CopyEntitySnapshotRecursive(NetworkID entityId, NetworkID parentId)
{
    DVASSERT(entityId != NetworkID::INVALID);

    SnapshotEntity* targetEntity = nullptr;
    const SnapshotEntity* sourceEntity = snapshot1->FindEntity(entityId);

    if (nullptr != sourceEntity)
    {
        DVASSERT(sourceEntity->entity != nullptr);

        targetEntity = snapshot2->AddEntity(sourceEntity->entity, parentId);
        targetEntity->components = sourceEntity->components;

        for (size_t i = 0; i < sourceEntity->children.size(); ++i)
        {
            CopyEntitySnapshotRecursive(sourceEntity->children[i], entityId);
        }
    }

    return targetEntity;
}

SnapshotEntity* EntitySnapshotReader::CreateEntitySnapshot(Entity* entity, NetworkID parentId)
{
    SnapshotEntity* resultEntity = snapshot2->AddEntity(entity, parentId);
    return resultEntity;
}

} // namespace DAVA
