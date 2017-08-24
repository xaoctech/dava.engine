#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/BoxShapeComponent.h"
#include "Physics/CapsuleShapeComponent.h"
#include "Physics/SphereShapeComponent.h"
#include "Physics/PlaneShapeComponent.h"
#include "Physics/PhysicsGeometryCache.h"
#include "Physics/Private/PhysicsMath.h"

#include <Scene3D/Entity.h>
#include <Entity/Component.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/RenderHelper.h>
#include <FileSystem/KeyedArchive.h>
#include <Utils/Utils.h>

#include <physx/PxScene.h>
#include <physx/PxRigidActor.h>
#include <physx/PxRigidDynamic.h>
#include <physx/common/PxRenderBuffer.h>
#include <PxShared/foundation/PxAllocatorCallback.h>
#include <PxShared/foundation/PxFoundation.h>

#include <functional>

namespace DAVA
{
namespace PhysicsSystemDetail
{
template <typename T>
void EraseComponent(T* component, Vector<T*>& pendingComponents, Vector<T*>& components)
{
    auto addIter = std::find(pendingComponents.begin(), pendingComponents.end(), component);
    if (addIter != pendingComponents.end())
    {
        RemoveExchangingWithLast(pendingComponents, std::distance(pendingComponents.begin(), addIter));
    }
    else
    {
        auto iter = std::find(components.begin(), components.end(), component);
        if (iter != components.end())
        {
            RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
        }
    }
}

void UpdateGlobalPose(physx::PxRigidActor* actor, const Matrix4& m, Vector3& scale)
{
    Vector3 position;
    Quaternion rotation;
    m.Decomposition(position, scale, rotation);
    actor->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(position), PhysicsMath::QuaternionToPxQuat(rotation)));
}

bool IsCollisionShapeType(uint32 componentType)
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<uint32>& shapeComponents = module->GetShapeComponentTypes();
    return std::any_of(shapeComponents.begin(), shapeComponents.end(), [componentType](uint32 type) {
        return componentType == type;
    });
}

Vector3 AccumulateMeshInfo(Entity* e, Vector<PolygonGroup*>& groups)
{
    RenderObject* ro = GetRenderObject(e);
    if (ro != nullptr)
    {
        uint32 batchesCount = ro->GetRenderBatchCount();
        int32 maxLod = ro->GetMaxLodIndex();
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            int32 lodIndex = -1;
            int32 switchIndex = -1;
            RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
            if (lodIndex == maxLod)
            {
                PolygonGroup* group = batch->GetPolygonGroup();
                if (group != nullptr)
                {
                    groups.push_back(group);
                }
            }
        }
    }

    Vector3 pos;
    Vector3 scale;
    Quaternion quat;
    TransformComponent* transformComponent = GetTransformComponent(e);
    transformComponent->GetWorldTransform().Decomposition(pos, scale, quat);

    return scale;
}

const uint32 DEFAULT_SIMULATION_BLOCK_SIZE = 16 * 1024 * 512;
} // namespace

PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
{
    const KeyedArchive* options = Engine::Instance()->GetOptions();

    simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", PhysicsSystemDetail::DEFAULT_SIMULATION_BLOCK_SIZE);
    DVASSERT((simulationBlockSize % (16 * 1024)) == 0); // simulationBlockSize must be 16K multiplier

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    simulationBlock = physics->Allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = options->GetVector3("physics.gravity", Vector3(0, 0, -9.81f));
    sceneConfig.threadCount = options->GetUInt32("physics.threadCount", 2);
    geometryCache = new PhysicsGeometryCache();
    physicsScene = physics->CreateScene(sceneConfig);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }
    DVASSERT(simulationBlock != nullptr);
    SafeDelete(geometryCache);

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    physics->Deallocate(simulationBlock);
    simulationBlock = nullptr;
    physicsScene->release();
}

