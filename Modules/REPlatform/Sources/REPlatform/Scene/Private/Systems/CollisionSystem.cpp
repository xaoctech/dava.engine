#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionRenderObject.h"
#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionLandscape.h"
#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBox.h"
#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBaseObject.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/Commands/ConvertToBillboardCommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/EntityParentChangeCommand.h"
#include "REPlatform/Commands/HeightmapEditorCommands2.h"
#include "REPlatform/Commands/InspMemberModifyCommand.h"
#include "REPlatform/Commands/ParticleEditorCommands.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Commands/TransformCommand.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <TArc/Core/Deprecated.h>

#include <Base/AlignedAllocator.h>
#include <Functional/Function.h>
#include <Math/AABBox3.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/GeoDecalComponent.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/SlotSystem.h>

#define SIMPLE_COLLISION_BOX_SIZE 1.0f

namespace DAVA
{
SceneCollisionSystem::SceneCollisionSystem(Scene* scene)
    : SceneSystem(scene)
{
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax(1000, 1000, 1000);

    objectsCollConf = new btDefaultCollisionConfiguration();
    objectsCollDisp = CreateObjectAligned<btCollisionDispatcher, 16>(objectsCollConf);
    objectsBroadphase = new btAxisSweep3(worldMin, worldMax);
    objectsDebugDrawer = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    objectsDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    objectsCollWorld = new btCollisionWorld(objectsCollDisp, objectsBroadphase, objectsCollConf);
    objectsCollWorld->setDebugDrawer(objectsDebugDrawer);

    landCollConf = new btDefaultCollisionConfiguration();
    landCollDisp = CreateObjectAligned<btCollisionDispatcher, 16>(landCollConf);
    landBroadphase = new btAxisSweep3(worldMin, worldMax);
    landDebugDrawer = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    landDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    landCollWorld = new btCollisionWorld(landCollDisp, landBroadphase, landCollConf);
    landCollWorld->setDebugDrawer(landDebugDrawer);

    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::GEO_DECAL_CHANGED);
}

SceneCollisionSystem::~SceneCollisionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::GEO_DECAL_CHANGED);
    }

    for (const auto& etc : objectToCollision)
    {
        delete etc.second;
    }
    objectToCollision.clear();

    SafeDelete(objectsCollWorld);
    SafeDelete(objectsBroadphase);
    SafeDelete(objectsDebugDrawer);
    SafeDelete(objectsCollConf);

    SafeDelete(landCollWorld);
    SafeDelete(landDebugDrawer);
    SafeDelete(landBroadphase);
    SafeDelete(landCollConf);

    DestroyObjectAligned(objectsCollDisp);
    DestroyObjectAligned(landCollDisp);
}

const SelectableGroup::CollectionType& SceneCollisionSystem::ObjectsRayTest(const Vector3& from, const Vector3& to)
{
    // check if cache is available
    if (rayIntersectCached && (lastRayFrom == from) && (lastRayTo == to))
    {
        return rayIntersectedEntities;
    }

    // no cache. start ray new ray test
    lastRayFrom = from;
    lastRayTo = to;
    rayIntersectedEntities.clear();

    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
    objectsCollWorld->rayTest(btFrom, btTo, btCallback);

    int collidedObjects = btCallback.m_collisionObjects.size();
    if (btCallback.hasHit() && (collidedObjects > 0))
    {
        // TODO: sort values inside btCallback, instead of creating separate vector for them
        using Hits = Vector<std::pair<btCollisionObject*, btScalar>>;
        Hits hits(collidedObjects);
        btCallback.m_collisionObjects.size();
        for (int i = 0; i < collidedObjects; ++i)
        {
            hits[i].first = btCallback.m_collisionObjects[i];
            hits[i].second = btCallback.m_hitFractions[i];
        }
        std::sort(hits.begin(), hits.end(), [&btFrom](const Hits::value_type& l, const Hits::value_type& r) {
            return l.second < r.second;
        });

        for (const auto& hit : hits)
        {
            auto entity = collisionToObject[hit.first];
            rayIntersectedEntities.emplace_back(entity);
            AABBox3 bbox = GetBoundingBox(entity);
            if (!bbox.IsEmpty())
            {
                rayIntersectedEntities.back().SetBoundingBox(bbox);
            }
        }
    }

    rayIntersectCached = true;
    return rayIntersectedEntities;
}

