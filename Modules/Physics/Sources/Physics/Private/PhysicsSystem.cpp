#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/BoxShapeComponent.h"
#include "Physics/CapsuleShapeComponent.h"
#include "Physics/SphereShapeComponent.h"
#include "Physics/PlaneShapeComponent.h"
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
        DVASSERT(iter != components.end());
        RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
    }
}

Array<uint32, 7> collisionShapeTypes = {
    Component::BOX_SHAPE_COMPONENT,
    Component::CAPSULE_SHAPE_COMPONENT,
    Component::SPHERE_SHAPE_COMPONENT,
    Component::PLANE_SHAPE_COMPONENT,
    Component::CONVEX_HULL_SHAPE_COMPONENT,
    Component::MESH_SHAPE_COMPONENT,
    Component::HEIGHT_FIELD_SHAPE_COMPONENT
};

bool IsCollisionShapeType(uint32 componentType)
{
    return std::any_of(collisionShapeTypes.begin(), collisionShapeTypes.end(), [componentType](uint32 type)
                       {
                           return componentType == type;
                       });
}
} // namespace

PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
{
    const KeyedArchive* options = Engine::Instance()->GetOptions();

    simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", 16 * 1024 * 512);
    DVASSERT((simulationBlockSize % (16 * 1024)) == 0);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
    simulationBlock = physics->Allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = options->GetVector3("physics.gravity", Vector3(0, 0, -9.81f));
    sceneConfig.threadCount = options->GetUInt32("physics.threadCount", 2);
    physicsScene = physics->CreateScene(sceneConfig);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }
    DVASSERT(simulationBlock != nullptr);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
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

    processEntity(entity, Component::STATIC_BODY_COMPONENT);
    processEntity(entity, Component::DYNAMIC_BODY_COMPONENT);
    for (uint32 type : PhysicsSystemDetail::collisionShapeTypes)
    {
        processEntity(entity, type);
    }
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

    processEntity(entity, Component::STATIC_BODY_COMPONENT);
    processEntity(entity, Component::DYNAMIC_BODY_COMPONENT);
    for (uint32 type : PhysicsSystemDetail::collisionShapeTypes)
    {
        processEntity(entity, type);
    }
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
}

void PhysicsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(component);
        PhysicsSystemDetail::EraseComponent(physicsComponent, pendingAddPhysicsComponents, physicsComponents);

        physx::PxRigidActor* actor = physicsComponent->GetPxActor()->is<physx::PxRigidActor>();
        if (actor != nullptr)
        {
            physx::PxU32 shapesCount = actor->getNbShapes();
            Vector<physx::PxShape*> shapes(shapesCount, nullptr);
            actor->getShapes(shapes.data(), shapesCount);

            for (physx::PxShape* shape : shapes)
            {
                DVASSERT(shape != nullptr);
                actor->detachShape(*shape);
            }

            physicsScene->removeActor(*physicsComponent->GetPxActor());
            physicsComponent->ReleasePxActor();
        }
    }

    using namespace PhysicsSystemDetail;
    if (IsCollisionShapeType(componentType))
    {
        CollisionShapeComponent* collisionComponent = static_cast<CollisionShapeComponent*>(component);
        PhysicsSystemDetail::EraseComponent(collisionComponent, pendingAddCollisionComponents, collisionComponents);

        physx::PxShape* shape = collisionComponent->GetPxShape();
        if (shape != nullptr)
        {
            physx::PxRigidActor* actor = shape->getActor();
            if (actor != nullptr)
            {
                actor->detachShape(*shape);
            }
            collisionComponent->ReleasePxShape();
        }
    }
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning == true)
    {
        FetchResults(false);
    }

    InitNewObjects();

    if (isSimulationEnabled == false)
    {
        //DrawDebugObject();
        SyncTransformToPhysx();
        return;
    }

    if (isSimulationRunning == false)
    {
        DrawDebugInfo();
        physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);
        isSimulationRunning = true;
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

