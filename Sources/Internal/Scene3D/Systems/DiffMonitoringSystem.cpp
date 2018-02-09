#include "Scene3D/Systems/DiffMonitoringSystem.h"

#ifdef DIFF_MONITORING_ENABLED

#include "Debug/ProfilerCPU.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"
#include "Math/Color.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"
#include <Scene3D/Scene.h>
#include "Time/SystemTimer.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DiffMonitoringSystem)
{
    ReflectionRegistrator<DiffMonitoringSystem>::Begin()
    .ConstructorByPointer<Scene*>()
    .Method("Process", &DiffMonitoringSystem::Process)
    .End();
}

namespace DiffMonitoringDetail
{
struct TrivialWatchLine : DiffMonitoringSystem::WatchLine
{
    struct TrivialWatchPoint
    {
        DiffMonitoringSystem::WatchPoint wp;
        void* origData;
        void* prevData;
        size_t size;

        void SetupPrevData()
        {
            prevData = const_cast<void*>(wp.fieldValue.GetData());
        }
    };

    bool CanWatch(const Reflection& field) const override
    {
        DVASSERT(field.IsValid());

        const Type* type = field.GetValueType()->Decay();
        void* valueAddr = field.GetValueObject().GetVoidPtr();
        size_t sz = type->GetSize();

        return (nullptr != valueAddr && type->IsTriviallyCopyable());
    }

    void AddWatchPoint(DiffMonitoringSystem::WatchPoint&& wp) override
    {
        void* data = wp.fieldRef.GetValueObject().GetVoidPtr();
        const Type* type = wp.fieldRef.GetValueType()->Decay();

        TrivialWatchPoint twp;
        twp.wp = std::move(wp);
        twp.size = type->GetSize();
        twp.origData = data;
        twp.prevData = nullptr;
        trivialWatchPoints.emplace_back(std::move(twp));
        trivialWatchPoints.back().SetupPrevData();

        isSortOpimized = false;
    }

    void RemoveWatchPoint(Component* component) override
    {
        auto it = trivialWatchPoints.begin();
        auto end = trivialWatchPoints.end();
        while (it != end)
        {
            if (it->wp.component == component)
            {
                diff.erase(&it->wp);
                it = trivialWatchPoints.erase(it);
            }
            else
            {
                it++;
            }
        }

        isSortOpimized = false;
    }

    void Process() override
    {
        DAVA_PROFILER_CPU_SCOPE("TrivialWatchLine::Process");

        /*
        if (!isSortOpimized)
        {
            std::sort(trivialWatchPoints.begin(), trivialWatchPoints.end(), [](const TrivialWatchPoint& w1, const TrivialWatchPoint& w2) {
                return w1.origData < w2.origData;
            });

            isSortOpimized = true;
        }
        */

        for (auto& twp : trivialWatchPoints)
        {
            if (0 != ::memcmp(twp.prevData, twp.origData, twp.size))
            {
                ::memcpy(twp.prevData, twp.origData, twp.size);
                diff.insert(&twp.wp);
            }
        }
    }

private:
    bool isSortOpimized = false;
    List<TrivialWatchPoint> trivialWatchPoints;
};

// fields without real memory addresses are in most
// cases fields that are registered with Setter|Getter, so
// they don't have correct valueObject, and so we are forced
// to monitor them with Any values, that is a bit slower
// than with real pointer got from valueObject
struct SetterGetterTrivialWatchLine : DiffMonitoringSystem::WatchLine
{
    struct AnyWatchPoint
    {
        DiffMonitoringSystem::WatchPoint wp;
        Any lastValue;
    };

    bool CanWatch(const Reflection& field) const override
    {
        DVASSERT(field.IsValid());

        const Type* type = field.GetValueType()->Decay();
        void* valueAddr = field.GetValueObject().GetVoidPtr();

        return (nullptr == valueAddr && type->IsTriviallyCopyable());
    }

    void AddWatchPoint(DiffMonitoringSystem::WatchPoint&& wp) override
    {
        Any val = wp.fieldRef.GetValue();
        watchPoints.emplace_back(std::move(wp));
    }