const SelectableGroup::CollectionType& SceneCollisionSystem::ObjectsRayTestFromCamera()
{
    Vector3 traceFrom;
    Vector3 traceTo;

    SceneCameraSystem* cameraSystem = GetScene()->GetSystem<SceneCameraSystem>();
    cameraSystem->GetRayTo2dPoint(lastMousePos, 1000.0f, traceFrom, traceTo);

    return ObjectsRayTest(traceFrom, traceTo);
}

bool SceneCollisionSystem::LandRayTest(const Vector3& from, const Vector3& to, Vector3& intersectionPoint)
{
    Vector3 ret;

    // check if cache is available
    if (landIntersectCached && lastLandRayFrom == from && lastLandRayTo == to)
    {
        intersectionPoint = lastLandCollision;
        return landIntersectCachedResult;
    }

    // no cache. start new ray test
    lastLandRayFrom = from;
    lastLandRayTo = to;
    landIntersectCached = true;
    landIntersectCachedResult = false;

    Vector3 rayDirection = to - from;
    float32 rayLength = rayDirection.Length();
    rayDirection.Normalize();
    rayDirection *= Min(5.0f, rayLength);
    float stepSize = rayDirection.Length();

    btVector3 btStep(rayDirection.x, rayDirection.y, rayDirection.z);
    btVector3 btFrom(from.x, from.y, from.z);
    while ((rayLength - stepSize) >= std::numeric_limits<float>::epsilon())
    {
        btVector3 btTo = btFrom + btStep;

        btCollisionWorld::ClosestRayResultCallback btCallback(btFrom, btTo);
        landCollWorld->rayTest(btFrom, btTo, btCallback);

        if (btCallback.hasHit())
        {
            btVector3 hitPoint = btCallback.m_hitPointWorld;
            ret = Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z());
            landIntersectCachedResult = true;
            break;
        }

        btFrom = btTo;
        rayLength -= stepSize;
    }

    lastLandCollision = ret;
    intersectionPoint = ret;

    return landIntersectCachedResult;
}

bool SceneCollisionSystem::LandRayTestFromCamera(Vector3& intersectionPoint)
{
    SceneCameraSystem* cameraSystem = GetScene()->GetSystem<SceneCameraSystem>();

    Vector3 camPos = cameraSystem->GetCameraPosition();
    Vector3 camDir = cameraSystem->GetPointDirection(lastMousePos);

    Vector3 traceFrom = camPos;
    Vector3 traceTo = traceFrom + camDir * 1000.0f;

    return LandRayTest(traceFrom, traceTo, intersectionPoint);
}

Landscape* SceneCollisionSystem::GetCurrentLandscape() const
{
    return GetLandscape(curLandscapeEntity);
}

void SceneCollisionSystem::UpdateCollisionObject(const Selectable& object)
{
    if (object.CanBeCastedTo<Entity>())
    {
        auto entity = object.AsEntity();
        RemoveEntity(entity);
        AddEntity(entity);
    }
    else
    {
        objectsToRemove.insert(object.GetContainedObject());
        objectsToAdd.insert(object.GetContainedObject());
    }
}