void PhysicsSystem::RegisterEntity(Entity* entity)
{
    auto processEntity = [this](Entity* entity, uint32 componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            RegisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<uint32>& bodyComponents = module->GetBodyComponentTypes();
    const Vector<uint32>& shapeComponents = module->GetShapeComponentTypes();

    for (uint32 type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : shapeComponents)
    {
        processEntity(entity, type);
    }

    processEntity(entity, Component::RENDER_COMPONENT);
}

void PhysicsSystem::UnregisterEntity(Entity* entity)
{
    auto processEntity = [this](Entity* entity, uint32 componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            UnregisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<uint32>& bodyComponents = module->GetBodyComponentTypes();
    const Vector<uint32>& shapeComponents = module->GetShapeComponentTypes();

    for (uint32 type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : shapeComponents)
    {
        processEntity(entity, type);
    }
    processEntity(entity, Component::RENDER_COMPONENT);
}

void PhysicsSystem::RegisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        pendingAddPhysicsComponents.push_back(static_cast<PhysicsComponent*>(component));
    }

    using namespace PhysicsSystemDetail;
    if (IsCollisionShapeType(componentType))
    {
        pendingAddCollisionComponents.push_back(static_cast<CollisionShapeComponent*>(component));
    }

    if (componentType == Component::RENDER_COMPONENT)
    {
        auto iter = waitRenderInfoComponents.find(entity);
        if (iter != waitRenderInfoComponents.end())
        {
            pendingAddCollisionComponents.insert(pendingAddCollisionComponents.end(), iter->second.begin(), iter->second.end());
            waitRenderInfoComponents.erase(iter);
        }
    }
}

void PhysicsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(component);
        PhysicsSystemDetail::EraseComponent(physicsComponent, pendingAddPhysicsComponents, physicsComponents);

        physx::PxActor* actor = physicsComponent->GetPxActor();
        if (actor != nullptr)
        {
            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            if (rigidActor != nullptr)
            {
                physx::PxU32 shapesCount = rigidActor->getNbShapes();
                Vector<physx::PxShape*> shapes(shapesCount, nullptr);
                rigidActor->getShapes(shapes.data(), shapesCount);

                for (physx::PxShape* shape : shapes)
                {
                    DVASSERT(shape != nullptr);
                    rigidActor->detachShape(*shape);
                }

                physicsScene->removeActor(*physicsComponent->GetPxActor());
                physicsComponent->ReleasePxActor();
            }
        }
    }

    using namespace PhysicsSystemDetail;
    if (IsCollisionShapeType(componentType))
    {
        CollisionShapeComponent* collisionComponent = static_cast<CollisionShapeComponent*>(component);
        PhysicsSystemDetail::EraseComponent(collisionComponent, pendingAddCollisionComponents, collisionComponents);

        auto iter = waitRenderInfoComponents.find(entity);
        if (iter != waitRenderInfoComponents.end())
        {
            FindAndRemoveExchangingWithLast(iter->second, collisionComponent);
        }

        ReleaseShape(collisionComponent);
    }

    if (componentType == Component::RENDER_COMPONENT)
    {
        Vector<CollisionShapeComponent*>* waitingComponents = nullptr;
        auto collisionProcess = [&](uint32 componentType)
        {
            for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(componentType, i));
                auto iter = std::find(collisionComponents.begin(), collisionComponents.end(), component);
                if (iter == collisionComponents.end())
                {
                    continue;
                }

                if (waitingComponents == nullptr)
                {
                    waitingComponents = &waitRenderInfoComponents[entity];
                }

                waitingComponents->push_back(component);
                ReleaseShape(component);
                PhysicsSystemDetail::EraseComponent(component, pendingAddCollisionComponents, collisionComponents);
            }
        };

        collisionProcess(Component::CONVEX_HULL_SHAPE_COMPONENT);
        collisionProcess(Component::MESH_SHAPE_COMPONENT);
        collisionProcess(Component::HEIGHT_FIELD_SHAPE_COMPONENT);
    }
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning == true)
    {
        FetchResults(false);
    }

    if (isSimulationRunning == false)
    {
        InitNewObjects();
        UpdateComponents();

        if (isSimulationEnabled == false)
        {
            SyncTransformToPhysx();
        }
        else
        {
            DrawDebugInfo();
            physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);
            isSimulationRunning = true;
        }
    }
}

void PhysicsSystem::SetSimulationEnabled(bool isEnabled)
{
    if (isSimulationEnabled != isEnabled)
    {
        if (isSimulationRunning == true)
        {
            DVASSERT(isSimulationEnabled == true);
            bool success = FetchResults(true);
            DVASSERT(success == true);
        }

        isSimulationEnabled = isEnabled;
    }
}

bool PhysicsSystem::IsSimulationEnabled() const
{
    return isSimulationEnabled;
}

void PhysicsSystem::SetDebugDrawEnabled(bool drawDebugInfo_)
{
    drawDebugInfo = drawDebugInfo_;
    physx::PxReal enabled = drawDebugInfo == true ? 1.0f : 0.0f;
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_MASS_AXES, enabled);
}