void PhysicsSystem::SetDrawDebugInfo(bool drawDebugInfo_)
{
    drawDebugInfo = drawDebugInfo_;
    physx::PxReal enabled = drawDebugInfo == true ? 1.0f : 0.0f;
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 2.0f * enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, enabled);

    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_MASS_AXES, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_LIN_VELOCITY, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_ANG_VELOCITY, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_POINT, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_NORMAL, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_FORCE, enabled);
}

bool PhysicsSystem::IsDrawDebugInfo() const
{
    return drawDebugInfo;
}

bool PhysicsSystem::FetchResults(bool block)
{
    DVASSERT(isSimulationRunning);
    bool isFetched = physicsScene->fetchResults(block);
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
            PhysicsComponent* component = reinterpret_cast<PhysicsComponent*>(actor->userData);
            Entity* entity = component->GetEntity();

            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            DVASSERT(rigidActor != nullptr);

            entity->SetWorldTransform(PhysicsMath::PxMat44ToMatrix4(rigidActor->getGlobalPose()));
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
    if (IsDrawDebugInfo() == false)
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
    Physics* physics = GetEngineContext()->moduleManager->GetModule<Physics>();
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
        rigidActor->setGlobalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(component->GetEntity()->GetWorldTransform())));

        Entity* entity = component->GetEntity();
        AttachShape(entity, rigidActor);

        physicsScene->addActor(*(component->GetPxActor()));
        physicsComponents.push_back(component);
    }
    pendingAddPhysicsComponents.clear();

    auto attachShapeFn = [](physx::PxShape* shape, PhysicsComponent* component)
    {
        physx::PxRigidActor* actor = component->GetPxActor()->is<physx::PxRigidActor>();
        DVASSERT(actor != nullptr);
        actor->attachShape(*shape);
    };

    for (CollisionShapeComponent* component : pendingAddCollisionComponents)
    {
        physx::PxShape* shape = CreateShape(component, physics);
        if (shape != nullptr)
        {
            DVASSERT(shape != nullptr);

            Entity* entity = component->GetEntity();
            PhysicsComponent* staticBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::STATIC_BODY_COMPONENT, 0));
            if (staticBodyComponent != nullptr)
            {
                attachShapeFn(shape, staticBodyComponent);
            }
            else
            {
                PhysicsComponent* dynamicBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, 0));
                if (dynamicBodyComponent != nullptr)
                {
                    attachShapeFn(shape, dynamicBodyComponent);
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
        }
    }
    pendingAddCollisionComponents.clear();
}

void PhysicsSystem::AttachShape(Entity* entity, physx::PxRigidActor* actor)
{
    auto componentLoop = [](Entity* entity, physx::PxRigidActor* actor, uint32 componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            CollisionShapeComponent* collision = static_cast<CollisionShapeComponent*>(entity->GetComponent(componentType, i));
            physx::PxShape* shape = collision->GetPxShape();
            if (shape != nullptr)
            {
                actor->attachShape(*shape);
            }
        }
    };

    for (uint32 type : PhysicsSystemDetail::collisionShapeTypes)
    {
        componentLoop(entity, actor, type);
    }
}

