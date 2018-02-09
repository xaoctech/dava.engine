#pragma once

#include "NetworkCore/Snapshot.h"

#include <Entity/SceneSystem.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
struct ReflectedComponentField;
class NetworkTimeSingleComponent;
class SnapshotSingleComponent;

class SnapshotSystemBase : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSystemBase, SceneSystem);

    SnapshotSystemBase(Scene* scene);
    ~SnapshotSystemBase();

    Snapshot snapshot;

protected:
    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void RegisterSingleComponent(Component* component) override;
    void UnregisterSingleComponent(Component* component) override;
    void PrepareForRemove() override;

    void UpdateSnapshot(NetworkID entityId = NetworkID::INVALID);
    void WatchAll(Entity* entity);
    void Watch(Entity* entity, Component* component);
    void UnwatchAll(Entity* entity);
    void Unwatch(Entity* entity, Component* component);

    [[deprecated]] Any ApplyQuantization(const Any& value, uint32 compression, float32 precision);
    [[deprecated]] uint32 GetComponentIndex(Entity* entity, Component* component);

    virtual bool NeedToBeTracked(Entity* entity);
    virtual bool NeedToBeTracked(Component* component);

    SnapshotSingleComponent* snapshotSingleComponent;
    const NetworkTimeSingleComponent* timeSingleComponent;

protected:
    enum class GetBranchPolicy
    {
        Get,
        Create
    };

    struct FieldWatchPoint
    {
        FieldWatchPoint(SnapshotField* snapshotField_, Entity* entity_, SnapshotComponentKey componentKey_, ReflectedObject refObject_, const ReflectedComponentField* refField_);

        bool Update();

        Entity* entity = nullptr;
        SnapshotField* snapshotField = nullptr;

        NetworkID entityId;
        SnapshotComponentKey componentKey;

        ReflectedObject refObject;
        const ReflectedComponentField* refField;

        Any cacheValue;

        size_t origSize = 0;
        void* origAddr = nullptr;
        bool isTrivial = false;
    };

    Vector<FieldWatchPoint> watchPoints;

    virtual SnapshotEntity* GetSnapshotEntity(Entity* entity, GetBranchPolicy policy);
};
} // namespace DAVA
