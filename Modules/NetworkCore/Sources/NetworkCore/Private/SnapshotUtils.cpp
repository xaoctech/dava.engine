#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/SnapshotStat.h"
#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Private/EntitySnapshotReader.h"
#include "NetworkCore/Private/EntitySnapshotWriter.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Debug/DVAssert.h>
#include <Entity/ComponentUtils.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
const Vector<ReflectedComponentField>& SnapshotUtils::GetComponentFields(Reflection componentRef)
{
    static UnorderedMap<const ReflectedType*, Vector<ReflectedComponentField>> cache;

    ReflectedObject compObj = componentRef.GetDirectObject();
    const ReflectedType* refType = compObj.GetReflectedType();

    auto it = cache.find(refType);
    if (it != cache.end())
    {
        return it->second;
    }
    else
    {
        auto fields = componentRef.GetFields([](const ReflectedMeta* meta) {
            if (nullptr != meta)
            {
                if (nullptr != meta->GetMeta<M::Replicable>())
                {
                    return true;
                }
            }
            return false;
        });

        Vector<ReflectedComponentField>& ret = cache[refType];

        for (size_t i = 0; i < fields.size(); ++i)
        {
            const M::Replicable* replicable = fields[i].ref.GetMeta<M::Replicable>();

            DVASSERT(nullptr != replicable);
            DVASSERT(fields[i].ref.GetDirectObject() == compObj);

            const Type* valueType = fields[i].ref.GetValueType()->Decay();
            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(valueType);
            DVASSERT(compressor != nullptr);

            ReflectedComponentField f;
            f.key = fields[i].key;
            f.replicable = replicable;
            f.precision = compressor->GetComparePrecision(fields[i].ref.meta);
#if defined(COMPRESSION_DISABLED)
            f.deltaPrecision = defaultDeltaPrecision;
            f.compressionScheme = 0;
#else
            f.deltaPrecision = compressor->GetDeltaPrecision(fields[i].ref.meta);
            f.compressionScheme = compressor->GetCompressionScheme(fields[i].ref.meta);
#endif
            f.valueWrapper = fields[i].ref.valueWrapper;

            ret.push_back(std::move(f));
        }

        return ret;
    }
}

bool SnapshotUtils::ApplySnapshot(Snapshot* snapshot, NetworkID entityId, Entity* dstEntity, SnapshotApplyPredicate pred)
{
    DVASSERT(nullptr != snapshot);
    DVASSERT(nullptr != dstEntity);

    SnapshotEntity* se = snapshot->FindEntity(entityId);
    if (nullptr == se)
    {
        return false;
    }

    Vector<Component*> componentsToRemove;

    // Add and update components from snapshot.
    for (auto& c : se->components)
    {
        SnapshotComponentKey componentKey = c.first;
        SnapshotComponent* sc = &c.second;

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "| Component " << componentKey.id << "-" << componentKey.index << ":\n");

        Component* dstComponent = nullptr;
        const Type* dstComponentType = ComponentUtils::GetType(componentKey.id);

        // single component
        if (entityId == NetworkID::SCENE_ID)
        {
            DVASSERT(componentKey.index == 0);
            dstComponent = dstEntity->GetScene()->GetSingletonComponent(dstComponentType);
        }
        // regular component
        else
        {
            dstComponent = dstEntity->GetOrCreateComponent(dstComponentType, componentKey.index);
        }

        // check if there is predicate and ask it about dstComponent
        bool allowApply = true;
        if (nullptr != pred)
        {
            allowApply = pred(dstComponent);
        }

        // now apply component snapshot
        if (allowApply)
        {
            ApplySnapshot(sc, dstComponent);
        }
    }

    return true;
}