bool PhysicsSystem::IsDebugDrawEnabled() const
{
    return drawDebugInfo;
}

bool PhysicsSystem::FetchResults(bool waitForFetchFinish)
{
    DVASSERT(isSimulationRunning);
    bool isFetched = physicsScene->fetchResults(waitForFetchFinish);
    if (isFetched == true)
    {
        isSimulationRunning = false;
        physx::PxU32 actorsCount = 0;
        physx::PxActor** actors = physicsScene->getActiveActors(actorsCount);

        Vector<Entity*> activeEntities;
        activeEntities.reserve(actorsCount);

        for (physx::PxU32 i = 0; i < actorsCount; ++i)
        {
            physx::PxActor* actor = actors[i];
            PhysicsComponent* component = PhysicsComponent::GetComponent(actor);
            Entity* entity = component->GetEntity();

            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            DVASSERT(rigidActor != nullptr);

            Matrix4 scaleMatrix = Matrix4::MakeScale(component->currentScale);

            entity->SetWorldTransform(scaleMatrix * PhysicsMath::PxMat44ToMatrix4(rigidActor->getGlobalPose()));
            activeEntities.push_back(entity);
        }

        Scene* scene = GetScene();
        if (scene->transformSingleComponent != nullptr)
        {
            for (Entity* entity : activeEntities)
            {
                DVASSERT(entity->GetScene() == scene);
                scene->transformSingleComponent->worldTransformChanged.Push(entity);
            }
        }
    }

    return isFetched;
}

void PhysicsSystem::DrawDebugInfo()
{
    DVASSERT(isSimulationRunning == false);
    DVASSERT(isSimulationEnabled == true);
    if (IsDebugDrawEnabled() == false)
    {
        return;
    }

    RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
    const physx::PxRenderBuffer& rb = physicsScene->getRenderBuffer();
    const physx::PxDebugLine* lines = rb.getLines();
    for (physx::PxU32 i = 0; i < rb.getNbLines(); ++i)
    {
        const physx::PxDebugLine& line = lines[i];
        renderHelper->DrawLine(PhysicsMath::PxVec3ToVector3(line.pos0), PhysicsMath::PxVec3ToVector3(line.pos1),
                               PhysicsMath::PxColorToColor(line.color0));
    }

    const physx::PxDebugTriangle* triangles = rb.getTriangles();
    for (physx::PxU32 i = 0; i < rb.getNbTriangles(); ++i)
    {
        const physx::PxDebugTriangle& triangle = triangles[i];
        Polygon3 polygon;
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos0));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos1));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos2));
        renderHelper->DrawPolygon(polygon, PhysicsMath::PxColorToColor(triangle.color0), RenderHelper::DRAW_WIRE_DEPTH);
    }

    const physx::PxDebugPoint* points = rb.getPoints();
    for (physx::PxU32 i = 0; i < rb.getNbPoints(); ++i)
    {
        const physx::PxDebugPoint& point = points[i];
        renderHelper->DrawIcosahedron(PhysicsMath::PxVec3ToVector3(point.pos), 5.0f, PhysicsMath::PxColorToColor(point.color), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void PhysicsSystem::InitNewObjects()
{
    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    for (PhysicsComponent* component : pendingAddPhysicsComponents)
    {
        uint32 componentType = component->GetType();
        physx::PxActor* createdActor = nullptr;
        if (componentType == Component::STATIC_BODY_COMPONENT)
        {
            createdActor = physics->CreateStaticActor();
        }
        else
        {
            DVASSERT(componentType == Component::DYNAMIC_BODY_COMPONENT);
            createdActor = physics->CreateDynamicActor();
        }

        component->SetPxActor(createdActor);
        physx::PxRigidActor* rigidActor = createdActor->is<physx::PxRigidActor>();
        Vector3 scale;
        PhysicsSystemDetail::UpdateGlobalPose(rigidActor, component->GetEntity()->GetWorldTransform(), scale);
        component->currentScale = scale;

        Entity* entity = component->GetEntity();
        AttachShape(entity, component, scale);

        physicsScene->addActor(*(component->GetPxActor()));
        physicsComponents.push_back(component);
    }
    pendingAddPhysicsComponents.clear();

    for (CollisionShapeComponent* component : pendingAddCollisionComponents)
    {
        physx::PxShape* shape = CreateShape(component, physics);
        if (shape != nullptr)
        {
            Entity* entity = component->GetEntity();
            Matrix4 worldTransform = entity->GetWorldTransform();
            Vector3 position, scale, rotation;
            worldTransform.Decomposition(position, scale, rotation);

            PhysicsComponent* staticBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::STATIC_BODY_COMPONENT, 0));
            if (staticBodyComponent != nullptr)
            {
                AttachShape(staticBodyComponent, component, scale);
            }
            else
            {
                PhysicsComponent* dynamicBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, 0));
                if (dynamicBodyComponent != nullptr)
                {
                    AttachShape(dynamicBodyComponent, component, scale);

                    physx::PxRigidDynamic* dynamicActor = dynamicBodyComponent->GetPxActor()->is<physx::PxRigidDynamic>();
                    DVASSERT(dynamicActor != nullptr);
                    if (dynamicActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) == false &&
                        dynamicActor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC) == false)
                    {
                        dynamicActor->wakeUp();
                    }
                }
            }

            collisionComponents.push_back(component);
            shape->release();
        }
    }
    pendingAddCollisionComponents.clear();
}

