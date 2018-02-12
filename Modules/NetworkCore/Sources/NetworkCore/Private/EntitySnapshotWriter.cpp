#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Private/EntitySnapshotWriter.h"

#include <Base/BitWriter.h>
#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
EntitySnapshotWriter::EntitySnapshotWriter(BitWriter& bitStream, NetworkID rootEntityId, M::Privacy privacy, const Snapshot* snapshot1, const Snapshot* snapshot2)
    : bstream(bitStream)
    , rootEntityId(rootEntityId)
    , privacy(privacy)
    , snapshot1(snapshot1)
    , snapshot2(snapshot2)
{
    DVASSERT(snapshot2 != nullptr);
}

size_t EntitySnapshotWriter::Write()
{
    // write diff type
    const bool isFullDiff = snapshot1 == nullptr;
    bstream.WriteBits(isFullDiff, 1);
    // reserve room for size present flag
    bstream.WriteBits(0, 1);

    const SnapshotEntity* rootEntity1 = snapshot1 != nullptr ? snapshot1->FindEntity(rootEntityId) : nullptr;
    const SnapshotEntity* rootEntity2 = snapshot2->FindEntity(rootEntityId);

    // Both rootEntities can be `nullptr` in one case: when rootEntityId isn't exists
    // in target snapshot and there is no base snapshot at all - it's `nullptr`.
    // In this case rootEntityId will be written with REMOVED flag that is
    // absolutely expected behavior.

    bool smthWritten = WriteRootEntity(rootEntity1, rootEntity2);
    bstream.WriteAlignmentBits();
    bstream.Flush();
    if (!bstream.IsOverflowed())
    {
        uint32 size;
        if (smthWritten)
        {
            size = bstream.GetBytesWritten();
            DVASSERT(0 < size && size < (1 << 11));
            bstream.PatchBits(1, 1, 1);
            bstream.PatchBits(4, size, 11);
            bstream.Flush();
        }
        else
        {
            uint32 bits = bstream.GetBitsWritten();
            bstream.Rewind(bits - 4);
            bstream.WriteAlignmentBits();
            bstream.Flush();
            size = bstream.GetBytesWritten();
        }
        return size;
    }
    return 0;
}

bool EntitySnapshotWriter::WriteRootEntity(const SnapshotEntity* entity1, const SnapshotEntity* entity2)
{
    bool smthWritten = false;
    if (entity1 != nullptr && entity2 != nullptr)
    {
        uint32 patchOffset = bstream.GetBitsWritten();
        bstream.WriteBits(ObjectMarker::MARKER_END, 2);
        bstream.WriteBits(0, 11);

        const SnapshotEntity::Components& components1 = entity1->components;
        const SnapshotEntity::Components& components2 = entity2->components;
        smthWritten = WriteComponents(components1, components2);

        if (smthWritten)
        {
            bstream.PatchBits(patchOffset, ObjectMarker::MARKER_CHANGED, 2);
            bstream.WriteBits(ObjectMarker::MARKER_END, 2);
        }

        if (rootEntityId != NetworkID::SCENE_ID)
        {
            smthWritten |= VisitChildren(rootEntityId, entity1->children, entity2->children);
        }

        // footer
        bstream.WriteBits(ObjectMarker::MARKER_END, 2);
    }
    else if (entity2 != nullptr)
    {
        bstream.WriteBits(ObjectMarker::MARKER_ADDED, 2);
        bstream.WriteBits(0, 11);

        smthWritten = true;
        for (const auto& o : entity2->components)
        {
            if (CheckPrivacy(o.second.privacy))
            {
                const SnapshotComponentKey& componentKey = o.first;
                const Vector<SnapshotField>& fields = o.second.fields;
                WriteComponentFieldsFull(componentKey, fields);
            }
        }
        bstream.WriteBits(ObjectMarker::MARKER_END, 2);

        if (rootEntityId != NetworkID::SCENE_ID)
        {
            VisitChildren(rootEntityId, Vector<NetworkID>{}, entity2->children);
        }

        // footer
        bstream.WriteBits(ObjectMarker::MARKER_END, 2);
    }
    else
    {
        bstream.WriteBits(ObjectMarker::MARKER_REMOVED, 2);
        smthWritten = false;
    }
    return smthWritten;
}