AABBox3 SceneCollisionSystem::GetBoundingBox(const Any& object) const
{
    DVASSERT(object.IsEmpty() == false);

    AABBox3 aabox;
    if (objectToCollision.count(object) != 0)
    {
        CollisionBaseObject* collObj = objectToCollision.at(object);
        if (collObj != nullptr)
        {
            aabox = collObj->object.GetBoundingBox();

            Selectable wrapper(object);
            if (wrapper.CanBeCastedTo<Entity>())
            {
                auto entity = wrapper.AsEntity();
                for (int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
                {
                    aabox.AddAABBox(GetBoundingBox(entity->GetChild(i)));
                }
            }
        }
    }

    return aabox;
}

void SceneCollisionSystem::AddCollisionObject(const Any& obj, CollisionBaseObject* collision)
{
    if (collision == nullptr)
        return;

    DVASSERT(obj.IsEmpty() == false);
    if (objectToCollision.count(obj) > 0)
    {
        DestroyFromObject(obj);
    }
    objectToCollision[obj] = collision;
    collisionToObject[collision->btObject] = obj;
}

void SceneCollisionSystem::Process(float32 timeElapsed)
{
    if (!systemIsEnabled)
    {
        return;
    }

    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (Entity* entity : tsc->localTransformChanged)
    {
        UpdateCollisionObject(Selectable(entity));
    }
    for (Entity* entity : tsc->transformParentChanged)
    {
        UpdateCollisionObject(Selectable(entity));
    }

    // check in there are entities that should be added or removed
    if (!(objectsToAdd.empty() && objectsToRemove.empty()))
    {
        for (auto obj : objectsToRemove)
        {
            Selectable wrapper(obj);
            EnumerateObjectHierarchy(wrapper, false, [this](const Any& object, CollisionBaseObject* collision)
                                     {
                                         DestroyFromObject(object);
                                     });
        }

        for (auto obj : objectsToAdd)
        {
            Selectable wrapper(obj);
            if (wrapper.CanBeCastedTo<Entity>() || wrapper.SupportsTransformType(Selectable::TransformType::Disabled))
            {
                EnumerateObjectHierarchy(wrapper, true, MakeFunction(this, &SceneCollisionSystem::AddCollisionObject));
            }
        }

        objectsToAdd.clear();
        objectsToRemove.clear();
    }

    // reset ray cache on new frame
    rayIntersectCached = false;
}

bool SceneCollisionSystem::Input(UIEvent* event)
{
    // don't have to update last mouse pos when event is not from the mouse
    if (eInputDevices::MOUSE == event->device)
    {
        lastMousePos = event->point;
    }
    return false;
}

void SceneCollisionSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<ModifyHeightmapCommand>() == true)
    {
        UpdateCollisionObject(Selectable(curLandscapeEntity));
    }

    commandNotification.ForEach<CreatePlaneLODCommand>([&](const CreatePlaneLODCommand* cmd) {
        UpdateCollisionObject(Selectable(cmd->GetEntity()));
    });

    commandNotification.ForEach<SetFieldValueCommand>([&](const SetFieldValueCommand* cmd) {
        const FastName HEIGHTMAP_PATH("heightmapPath");
        const Reflection::Field& field = cmd->GetField();
        if (field.key.Cast<FastName>(FastName("")) == HEIGHTMAP_PATH)
        {
            UpdateCollisionObject(Selectable(curLandscapeEntity));
        }
    });

    commandNotification.ForEach<DeleteLODCommand>([&](const DeleteLODCommand* cmd) {
        UpdateCollisionObject(Selectable(cmd->GetEntity()));
    });

    commandNotification.ForEach<CommandRemoveParticleEmitter>([&](const CommandRemoveParticleEmitter* cmd) {
        (commandNotification.IsRedo() ? objectsToRemove : objectsToAdd).insert(cmd->GetEmitterInstance());
    });

    commandNotification.ForEach<TransformCommand>([&](const TransformCommand* cmd) {
        UpdateCollisionObject(cmd->GetTransformedObject());
    });

    commandNotification.ForEach<ConvertToBillboardCommand>([&](const ConvertToBillboardCommand* cmd) {
        UpdateCollisionObject(Selectable(cmd->GetEntity()));
    });
}

void SceneCollisionSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (!systemIsEnabled)
    {
        return;
    }

    switch (event)
    {
    case EventSystem::SWITCH_CHANGED:
    {
        UpdateCollisionObject(Selectable(component->GetEntity()));
        break;
    }
    case EventSystem::GEO_DECAL_CHANGED:
    {
        UpdateCollisionObject(Selectable(component->GetEntity()));
        break;
    }
    default:
        break;
    }
}

void SceneCollisionSystem::AddEntity(Entity* entity)
{
    if (!systemIsEnabled || entity == nullptr)
        return;

    if (GetLandscape(entity) != nullptr)
    {
        curLandscapeEntity = entity;
    }

    objectsToRemove.erase(entity);
    objectsToAdd.insert(entity);

    // build collision object for entity childs
    for (int i = 0; i < entity->GetChildrenCount(); ++i)
    {
        AddEntity(entity->GetChild(i));
    }
}