void PhysicsSystem::AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale)
{
    physx::PxActor* actor = bodyComponent->GetPxActor();
    DVASSERT(actor);
    physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
    DVASSERT(rigidActor);

    shapeComponent->scale = scale;

    physx::PxShape* shape = shapeComponent->GetPxShape();
    if (shape != nullptr)
    {
        rigidActor->attachShape(*shape);
        ScheduleUpdate(shapeComponent);
        ScheduleUpdate(bodyComponent);
    }
}

void PhysicsSystem::AttachShape(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale)
{
    for (uint32 type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            AttachShape(bodyComponent, static_cast<CollisionShapeComponent*>(entity->GetComponent(type, i)), scale);
        }
    }
}

physx::PxShape* PhysicsSystem::CreateShape(CollisionShapeComponent* component, PhysicsModule* physics)
{
    using namespace PhysicsSystemDetail;
    physx::PxShape* shape = nullptr;
    switch (component->GetType())
    {
    case Component::BOX_SHAPE_COMPONENT:
    {
        BoxShapeComponent* boxShape = static_cast<BoxShapeComponent*>(component);
        shape = physics->CreateBoxShape(boxShape->GetHalfSize());
    }
    break;
    case Component::CAPSULE_SHAPE_COMPONENT:
    {
        CapsuleShapeComponent* capsuleShape = static_cast<CapsuleShapeComponent*>(component);
        shape = physics->CreateCapsuleShape(capsuleShape->GetRadius(), capsuleShape->GetHalfHeight());
    }
    break;
    case Component::SPHERE_SHAPE_COMPONENT:
    {
        SphereShapeComponent* sphereShape = static_cast<SphereShapeComponent*>(component);
        shape = physics->CreateSphereShape(sphereShape->GetRadius());
    }
    break;
    case Component::PLANE_SHAPE_COMPONENT:
    {
        shape = physics->CreatePlaneShape();
    }
    break;
    case Component::CONVEX_HULL_SHAPE_COMPONENT:
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateConvexHullShape(std::move(groups), scale, geometryCache);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }
    break;
    case Component::MESH_SHAPE_COMPONENT:
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateMeshShape(std::move(groups), scale, geometryCache);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }
    break;
    case Component::HEIGHT_FIELD_SHAPE_COMPONENT:
    {
        Entity* entity = component->GetEntity();
        Landscape* landscape = GetLandscape(entity);
        if (landscape != nullptr)
        {
            Matrix4 localPose;
            shape = physics->CreateHeightField(landscape, localPose);
            component->SetLocalPose(localPose);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }
    break;
    default:
        DVASSERT(false);
        break;
    }

    if (shape != nullptr)
    {
        component->SetPxShape(shape);
    }

    return shape;
}

void PhysicsSystem::SyncTransformToPhysx()
{
    TransformSingleComponent* transformSingle = GetScene()->transformSingleComponent;
    for (Entity* entity : transformSingle->localTransformChanged)
    {
        SyncEntityTransformToPhysx(entity);
    }

    for (auto& mapNode : transformSingle->worldTransformChanged.map)
    {
        for (Entity* entity : mapNode.second)
        {
            SyncEntityTransformToPhysx(entity);
        }
    }
}