    void RemoveWatchPoint(Component* component) override
    {
        auto it = watchPoints.begin();
        auto end = watchPoints.end();
        while (it != end)
        {
            if (it->component == component)
            {
                diff.erase(&(*it));
                it = watchPoints.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void Process() override
    {
        DAVA_PROFILER_CPU_SCOPE("SetterGetterTrivialWatchLine::Process");

        for (auto& wp : watchPoints)
        {
            Any curValue = wp.fieldRef.GetValue();
            if (curValue != wp.fieldValue)
            {
                wp.fieldValue = std::move(curValue);
                diff.insert(&wp);
            }
        }
    }

private:
    List<DiffMonitoringSystem::WatchPoint> watchPoints;
};

} // namespace DiffMonitoringDetail

DiffMonitoringSystem::DiffMonitoringSystem(Scene* scene, UDPServer* server_, UDPClient* client_)
    : SceneSystem(scene)
    , server(server_)
    , client(client_)
{
    static bool registered = false;
    if (!registered)
    {
        uint16 code = 1;
        TypeCode::Register(Type::Instance<bool>(), code++);
        TypeCode::Register(Type::Instance<int8>(), code++);
        TypeCode::Register(Type::Instance<uint8>(), code++);
        TypeCode::Register(Type::Instance<int16>(), code++);
        TypeCode::Register(Type::Instance<uint16>(), code++);
        TypeCode::Register(Type::Instance<int32>(), code++);
        TypeCode::Register(Type::Instance<size_t>(), code++);
        TypeCode::Register(Type::Instance<uint32>(), code++);
        TypeCode::Register(Type::Instance<int64>(), code++);
        TypeCode::Register(Type::Instance<uint64>(), code++);
        TypeCode::Register(Type::Instance<float32>(), code++);
        TypeCode::Register(Type::Instance<float64>(), code++);
        TypeCode::Register(Type::Instance<Vector3>(), code++);
        TypeCode::Register(Type::Instance<Vector4>(), code++);
        TypeCode::Register(Type::Instance<Matrix3>(), code++);
        TypeCode::Register(Type::Instance<Matrix4>(), code++);
        TypeCode::Register(Type::Instance<Color>(), code++);
        TypeCode::Register(Type::Instance<Quaternion>(), code++);
        TypeCode::Register(Type::Instance<NetworkPackedQuaternion>(), code++);
        TypeCode::Register(Type::Instance<FastName>(), code++);
        TypeCode::Register(Type::Instance<FrameActionID>(), code++);
        TypeCode::Register(Type::Instance<NetworkID>(), code++);
        registered = true;
    }

    if (server)
    {
        AddWatchLine(std::make_unique<DiffMonitoringDetail::TrivialWatchLine>());
        AddWatchLine(std::make_unique<DiffMonitoringDetail::SetterGetterTrivialWatchLine>());

        for (auto& sc : scene->singletonComponents)
        {
            StartWatching(sc.first);
        }
    }

    if (client)
    {
        client->SubscribeOnReceive(PacketParams::REPLICATION_DIFF_CHANNEL_ID,
                                   MakeFunction(this, &DiffMonitoringSystem::OnReceiveClient));
    }
}

DiffMonitoringSystem::~DiffMonitoringSystem()
{
}

void DiffMonitoringSystem::Process(float32 timeElapsed)
{
    static std::array<uint8, 1024 * 1024 * 16> sendBuffer; // 16K buffer

    DAVA_PROFILER_CPU_SCOPE("DiffMonitoringSystem::Process");

    size_t diffSize = 0;
    if (server)
    {
        NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
        AnyBitIOStream anyStream(sendBuffer.data(), sendBuffer.size());

        for (auto& line : watchLines)
        {
            line->Process();
        }

        server->Foreach(
        [this, &anyStream, netGameModeComp](const Responder& responder)
        {
            DAVA_PROFILER_CPU_SCOPE("DiffMonitoringSystem::SaveDiff");

            anyStream.OpenForWrite();
            for (auto& line : watchLines)
            {
                if (!line->diff.empty())
                {
                    for (auto wp : line->diff)
                    {
                        M::Privacy privacy = M::Privacy::PUBLIC;
                        Entity* entity = wp->component->GetEntity();

                        if (nullptr != entity)
                        {
                            NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
                            NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(responder.GetToken());
                            if (playerID == netReplComp->GetNetworkPlayerID())
                            {
                                privacy = M::Privacy::PRIVATE;
                            }
                            else if (responder.GetTeamID() == netReplComp->GetOwnerTeamID())
                            {
                                privacy = M::Privacy::TEAM_ONLY;
                            }
                            else
                            {
                                privacy = M::Privacy::PUBLIC;
                            }
                        }

                        if (wp->fieldMeta.privacy >= privacy)
                        {
                            this->SaveSingleDiff(anyStream, wp);
                        }
                    }

                    line->diff.clear();
                }
            }

            size_t wrSize = anyStream.Tell();
            if (wrSize > 0)
            {
                anyStream.Flush();
                responder.Send(static_cast<uint8*>(anyStream.Data()), wrSize, PacketParams::Reliable(PacketParams::REPLICATION_DIFF_CHANNEL_ID));
            }
            anyStream.Close();
        }
        );
    }
}

void DiffMonitoringSystem::RegisterEntity(Entity* entity)
{
    SceneSystem::RegisterEntity(entity);

    if (server)
    {
        for (uint32_t i = 0; i < entity->GetComponentCount(); ++i)
        {
            Component* component = entity->GetComponentByIndex(i);
            StartWatching(component);
        }
    }
}

void DiffMonitoringSystem::UnregisterEntity(Entity* entity)
{
    SceneSystem::UnregisterEntity(entity);

    if (server)
    {
        for (uint32_t i = 0; i < entity->GetComponentCount(); ++i)
        {
            Component* component = entity->GetComponentByIndex(i);
            StopWatching(component);
        }
    }
}

void DiffMonitoringSystem::RegisterComponent(Entity* entity, Component* component)
{
    SceneSystem::RegisterComponent(entity, component);

    if (server)
    {
        StartWatching(component);
    }
}

void DiffMonitoringSystem::UnregisterComponent(Entity* entity, Component* component)
{
    SceneSystem::UnregisterComponent(entity, component);

    if (server)
    {
        StopWatching(component);
    }
}

void DiffMonitoringSystem::AddWatchLine(std::unique_ptr<WatchLine> watchLine)
{
    watchLines.emplace_back(std::move(watchLine));
}

void DiffMonitoringSystem::StartWatching(Component* component)
{
    const ReflectedType* compRefType = ReflectedTypeDB::GetByPointer(component);

    DVASSERT(nullptr != compRefType);
    if (nullptr == compRefType)
        return;

    const M::Replicable* compReplicableMeta = compRefType->GetMeta<M::Replicable>();
    if (nullptr != compReplicableMeta)
    {
        // single component
        if (nullptr == component->GetEntity())
        {
            if (compReplicableMeta->privacy <= M::Privacy::PRIVATE)
            {
                // don't watch for server private single components
                return;
            }
        }

        // we should track only components, that are
        // registered with M::Replicable meta
        Reflection compRef = Reflection::Create(ReflectedObject(component));
        Vector<Reflection::Field> fields = compRef.GetFields();

        for (auto& field : fields)
        {
            const M::Replicable* fieldReplicableMeta = field.ref.GetMeta<M::Replicable>();

            // we should track only fields, that are
            // registered with M::Replicable meta
            if (nullptr != fieldReplicableMeta)
            {
                M::Replicable fieldReplicableMetaCopy = *fieldReplicableMeta;
                fieldReplicableMetaCopy.ApplyParent(compReplicableMeta);

                Vector<Any> fieldRoot;
                StartFieldWatching(component, field.ref, field.key, fieldRoot, fieldReplicableMetaCopy);
            }
        }
    }
}

void DiffMonitoringSystem::StopWatching(Component* component)
{
    for (auto& line : watchLines)
    {
        line->RemoveWatchPoint(component);
    }
}

void DiffMonitoringSystem::StartFieldWatching(Component* component, Reflection fieldRef, Any fieldKey, Vector<Any> rootKey, const M::Replicable& fieldMeta)
{
    bool addedToWatch = false;

    DVASSERT(!fieldRef.GetValueType()->IsPointer());

    // search for watchLine with appropriate type
    // to be able to watch for given fieldRef
    for (auto& watchLine : watchLines)
    {
        if (watchLine->CanWatch(fieldRef) && !fieldRef.GetFieldsCaps().isContainer)
        {
            watchLine->AddWatchPoint(WatchPoint(component, fieldRef, fieldMeta, fieldKey, rootKey));
            addedToWatch = true;
            break;
        }
    }

    // if field wasn't added to watch we will try to iterate its
    // children and add them to watch
    if (!addedToWatch)
    {
        if (fieldRef.HasFields() || fieldRef.GetFieldsCaps().hasDynamicStruct)
        {
            if (fieldRef.GetFieldsCaps().hasDynamicStruct)
            {
                DVASSERT(false && "Can't watch for fields with dynamic structure");
            }
            else
            {
                Vector<Any> subFieldRoot(rootKey);
                subFieldRoot.push_back(fieldKey);

                Vector<Reflection::Field> subfields = fieldRef.GetFields();
                for (auto& subfield : subfields)
                {
                    StartFieldWatching(component, subfield.ref, subfield.key, subFieldRoot, fieldMeta); // <- use the same meta as parent one
                }
            }
        }
        else
        {
            DVASSERT(false && "Can't watch for field with unknown type");
        }
    }
}

void DiffMonitoringSystem::OnReceiveClient(const uint8* data, size_t size, uint8, uint32)
{
    AnyBitIOStream anyStream(const_cast<uint8*>(data), size);
    anyStream.OpenForRead();

    while (!anyStream.Eof())
    {
        LoadSingleDiff(anyStream);
    }

    anyStream.Close();
}

void DiffMonitoringSystem::SaveSingleDiff(AnyBitIOStream& wrStream, const WatchPoint* wp)
{
    Component* component = wp->component;
    Entity* entity = component->GetEntity();

    NetworkID entityID = NetworkID::INVALID;
    uint32 componentID = component->GetType();

    if (entity != nullptr)
    {
        NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
        entityID = netReplComp->GetNetworkUniqueID();

        DVASSERT(entityID != NetworkID::INVALID);
    }

    wrStream.Write(&entityID, Type::Instance<NetworkID>());
    wrStream.Write(&componentID, Type::Instance<uint32>());

    uint16 pathLen = wp->rootKey.size();
    wrStream.Write(&pathLen, Type::Instance<uint16>());
    for (size_t i = 0; i < wp->rootKey.size(); ++i)
    {
        wrStream.Write(wp->rootKey[i]);
    }

    wrStream.Write(wp->fieldKey);
    wrStream.Write(wp->fieldValue);
}

void DiffMonitoringSystem::LoadSingleDiff(AnyBitIOStream& rdStream)
{
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();

    Any entityId = rdStream.Read();
    Any componentId = rdStream.Read();

    Component* component = nullptr;
    const Type* compType = GetEngineContext()->componentManager->GetSceneTypeFromRuntimeType(componentId.Get<uint32>());

    if (entityId.Get<NetworkID>() != NetworkID::INVALID)
    {
        Entity* entity = networkEntities->FindByID(entityId.Get<NetworkID>());
        if (entity)
        {
            component = entity->GetComponent(componentId.Get<uint32>(), 0);
        }
    }
    else
    {
        component = GetScene()->GetSingletonComponent(compType);
    }

    Any rootPathLen = rdStream.Read();
    size_t pathLen = rootPathLen.Get<uint16>() + 1;

    if (nullptr != component)
    {
        Reflection ref = Reflection::Create(ReflectedObject(component));
        for (size_t i = 0; i < pathLen; ++i)
        {
            Any pathPart = rdStream.Read();
            ref = ref.GetField(pathPart);

            DVASSERT(ref.IsValid());
        }

        Any value = rdStream.Read();
        ref.SetValue(value);
    }
    else
    {
        Logger::Warning("Skiping diff for Entity %hu, Component %s", entityId.Get<NetworkID>().networkID, compType->GetName());

        // just skip this
        for (size_t i = 0; i < pathLen; ++i) // 1. skip path
            rdStream.Read();

        rdStream.Read(); // 2. skip value
    }
}

} // namespace DAVA

#endif // DIFF_MONITORING_ENABLED