void SceneCollisionSystem::RemoveEntity(Entity* entity)
{
    if (!systemIsEnabled || entity == nullptr)
        return;

    if (curLandscapeEntity == entity)
    {
        curLandscapeEntity = nullptr;
    }

    objectsToAdd.erase(entity);
    objectsToRemove.insert(entity);

    // destroy collision object for entities childs
    for (int i = 0; i < entity->GetChildrenCount(); ++i)
    {
        RemoveEntity(entity->GetChild(i));
    }
}

void SceneCollisionSystem::PrepareForRemove()
{
    objectsToAdd.clear();
    objectsToRemove.clear();
    objectToCollision.clear();
}

void SceneCollisionSystem::DestroyFromObject(const Any& entity)
{
    CollisionBaseObject* cObj = objectToCollision[entity];
    if (cObj != nullptr)
    {
        objectToCollision.erase(entity);
        collisionToObject.erase(cObj->btObject);
        delete cObj;
    }
}

const SelectableGroup& SceneCollisionSystem::ClipObjectsToPlanes(const Vector<Plane>& planes)
{
    planeClippedObjects.Clear();
    for (const auto& object : objectToCollision)
    {
        if ((object.first.IsEmpty() == false) && (object.second != nullptr) &&
            (object.second->ClassifyToPlanes(planes) == CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects))
        {
            planeClippedObjects.Add(object.first, AABBox3());
        }
    }

    return planeClippedObjects;
}

void SceneCollisionSystem::EnableSystem()
{
    EditorSceneSystem::EnableSystem();
    AddEntity(GetScene());
}

namespace CollisionDetails
{
struct CollisionObj
{
    CollisionBaseObject* collisionObject = nullptr;
    bool isValid = false;
};

template <typename T, class... Args>
CollisionObj InitCollision(bool createCollision, Args... args)
{
    CollisionObj result;
    result.isValid = true;
    if (createCollision)
    {
        result.collisionObject = new T(std::forward<Args>(args)...);
    }
    return result;
}
}

void SceneCollisionSystem::EnumerateObjectHierarchy(const Selectable& object, bool createCollision, const TCallBack& callback)
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    float32 debugBoxScale = SIMPLE_COLLISION_BOX_SIZE * settings->debugBoxScale;
    float32 debugBoxParticleScale = SIMPLE_COLLISION_BOX_SIZE * settings->debugBoxParticleScale;
    if (object.CanBeCastedTo<Entity>())
    {
        CollisionDetails::CollisionObj result;
        Entity* entity = object.AsEntity();

        if (entity != nullptr && entity->GetParent() != nullptr)
        {
            if (GetScene()->slotSystem->LookUpSlot(entity->GetParent()) != nullptr)
                return;
        }

        float32 debugBoxUserScale = SIMPLE_COLLISION_BOX_SIZE * settings->debugBoxUserScale;
        float32 debugBoxWaypointScale = SIMPLE_COLLISION_BOX_SIZE * settings->debugBoxWaypointScale;

        Landscape* landscape = GetLandscape(entity);
        if (landscape != nullptr)
        {
            result = CollisionDetails::InitCollision<CollisionLandscape>(createCollision, entity, landCollWorld, landscape);
        }

        ParticleEffectComponent* particleEffect = GetEffectComponent(entity);
        if ((result.isValid == false) && (particleEffect != nullptr))
        {
            for (int32 i = 0, e = particleEffect->GetEmittersCount(); i < e; ++i)
            {
                EnumerateObjectHierarchy(Selectable(particleEffect->GetEmitterInstance(i)), createCollision, callback);
            }
            result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxParticleScale);
        }

        GeoDecalComponent* geoDecalComponent = GetGeoDecalComponent(entity);
        if ((result.isValid == false) && (geoDecalComponent != nullptr))
        {
            result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), geoDecalComponent->GetDimensions());
        }

        RenderObject* renderObject = GetRenderObject(entity);
        if ((result.isValid == false) && (renderObject != nullptr))
        {
            RenderObject::eType objType = renderObject->GetType();
            if (objType == RenderObject::TYPE_BILLBOARD)
            {
                const AABBox3& box = renderObject->GetBoundingBox();
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, box.GetCenter(), box.GetSize().x);
            }
            else if ((objType != RenderObject::TYPE_SPRITE) && (objType != RenderObject::TYPE_VEGETATION))
            {
                result = CollisionDetails::InitCollision<CollisionRenderObject>(createCollision, entity, objectsCollWorld, renderObject);
            }
        }

        Camera* camera = GetCamera(entity);
        if ((result.isValid == false) && (camera != nullptr))
        {
            result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, camera->GetPosition(), debugBoxScale);
        }

        // build simple collision box for all other entities, that has more than two components
        if ((result.isValid == false) && (entity != nullptr))
        {
            if ((entity->GetComponent(Component::SOUND_COMPONENT) != nullptr) ||
                (entity->GetComponent(Component::LIGHT_COMPONENT) != nullptr) ||
                (entity->GetComponent(Component::TEXT_COMPONENT) != nullptr) ||
                (entity->GetComponent(Component::WIND_COMPONENT) != nullptr))
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxScale);
            }
            else if (entity->GetComponent(Component::USER_COMPONENT) != nullptr)
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxUserScale);
            }
            else if (GetWaypointComponent(entity) != nullptr)
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxWaypointScale);
            }
            else
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxScale);
            }
        }

        DVASSERT(result.isValid == true);
        callback(entity, result.collisionObject);
    }
    else
    {
        float32 scale = object.CanBeCastedTo<ParticleEmitterInstance>() ? debugBoxParticleScale : debugBoxScale;
        const Any& containedObject = object.GetContainedObject();
        CollisionDetails::CollisionObj result = CollisionDetails::InitCollision<CollisionBox>(createCollision, containedObject, objectsCollWorld, object.GetWorldTransform().GetTranslationVector(), scale);
        callback(containedObject, result.collisionObject);
    }
}