void PhysicsSystem::SyncEntityTransformToPhysx(Entity* entity)
{
    DVASSERT(isSimulationEnabled == false);
    DVASSERT(isSimulationRunning == false);
    TransformSingleComponent* transformSingle = GetScene()->transformSingleComponent;
    if (transformSingle == nullptr)
    {
        return;
    }

    auto updatePose = [this](Entity* e, PhysicsComponent* component)
    {
        if (component != nullptr)
        {
            physx::PxActor* actor = component->GetPxActor();
            DVASSERT(actor != nullptr);
            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            DVASSERT(rigidActor != nullptr);
            Vector3 scale;
            PhysicsSystemDetail::UpdateGlobalPose(rigidActor, e->GetWorldTransform(), scale);

            physx::PxU32 shapesCount = rigidActor->getNbShapes();
            for (physx::PxU32 i = 0; i < shapesCount; ++i)
            {
                physx::PxShape* shape = nullptr;
                rigidActor->getShapes(&shape, 1, i);

                physx::PxGeometryHolder geomHolder = shape->getGeometry();

                physx::PxGeometryType::Enum geomType = geomHolder.getType();
                if (geomType == physx::PxGeometryType::eTRIANGLEMESH)
                {
                    physx::PxTriangleMeshGeometry geom;
                    bool extracted = shape->getTriangleMeshGeometry(geom);
                    DVASSERT(extracted);
                    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
                    shape->setGeometry(geom);
                }
                else if (geomType == physx::PxGeometryType::eCONVEXMESH)
                {
                    physx::PxConvexMeshGeometry geom;
                    bool extracted = shape->getConvexMeshGeometry(geom);
                    DVASSERT(extracted);
                    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
                    shape->setGeometry(geom);
                }
            }
        }
    };

    PhysicsComponent* staticBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::STATIC_BODY_COMPONENT, 0));
    updatePose(entity, staticBodyComponent);

    PhysicsComponent* dynamicBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, 0));
    updatePose(entity, dynamicBodyComponent);

    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        SyncEntityTransformToPhysx(entity->GetChild(i));
    }
}

void PhysicsSystem::ReleaseShape(CollisionShapeComponent* component)
{
    physx::PxShape* shape = component->GetPxShape();
    if (shape == nullptr)
    {
        return;
    }
    DVASSERT(shape->isExclusive() == true);

    physx::PxActor* actor = shape->getActor();
    if (actor != nullptr)
    {
        actor->is<physx::PxRigidActor>()->detachShape(*shape);
    }

    component->ReleasePxShape();
}

void PhysicsSystem::ScheduleUpdate(PhysicsComponent* component)
{
    physicsComponensUpdatePending.insert(component);
}

void PhysicsSystem::ScheduleUpdate(CollisionShapeComponent* component)
{
    collisionComponentsUpdatePending.insert(component);
}

bool PhysicsSystem::Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback)
{
    using namespace physx;

    return physicsScene->raycast(PhysicsMath::Vector3ToPxVec3(origin), PhysicsMath::Vector3ToPxVec3(Normalize(direction)),
                                 static_cast<PxReal>(distance), callback);
}

void PhysicsSystem::UpdateComponents()
{
    for (CollisionShapeComponent* shapeComponent : collisionComponentsUpdatePending)
    {
        shapeComponent->UpdateLocalProperties();
        physx::PxShape* shape = shapeComponent->GetPxShape();
        DVASSERT(shape != nullptr);
        physx::PxActor* actor = shape->getActor();
        if (actor != nullptr)
        {
            PhysicsComponent* bodyComponent = PhysicsComponent::GetComponent(actor);
            physicsComponensUpdatePending.insert(bodyComponent);
        }
    }

    for (PhysicsComponent* bodyComponent : physicsComponensUpdatePending)
    {
        bodyComponent->UpdateLocalProperties();

        physx::PxRigidDynamic* dynamicActor = bodyComponent->GetPxActor()->is<physx::PxRigidDynamic>();
        if (dynamicActor != nullptr)
        {
            physx::PxU32 shapesCount = dynamicActor->getNbShapes();
            if (shapesCount > 0)
            {
                Vector<physx::PxShape*> shapes(shapesCount, nullptr);
                physx::PxU32 extractedShapesCount = dynamicActor->getShapes(shapes.data(), shapesCount);
                DVASSERT(shapesCount == extractedShapesCount);

                Vector<physx::PxReal> masses;
                masses.reserve(shapesCount);

                for (physx::PxShape* shape : shapes)
                {
                    CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                    masses.push_back(shapeComponent->GetMass());
                }

                physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, masses.data(), static_cast<physx::PxU32>(masses.size()));
            }
        }
    }

    collisionComponentsUpdatePending.clear();
    physicsComponensUpdatePending.clear();
}

} // namespace DAVA