bool EntitySnapshotWriter::VisitChildren(NetworkID parentEntityId, const Vector<NetworkID>& children1, const Vector<NetworkID>& children2)
{
    const size_t size1 = children1.size();
    const size_t size2 = children2.size();
    size_t index1 = 0;
    size_t index2 = 0;

    bool smthWritten = false;
    while (index1 < size1 && index2 < size2)
    {
        if (children1[index1] == children2[index2])
        {
            const SnapshotEntity* entity1 = snapshot1->FindEntity(children1[index1]);
            const SnapshotEntity* entity2 = snapshot2->FindEntity(children2[index2]);
            DVASSERT(entity1 != nullptr && entity2 != nullptr);

            smthWritten |= WriteChildEntity(parentEntityId, children1[index1], entity1, entity2);

            index1 += 1;
            index2 += 1;
        }
        else if (children1[index1] > children2[index2])
        {
            const SnapshotEntity* entity2 = snapshot1->FindEntity(children2[index2]);
            DVASSERT(entity2 != nullptr);

            smthWritten |= WriteChildEntity(parentEntityId, children2[index2], nullptr, entity2);
            index2 += 1;
        }
        else
        {
            smthWritten |= WriteChildEntity(parentEntityId, children1[index1], nullptr, nullptr);
            index1 += 1;
        }
    }

    while (index1 < size1)
    {
        smthWritten |= WriteChildEntity(parentEntityId, children1[index1], nullptr, nullptr);
        index1 += 1;
    }

    while (index2 < size2)
    {
        const SnapshotEntity* entity2 = snapshot2->FindEntity(children2[index2]);
        DVASSERT(entity2 != nullptr);

        smthWritten |= WriteChildEntity(parentEntityId, children2[index2], nullptr, entity2);
        index2 += 1;
    }
    return smthWritten;
}

bool EntitySnapshotWriter::WriteChildEntity(NetworkID parentEntityId, NetworkID entityId, const SnapshotEntity* entity1, const SnapshotEntity* entity2)
{
    bool smthWritten = false;
    if (entity1 != nullptr && entity2 != nullptr)
    {
        bstream.WriteBits(ObjectMarker::MARKER_CHANGED, 2);
        bstream.WriteBits(static_cast<uint32>(parentEntityId), 32);
        bstream.WriteBits(static_cast<uint32>(entityId), 32);

        const SnapshotEntity::Components& components1 = entity1->components;
        const SnapshotEntity::Components& components2 = entity2->components;
        smthWritten = WriteComponents(components1, components2);

        if (smthWritten)
        {
            bstream.WriteBits(ObjectMarker::MARKER_END, 2);
        }
        else
        {
            bstream.Rewind(2 + 32 + 32);
        }
        smthWritten |= VisitChildren(entityId, entity1->children, entity2->children);
    }
    else if (entity2 != nullptr)
    {
        bstream.WriteBits(ObjectMarker::MARKER_ADDED, 2);
        bstream.WriteBits(static_cast<uint32>(parentEntityId), 32);
        bstream.WriteBits(static_cast<uint32>(entityId), 32);

        smthWritten = true;
        for (const auto& o : entity2->components)
        {
            if (CheckPrivacy(o.second.privacy))
            {
                const SnapshotComponentKey& componentKey = o.first;
                const Vector<SnapshotField>& fields = o.second.fields;
                WriteComponentFieldsFull(componentKey, fields);
            }
        }
        bstream.WriteBits(ObjectMarker::MARKER_END, 2);

        smthWritten |= VisitChildren(entityId, Vector<NetworkID>{}, entity2->children);
    }
    else
    {
        bstream.WriteBits(ObjectMarker::MARKER_REMOVED, 2);
        bstream.WriteBits(static_cast<uint32>(parentEntityId), 32);
        bstream.WriteBits(static_cast<uint32>(entityId), 32);

        smthWritten = true;
    }
    return smthWritten;
}

bool EntitySnapshotWriter::WriteComponents(const SnapshotEntity::Components& components1, const SnapshotEntity::Components& components2)
{
    struct X
    {
        SnapshotComponentKey key;
        M::Privacy privacy;
        const Vector<SnapshotField>* fields;
    };

    Vector<X> flat1;
    flat1.reserve(components1.size());
    for (auto& i : components1)
    {
        flat1.push_back(X{ i.first, i.second.privacy, &i.second.fields });
    }
    std::sort(std::begin(flat1), std::end(flat1), [](const X& l, const X& r) { return l.key < r.key; });

    Vector<X> flat2;
    flat2.reserve(components2.size());
    for (auto& i : components2)
    {
        flat2.push_back(X{ i.first, i.second.privacy, &i.second.fields });
    }
    std::sort(std::begin(flat2), std::end(flat2), [](const X& l, const X& r) { return l.key < r.key; });

    //////////////////////////////////////////////////////////////////////////

    size_t index1 = 0;
    size_t index2 = 0;
    bool smthChanged = false;
    const size_t size1 = flat1.size();
    const size_t size2 = flat2.size();
    while (index1 < size1 && index2 < size2)
    {
        const X& x1 = flat1[index1];
        const X& x2 = flat2[index2];
        if (x1.key == x2.key)
        {
            DVASSERT(x1.privacy == x2.privacy);
            if (CheckPrivacy(x1.privacy))
            {
                smthChanged |= WriteComponentFieldsDiff(x1.key, *x1.fields, *x2.fields);
            }
            index1 += 1;
            index2 += 1;
        }
        else if (x1.key < x2.key)
        {
            // component removed
            if (CheckPrivacy(x1.privacy))
            {
                smthChanged |= WriteComponentRemoved(x1.key);
            }
            index1 += 1;
        }
        else
        {
            // component added
            if (CheckPrivacy(x2.privacy))
            {
                smthChanged |= WriteComponentFieldsFull(x2.key, *x2.fields);
            }
            index2 += 1;
        }
    }

    while (index1 < size1)
    {
        // removed tail
        const X& x1 = flat1[index1];
        if (CheckPrivacy(x1.privacy))
        {
            smthChanged |= WriteComponentRemoved(x1.key);
        }
        index1 += 1;
    }

    while (index2 < size2)
    {
        // added tail
        const X& x2 = flat2[index2];
        if (CheckPrivacy(x2.privacy))
        {
            smthChanged |= WriteComponentFieldsFull(x2.key, *x2.fields);
        }
        index2 += 1;
    }
    return smthChanged;
}