physx::PxShape* PhysicsSystem::CreateShape(CollisionShapeComponent* component, Physics* physics)
{
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
        PlaneShapeComponent* planeComponent = static_cast<PlaneShapeComponent*>(component);
        shape = physics->CreatePlaneShape();
    }
    break;
    case Component::CONVEX_HULL_SHAPE_COMPONENT:
    {
        Entity* entity = component->GetEntity();
        RenderObject* ro = GetRenderObject(entity);
        DVASSERT(ro != nullptr);
        uint32 batchesCount = ro->GetRenderBatchCount();
        DVASSERT(batchesCount > 0);
        RenderBatch* batch = ro->GetRenderBatch(0);
        Vector3 pos;
        Vector3 scale;
        Quaternion quat;
        TransformComponent* transformComponent = GetTransformComponent(entity);
        transformComponent->GetWorldTransform().Decomposition(pos, scale, quat);
        shape = physics->CreateConvexHullShape(batch->GetPolygonGroup(), scale);
    }
    break;
    case Component::MESH_SHAPE_COMPONENT:
    {
        Entity* entity = component->GetEntity();
        RenderObject* ro = GetRenderObject(entity);
        DVASSERT(ro != nullptr);
        uint32 batchesCount = ro->GetRenderBatchCount();
        DVASSERT(batchesCount > 0);
        RenderBatch* batch = ro->GetRenderBatch(0);
        Vector3 pos;
        Vector3 scale;
        Quaternion quat;
        TransformComponent* transformComponent = GetTransformComponent(entity);
        transformComponent->GetWorldTransform().Decomposition(pos, scale, quat);
        shape = physics->CreateMeshShape(batch->GetPolygonGroup(), scale);
    }
    break;
    case Component::HEIGHT_FIELD_SHAPE_COMPONENT:
    {
        Entity* entity = component->GetEntity();
        Landscape* landscape = GetLandscape(entity);
        DVASSERT(landscape);
        Matrix4 localPose;
        shape = physics->CreateHeightField(landscape, localPose);
        component->SetLocalPose(localPose);
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
    auto updatePose = [this](Entity* e, PhysicsComponent* component)
    {
        if (component != nullptr)
        {
            physx::PxActor* actor = component->GetPxActor();
            DVASSERT(actor != nullptr);
            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            DVASSERT(rigidActor != nullptr);
            Matrix4 worldTransform = e->GetWorldTransform();
            Vector3 position;
            Vector3 scale;
            Quaternion rotation;
            worldTransform.Decomposition(position, scale, rotation);
            rigidActor->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(position), PhysicsMath::QuaternionToPxQuat(rotation)));

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

void PhysicsSystem::DrawDebugObject()
{
    physx::PxActorTypeFlags actorTypes = physx::PxActorTypeFlag::eRIGID_STATIC | physx::PxActorTypeFlag::eRIGID_DYNAMIC;
    physx::PxU32 count = physicsScene->getNbActors(actorTypes);
    Vector<physx::PxActor*> actors;
    actors.resize(count);
    Vector<physx::PxShape*> shapes;

    RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();

    count = physicsScene->getActors(actorTypes, actors.data(), static_cast<physx::PxU32>(actors.size()));
    for (physx::PxU32 actorIndex = 0; actorIndex < count; ++actorIndex)
    {
        physx::PxRigidActor* actor = actors[actorIndex]->is<physx::PxRigidActor>();
        DVASSERT(actor != nullptr);

        Matrix4 globalPose = PhysicsMath::PxMat44ToMatrix4(physx::PxMat44(actor->getGlobalPose()));

        physx::PxU32 shapesCount = actor->getNbShapes();
        shapes.resize(shapesCount);
        shapesCount = actor->getShapes(shapes.data(), static_cast<physx::PxU32>(shapes.size()));

        for (physx::PxU32 shapeIndex = 0; shapeIndex < shapesCount; ++shapeIndex)
        {
            physx::PxShape* shape = shapes[shapeIndex];
            physx::PxGeometryHolder geomHolder = shape->getGeometry();

            Matrix4 localPose = PhysicsMath::PxMat44ToMatrix4(physx::PxMat44(shape->getLocalPose()));
            Matrix4 finalTranfrom = globalPose * localPose;

            switch (geomHolder.getType())
            {
            case physx::PxGeometryType::eBOX:
            {
                const physx::PxBoxGeometry& boxGeom = geomHolder.box();
                Vector3 corner(boxGeom.halfExtents.x, boxGeom.halfExtents.y, boxGeom.halfExtents.z);
                AABBox3 box(-corner, corner);
                renderHelper->DrawAABoxTransformed(box, finalTranfrom, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
            }
            break;
            case physx::PxGeometryType::eSPHERE:
            {
                const physx::PxSphereGeometry& sphereGeom = geomHolder.sphere();
                Vector3 position(0.0f, 0.0f, 0.0f);
                position = position * finalTranfrom;
                renderHelper->DrawCircle(position, Vector3(1.0f, 0.0f, 0.0f), sphereGeom.radius, 32, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
                renderHelper->DrawCircle(position, Vector3(0.0f, 1.0f, 0.0f), sphereGeom.radius, 32, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
                renderHelper->DrawCircle(position, Vector3(0.0f, 0.0f, 1.0f), sphereGeom.radius, 32, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
            }
            break;
            case physx::PxGeometryType::eCAPSULE:
                //renderHelper->DrawIcosahedron();
                // not implemented
                break;
            case physx::PxGeometryType::ePLANE:
                // not implemented
                break;
            case physx::PxGeometryType::eTRIANGLEMESH:
            {
                const physx::PxTriangleMeshGeometry& meshGeom = geomHolder.triangleMesh();
                Matrix4 scaleMatrix = Matrix4::MakeScale(PhysicsMath::PxVec3ToVector3(meshGeom.scale.scale));
                Matrix4 meshTransform = scaleMatrix * finalTranfrom;
                physx::PxTriangleMesh* mesh = meshGeom.triangleMesh;

                const physx::PxU32 triangleCount = mesh->getNbTriangles();

                const physx::PxVec3* vertices = mesh->getVertices();
                const void* indices = mesh->getTriangles();

                const bool indices16Bits = mesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES;

                for (physx::PxU32 i = 0; i < triangleCount; ++i)
                {
                    physx::PxU32 i0 = indices16Bits ? ((physx::PxU16*)indices)[3 * i] : ((physx::PxU32*)indices)[3 * i];
                    physx::PxU32 i1 = indices16Bits ? ((physx::PxU16*)indices)[3 * i + 1] : ((physx::PxU32*)indices)[3 * i + 1];
                    physx::PxU32 i2 = indices16Bits ? ((physx::PxU16*)indices)[3 * i + 2] : ((physx::PxU32*)indices)[3 * i + 2];

                    Vector3 v0 = PhysicsMath::PxVec3ToVector3(vertices[i0]);
                    Vector3 v1 = PhysicsMath::PxVec3ToVector3(vertices[i1]);
                    Vector3 v2 = PhysicsMath::PxVec3ToVector3(vertices[i2]);

                    v0 = v0 * meshTransform;
                    v1 = v1 * meshTransform;
                    v2 = v2 * meshTransform;

                    Polygon3 polygon;
                    polygon.AddPoint(v0);
                    polygon.AddPoint(v1);
                    polygon.AddPoint(v2);
                    polygon.AddPoint(v0);
                    renderHelper->DrawPolygon(polygon, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
                }
            }
            break;
            case physx::PxGeometryType::eCONVEXMESH:
            {
                const physx::PxConvexMeshGeometry& meshGeom = geomHolder.convexMesh();
                Matrix4 scaleMatrix = Matrix4::MakeScale(PhysicsMath::PxVec3ToVector3(meshGeom.scale.scale));
                Matrix4 meshTransform = scaleMatrix * finalTranfrom;
                physx::PxConvexMesh* mesh = meshGeom.convexMesh;

                physx::PxU32 polygonCount = mesh->getNbPolygons();
                const physx::PxU8* indices = mesh->getIndexBuffer();
                const physx::PxVec3* vertices = mesh->getVertices();
                for (physx::PxU32 i = 0; i < polygonCount; ++i)
                {
                    physx::PxHullPolygon data;
                    mesh->getPolygonData(i, data);

                    physx::PxU16 vertexCount = data.mNbVerts;
                    for (physx::PxU16 j = 1; j < vertexCount; j++)
                    {
                        Vector3 v0 = PhysicsMath::PxVec3ToVector3(vertices[indices[j - 1]]);
                        Vector3 v1 = PhysicsMath::PxVec3ToVector3(vertices[indices[j]]);

                        v0 = v0 * meshTransform;
                        v1 = v1 * meshTransform;

                        renderHelper->DrawLine(v0, v1, Color::White);
                    }

                    {
                        Vector3 v0 = PhysicsMath::PxVec3ToVector3(vertices[indices[0]]);
                        Vector3 v1 = PhysicsMath::PxVec3ToVector3(vertices[indices[vertexCount - 1]]);

                        v0 = v0 * meshTransform;
                        v1 = v1 * meshTransform;

                        renderHelper->DrawLine(v0, v1, Color::White);
                    }

                    indices += vertexCount;
                }
            }
            break;
            case physx::PxGeometryType::eHEIGHTFIELD:
                break;
            default:
                DVASSERT(false);
                break;
            }
        }
    }
}

} // namespace DAVA