AABBox3 SceneCollisionSystem::GetUntransformedBoundingBox(const Any& entity) const
{
    return GetTransformedBoundingBox(Selectable(entity), Matrix4::IDENTITY);
}

AABBox3 SceneCollisionSystem::GetTransformedBoundingBox(const Selectable& object, const Matrix4& transform) const
{
    AABBox3 entityBox = GetBoundingBox(object.GetContainedObject());
    if (object.CanBeCastedTo<Entity>())
    {
        // add childs boxes into entity box
        Entity* entity = object.AsEntity();
        for (int32 i = 0; i < entity->GetChildrenCount(); i++)
        {
            Selectable childEntity(entity->GetChild(i));
            AABBox3 childBox = GetTransformedBoundingBox(childEntity, childEntity.GetLocalTransform());
            if (childBox.IsEmpty() == false)
            {
                if (entityBox.IsEmpty())
                {
                    entityBox = childBox;
                }
                else
                {
                    entityBox.AddAABBox(childBox);
                }
            }
        }
    }

    AABBox3 ret;
    if (entityBox.IsEmpty() == false)
    {
        entityBox.GetTransformedBox(transform, ret);
    }
    return ret;
}

// -----------------------------------------------------------------------------------------------
// debug draw
// -----------------------------------------------------------------------------------------------

SceneCollisionDebugDrawer::SceneCollisionDebugDrawer(RenderHelper* _drawer)
    : dbgMode(0)
    , drawer(_drawer)
{
}

SceneCollisionDebugDrawer::~SceneCollisionDebugDrawer()
{
}

void SceneCollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    Vector3 davaFrom(from.x(), from.y(), from.z());
    Vector3 davaTo(to.x(), to.y(), to.z());
    Color davaColor(color.x(), color.y(), color.z(), 1.0f);

    drawer->DrawLine(davaFrom, davaTo, davaColor, RenderHelper::DRAW_WIRE_DEPTH);
}

void SceneCollisionDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    Color davaColor(color.x(), color.y(), color.z(), 1.0f);
    drawer->DrawIcosahedron(Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()), distance / 20.f, davaColor, RenderHelper::DRAW_SOLID_DEPTH);
}

void SceneCollisionDebugDrawer::reportErrorWarning(const char* warningString)
{
}

void SceneCollisionDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
}

void SceneCollisionDebugDrawer::setDebugMode(int debugMode)
{
    dbgMode = debugMode;
}

int SceneCollisionDebugDrawer::getDebugMode() const
{
    return dbgMode;
}
} // namespace DAVA