bool EntitySnapshotWriter::WriteComponentRemoved(SnapshotComponentKey componentKey)
{
    // write component header
    bstream.WriteBits(ObjectMarker::MARKER_REMOVED, 2);
    CompressionUtils::CompressVarInt<uint16>(componentKey.id, bstream);
    CompressionUtils::CompressVarInt<uint16>(componentKey.index, bstream);
    return true;
}

bool EntitySnapshotWriter::WriteComponentFieldsDiff(SnapshotComponentKey componentKey, const Vector<SnapshotField>& fields1, const Vector<SnapshotField>& fields2)
{
    DVASSERT(fields1.size() == fields2.size());

    // write component header
    bstream.WriteBits(ObjectMarker::MARKER_CHANGED, 2);

    uint32 bitsPerId = CompressionUtils::CompressVarInt<uint16>(componentKey.id, bstream);
    bitsPerId += CompressionUtils::CompressVarInt<uint16>(componentKey.index, bstream);

    size_t nwritten = 0;
    const size_t nfields = fields1.size();
    for (size_t i = 0; i < nfields; ++i)
    {
        DVASSERT(fields1[i].compression == fields2[i].compression);
        DVASSERT(fields1[i].deltaPrecision == fields2[i].deltaPrecision);
        DVASSERT(fields1[i].privacy == fields2[i].privacy);

        if (CheckPrivacy(fields1[i].privacy))
        {
            const Any& value1 = fields1[i].value;
            const Any& value2 = fields2[i].value;
            const float32& deltaPrecision = fields1[i].deltaPrecision;
            const CompressionScheme& scheme = fields1[i].compression;

            DVASSERT(value1.GetType() == value2.GetType());

            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(value1);
            DVASSERT(compressor != nullptr, Format("Compressor is not registered for '%s'", value1.GetType()->GetName()).c_str());

            bstream.WriteBits(1, 1);
            if (compressor->CompressDelta(value1, value2, scheme, deltaPrecision, bstream))
            {
                nwritten += 1;
            }
            else
            {
                bstream.Rewind(1);
                bstream.WriteBits(0, 1);
            }
        }
        else
        {
            bstream.WriteBits(0, 1);
        }
    }

    if (nwritten == 0)
    {
        // revert component header
        bstream.Rewind(2 + bitsPerId + static_cast<uint32>(nfields));
    }
    return nwritten > 0;
}

bool EntitySnapshotWriter::WriteComponentFieldsFull(SnapshotComponentKey componentKey, const Vector<SnapshotField>& fields)
{
    // write component header
    bstream.WriteBits(ObjectMarker::MARKER_ADDED, 2);

    CompressionUtils::CompressVarInt<uint16>(componentKey.id, bstream);
    CompressionUtils::CompressVarInt<uint16>(componentKey.index, bstream);

    size_t i = 0;
    size_t nwritten = 0;
    for (const SnapshotField& field : fields)
    {
        if (CheckPrivacy(field.privacy))
        {
            bstream.WriteBits(1, 1);

            const Any& value = field.value;
            const float32& deltaPrecision = field.deltaPrecision;
            const CompressionScheme& scheme = field.compression;

            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(value);
            DVASSERT(compressor != nullptr, Format("Compressor is not registered for '%s'", value.GetType()->GetName()).c_str());

            compressor->CompressFull(value, scheme, deltaPrecision, bstream);
            nwritten += 1;
        }
        else
        {
            bstream.WriteBits(0, 1);
        }
        i += 1;
    }

    if (nwritten == 0 && !fields.empty())
    {
        // revert component header
        bstream.Rewind(static_cast<uint32>(fields.size()));
    }

    return true;
}

} // namespace DAVA
