#pragma once

#include "REPlatform/DataNodes/Selectable.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Scene/SceneTypes.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Base/UnordererSet.h>
#include <Debug/DVAssert.h>
#include <Entity/SceneSystem.h>
#include <Math/Matrix4.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/RenderHelper.h>
#include <UI/UIEvent.h>

namespace physx
{
class PxRigidActor;
class PxScene;
} // namespace physx

namespace DAVA
{
class RECommandNotificationObject;
class Entity;
class PhysicsGeometryCache;
class RECommandNotificationObject;

class QueryObjectDataComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

    uint32 GetData() const;
    void SetData(uint32 newData);

private:
    uint32 data = 0;

    DAVA_VIRTUAL_REFLECTION(QueryObjectDataComponent, Component);
};

void* CreateQueryDataComponent();
;

class SceneCollisionSystem : public SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;
    friend class EntityModificationSystem;

public:
    SceneCollisionSystem(Scene* scene);
    ~SceneCollisionSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    void ObjectsRayTest(const Vector3& from, const Vector3& to, SelectableGroup::CollectionType& result) const;
    void ObjectsRayTestFromCamera(SelectableGroup::CollectionType& result) const;

    void OverlapSphereTest(const DAVA::Vector3& position, DAVA::float32 sphereRadius, SelectableGroup::CollectionType& result, DAVA::uint32 addFilterData = 0) const;
    bool OverlapSphereTest(const DAVA::Vector3& position, DAVA::float32 sphereRadius, DAVA::uint32 addFilterData = 0) const;

    bool LandRayTest(const Vector3& from, const Vector3& to, Vector3& intersectionPoint) const;
    bool LandRayTestFromCamera(Vector3& intersectionPoint) const;

    void ClipObjectsToPlanes(const Vector<Plane>& planes, SelectableGroup::CollectionType& result);

    AABBox3 GetBoundingBox(const Any& object) const;
    AABBox3 GetUntransformedBoundingBox(const Any& entity) const;
    void UpdateCollisionObject(const Selectable& object, bool forceRecreate = false);

    Landscape* GetCurrentLandscape() const;
    void EnableSystem() override;

    void UpdateActor(QueryObjectDataComponent* component);

protected:
    void SetScene(Scene* scene) override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    AABBox3 GetTransformedBoundingBox(const Selectable& object, const Matrix4& transform) const;

    void ImmediateEvent(Component* component, uint32 event) override;

    using TCallBack = Function<void(const Any&, physx::PxRigidActor*, bool shouldBeRecreated)>;
    void EnumerateObjectHierarchy(const Selectable& object, bool createCollision, const TCallBack& callback);

    Vector2 lastMousePos;
    Entity* curLandscapeEntity = nullptr;

    physx::PxScene* physicsScene = nullptr;
    PhysicsGeometryCache* geometryCache = nullptr;

    UnorderedSet<Any> objectsToAdd;
    UnorderedSet<Any> objectsToRemove;
    UnorderedSet<Any> objectsToUpdateTransform;
    UnorderedMap<Any, physx::PxRigidActor*> objToPhysx;
};
} // namespace DAVA