bool SnapshotUtils::ApplySnapshot(Snapshot* snapshot, NetworkID entityId, SnapshotComponentKey componentKey, Component* dstComponent)
{
    DVASSERT(nullptr != snapshot);
    DVASSERT(nullptr != dstComponent);

    SnapshotEntity* se = snapshot->FindEntity(entityId);
    if (nullptr == se)
    {
        return false;
    }

    auto it = se->components.find(componentKey);
    if (it == se->components.end())
    {
        return false;
    }

    SnapshotComponent* sc = &it->second;
    ApplySnapshot(sc, dstComponent);

    return true;
}

void SnapshotUtils::ApplySnapshot(SnapshotComponent* snapshotComponent, Component* dstComponent)
{
    DVASSERT(nullptr != snapshotComponent);
    DVASSERT(nullptr != dstComponent);

    size_t fieldsCount = snapshotComponent->fields.size();

    ReflectedObject compObj(dstComponent);
    Reflection compRef = Reflection::Create(compObj);

    const Vector<ReflectedComponentField>& fields = GetComponentFields(compRef);
    DVASSERT(fieldsCount == fields.size());

    for (size_t i = 0; i < fieldsCount; ++i)
    {
        fields[i].valueWrapper->SetValue(compObj, snapshotComponent->fields[i].value);
        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- " << fields[i].key << " = " << snapshotComponent->fields[i].value << "\n");
    }
}

bool SnapshotUtils::TestSnapshotComponentsEqual(const SnapshotComponent* snapshotComponent1, const SnapshotComponent* snapshotComponent2)
{
    DVASSERT(snapshotComponent1 != nullptr && snapshotComponent2 != nullptr);
    DVASSERT(snapshotComponent1->fields.size() == snapshotComponent2->fields.size());

    const size_t nfields = snapshotComponent1->fields.size();
    for (size_t i = 0; i < nfields; ++i)
    {
        const Any& any1 = snapshotComponent1->fields[i].value;
        const Any& any2 = snapshotComponent2->fields[i].value;
        DVASSERT(any1.GetType() == any2.GetType());

        float32 precision = snapshotComponent1->fields[i].precision;
        DVASSERT(precision == snapshotComponent2->fields[i].precision);

        const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(any1);
        DVASSERT(compressor != nullptr);

        if (!compressor->IsEqual(any1, any2, precision))
        {
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Field " << i << " isn't equal " << any1 << " != " << any2 << "\n");
            return false;
        }
    }
    return true;
}

size_t SnapshotUtils::CreateSnapshotDiff(const Snapshot* base, Snapshot* current, NetworkID entityId, M::Privacy privacy, uint8* dstBuff, size_t dstSize)
{
    DVASSERT(current != nullptr);

    BitWriter bitStream(dstBuff, dstSize);
    EntitySnapshotWriter writer(bitStream, entityId, privacy, base, current);
    size_t n = writer.Write();
    return n;
}

size_t SnapshotUtils::ApplySnapshotDiff(const Snapshot* base, Snapshot* target, NetworkID entityId, const uint8* srcBuff, size_t srcSize, SnapshotApplyCallback callback)
{
    DVASSERT(target != nullptr);

#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    if (clientSnapshotStat == nullptr)
    {
        clientSnapshotStat = new SnapshotStat;
    }
#endif

    BitReader bitStream(srcBuff, srcSize);
    EntitySnapshotReader reader(bitStream, entityId, base, target, callback);
    size_t n = reader.Read();

#if defined(COLLECT_CLIENT_SNAPSHOT_STAT)
    clientSnapshotStat->size += static_cast<uint32>(n);
    clientSnapshotStat->PeriodicDump();
#endif
    return n;
}

size_t SnapshotUtils::GetSnapshotDiffSize(const uint8* srcBuff, size_t srcSize)
{
    BitReader bitStream(srcBuff, srcSize);
    EntitySnapshotReader reader(bitStream);
    size_t n = reader.GetSize();
    return n;
}

std::ostream& SnapshotUtils::Log()
{
    static std::ofstream log("Snapshot.log");
    return log;
}

} // namespace DAVA
