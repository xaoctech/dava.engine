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
#include "Physics/PhysicsUtils.h"
#include "Physics/BoxCharacterControllerComponent.h"
#include "Physics/CapsuleCharacterControllerComponent.h"
#include "Physics/CollisionSingleComponent.h"
#include "Physics/Private/PhysicsMath.h"
#include "Physics/PhysicsVehiclesSubsystem.h"

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

void UpdateShapeLocalPose(physx::PxShape* shape, const Matrix4& m)
{
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    m.Decomposition(position, scale, rotation);
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(position), PhysicsMath::QuaternionToPxQuat(rotation)));
}

bool IsCollisionShapeType(uint32 componentType)
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<uint32>& shapeComponents = module->GetShapeComponentTypes();
    return std::any_of(shapeComponents.begin(), shapeComponents.end(), [componentType](uint32 type) {
        return componentType == type;
    });
}

bool IsCharacterControllerType(uint32 componentType)
{
    return componentType == Component::BOX_CHARACTER_CONTROLLER_COMPONENT || componentType == Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT;
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

PhysicsComponent* GetParentPhysicsComponent(Entity* entity)
{
    PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::STATIC_BODY_COMPONENT));
    if (physicsComponent == nullptr)
    {
        physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT));
    }

    if (physicsComponent != nullptr)
    {
        return physicsComponent;
    }
    else
    {
        // Move up in the hierarchy
        Entity* parent = entity->GetParent();
        if (parent != nullptr)
        {
            return GetParentPhysicsComponent(parent);
        }
        else
        {
            return nullptr;
        }
    }
}

const uint32 DEFAULT_SIMULATION_BLOCK_SIZE = 16 * 1024 * 512;
} // namespace

physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0,
                                  physx::PxFilterData filterData0,
                                  physx::PxFilterObjectAttributes attributes1,
                                  physx::PxFilterData filterData1,
                                  physx::PxPairFlags& pairFlags,
                                  const void* constantBlock,
                                  physx::PxU32 constantBlockSize)
{
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);

    // PxFilterData for a shape is used this way:
    // - PxFilterData.word0 is used for engine-specific features (i.e. for CCD)
    // - PxFilterData.word1 is a bitmask for encoding type of object
    // - PxFilterData.word2 is a bitmask for encoding types of objects this object collides with
    // - PxFilterData.word3 is not used right now
    // Type of a shape and types it collides with can be set using CollisionShapeComponent::SetTypeMask and CollisionShapeComponent::SetTypeMaskToCollideWith methods

    if ((filterData0.word1 & filterData1.word2) == 0 &&
        (filterData1.word1 & filterData0.word2) == 0)
    {
        // If these types of objects do not collide, ignore this pair unless filter data for either of them changes
        return physx::PxFilterFlag::eSUPPRESS;
    }

    pairFlags =
    physx::PxPairFlag::eCONTACT_DEFAULT | // default collision processing
    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | // notify about a first contact
    physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | // notify about ongoing contacts
    physx::PxPairFlag::eNOTIFY_CONTACT_POINTS; // report contact points

    return physx::PxFilterFlag::eDEFAULT;
}

PhysicsSystem::SimulationEventCallback::SimulationEventCallback(DAVA::CollisionSingleComponent* targetCollisionSingleComponent)
    : targetCollisionSingleComponent(targetCollisionSingleComponent)
{
    DVASSERT(targetCollisionSingleComponent != nullptr);
}

void PhysicsSystem::SimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onWake(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onSleep(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onTrigger(physx::PxTriggerPair*, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    // Buffer for extracting physx contact points
    static const size_t MAX_CONTACT_POINTS_COUNT = 10;
    static physx::PxContactPairPoint physxContactPoints[MAX_CONTACT_POINTS_COUNT];

    for (physx::PxU32 i = 0; i < nbPairs; ++i)
    {
        const physx::PxContactPair& pair = pairs[i];

        if (pair.contactCount > 0)
        {
            if ((pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0) ||
                (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1))
            {
                // Either first or second shape has been removed from the scene
                // Do not report such contacts
                continue;
            }

            // Extract physx points
            const physx::PxU32 contactPointsCount = pair.extractContacts(&physxContactPoints[0], MAX_CONTACT_POINTS_COUNT);
            DVASSERT(contactPointsCount > 0);

            Vector<CollisionPoint> davaContactPoints(contactPointsCount);

            // Convert each contact point from physx structure to engine structure
            for (size_t j = 0; j < contactPointsCount; ++j)
            {
                CollisionPoint& davaPoint = davaContactPoints[j];
                physx::PxContactPairPoint& physxPoint = physxContactPoints[j];

                davaPoint.position = PhysicsMath::PxVec3ToVector3(physxPoint.position);
                davaPoint.impulse = PhysicsMath::PxVec3ToVector3(physxPoint.impulse);
            }

            CollisionShapeComponent* firstCollisionComponent = CollisionShapeComponent::GetComponent(pair.shapes[0]);
            DVASSERT(firstCollisionComponent != nullptr);

            CollisionShapeComponent* secondCollisionComponent = CollisionShapeComponent::GetComponent(pair.shapes[1]);
            DVASSERT(secondCollisionComponent != nullptr);

            Entity* firstEntity = firstCollisionComponent->GetEntity();
            DVASSERT(firstEntity != nullptr);

            Entity* secondEntity = secondCollisionComponent->GetEntity();
            DVASSERT(secondEntity != nullptr);

            // Register collision
            CollisionInfo collisionInfo;
            collisionInfo.first = firstEntity;
            collisionInfo.second = secondEntity;
            collisionInfo.points = std::move(davaContactPoints);
            targetCollisionSingleComponent->collisions.push_back(collisionInfo);
        }
    }
}

PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
    , simulationEventCallback(scene->collisionSingleComponent)
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

    physicsScene = physics->CreateScene(sceneConfig, FilterShader, &simulationEventCallback);

    vehiclesSubsystem = new PhysicsVehiclesSubsystem(scene, physicsScene);
    controllerManager = PxCreateControllerManager(*physicsScene);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }

    SafeDelete(vehiclesSubsystem);

    DVASSERT(simulationBlock != nullptr);
    SafeDelete(geometryCache);

    controllerManager->release();

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
    const Vector<uint32>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (uint32 type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : shapeComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : characterControllerComponents)
    {
        processEntity(entity, type);
    }

    processEntity(entity, Component::RENDER_COMPONENT);

    vehiclesSubsystem->RegisterEntity(entity);
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
    const Vector<uint32>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (uint32 type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : shapeComponents)
    {
        processEntity(entity, type);
    }

    for (uint32 type : characterControllerComponents)
    {
        processEntity(entity, type);
    }

    processEntity(entity, Component::RENDER_COMPONENT);

    vehiclesSubsystem->UnregisterEntity(entity);
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

    if (IsCharacterControllerType(componentType))
    {
        pendingAddCharacterControllerComponents.push_back(static_cast<CharacterControllerComponent*>(component));
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

    vehiclesSubsystem->RegisterComponent(entity, component);
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
            }
            physicsScene->removeActor(*actor);
            physicsComponent->ReleasePxActor();
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

    if (IsCharacterControllerType(componentType))
    {
        CharacterControllerComponent* controllerComponent = static_cast<CharacterControllerComponent*>(component);
        PhysicsSystemDetail::EraseComponent(controllerComponent, pendingAddCharacterControllerComponents, characterControllerComponents);

        if (controllerComponent->controller != nullptr)
        {
            controllerComponent->controller->release();
        }
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

    vehiclesSubsystem->UnregisterComponent(entity, component);
}

void PhysicsSystem::PrepareForRemove()
{
    waitRenderInfoComponents.clear();
    physicsComponensUpdatePending.clear();
    collisionComponentsUpdatePending.clear();
    for (CollisionShapeComponent* component : collisionComponents)
    {
        ReleaseShape(component);
    }
    collisionComponents.clear();

    for (CollisionShapeComponent* component : pendingAddCollisionComponents)
    {
        ReleaseShape(component);
    }
    pendingAddCollisionComponents.clear();

    for (PhysicsComponent* component : physicsComponents)
    {
        physx::PxActor* actor = component->GetPxActor();
        if (actor != nullptr)
        {
            physicsScene->removeActor(*actor);
            component->ReleasePxActor();
        }
    }
    physicsComponents.clear();

    for (PhysicsComponent* component : pendingAddPhysicsComponents)
    {
        physx::PxActor* actor = component->GetPxActor();
        if (actor != nullptr)
        {
            physicsScene->removeActor(*actor);
            component->ReleasePxActor();
        }
    }
    pendingAddPhysicsComponents.clear();
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

            vehiclesSubsystem->Simulate(timeElapsed);
            MoveCharacterControllers(timeElapsed);
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

        vehiclesSubsystem->OnSimulationEnabled(isSimulationEnabled);
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

        for (physx::PxU32 i = 0; i < actorsCount; ++i)
        {
            physx::PxActor* actor = actors[i];
            PhysicsComponent* component = PhysicsComponent::GetComponent(actor);

            // When character controller is created, actor is created by physx implicitly
            // In this case there is no PhysicsComponent attached to this entity
            if (component != nullptr)
            {
                Entity* entity = component->GetEntity();

                physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
                DVASSERT(rigidActor != nullptr);

                // Update entity's transform and its shapes down the hierarchy recursively

                Matrix4 scaleMatrix = Matrix4::MakeScale(component->currentScale);
                entity->SetWorldTransform(scaleMatrix * PhysicsMath::PxMat44ToMatrix4(rigidActor->getGlobalPose()));

                Vector<Entity*> children;
                entity->GetChildEntitiesWithCondition(children, [component](Entity* e) { return PhysicsSystemDetail::GetParentPhysicsComponent(e) == component; });

                for (int i = 0; i < children.size(); ++i)
                {
                    Entity* child = children[i];
                    DVASSERT(child != nullptr);

                    Vector<CollisionShapeComponent*> shapes = PhysicsUtils::GetShapeComponents(child);
                    if (shapes.size() > 0)
                    {
                        // Update entity using just first shape for now
                        CollisionShapeComponent* shape = shapes[0];

                        Matrix4 scaleMatrix = Matrix4::MakeScale(shape->scale);
                        child->SetLocalTransform(scaleMatrix * PhysicsMath::PxMat44ToMatrix4(shape->GetPxShape()->getLocalPose()));
                    }
                }
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
        AttachShapesRecursively(entity, component, scale);

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

            PhysicsComponent* physicsComponent = PhysicsSystemDetail::GetParentPhysicsComponent(entity);
            if (physicsComponent != nullptr)
            {
                if (physicsComponent->GetType() == Component::STATIC_BODY_COMPONENT)
                {
                    AttachShapesRecursively(entity, physicsComponent, scale);
                }
                else
                {
                    AttachShapesRecursively(entity, physicsComponent, scale);

                    physx::PxRigidDynamic* dynamicActor = physicsComponent->GetPxActor()->is<physx::PxRigidDynamic>();
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

    for (CharacterControllerComponent* component : pendingAddCharacterControllerComponents)
    {
        Entity* entity = component->GetEntity();
        DVASSERT(entity != nullptr);

        // Character controllers are only allowed to be root objects
        DVASSERT(entity->GetParent() == entity->GetScene());

        const EngineContext* ctx = GetEngineContext();
        PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();

        physx::PxController* controller = nullptr;
        if (component->GetType() == Component::BOX_CHARACTER_CONTROLLER_COMPONENT)
        {
            BoxCharacterControllerComponent* boxCharacterControllerComponent = static_cast<BoxCharacterControllerComponent*>(component);

            physx::PxBoxControllerDesc desc;
            desc.position = PhysicsMath::Vector3ToPxExtendedVec3(entity->GetLocalTransform().GetTranslationVector());
            desc.halfHeight = boxCharacterControllerComponent->GetHalfHeight();
            desc.halfForwardExtent = boxCharacterControllerComponent->GetHalfForwardExtent();
            desc.halfSideExtent = boxCharacterControllerComponent->GetHalfSideExtent();
            desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
            desc.material = physics->GetDefaultMaterial();
            DVASSERT(desc.isValid());

            controller = controllerManager->createController(desc);
        }
        else if (component->GetType() == Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT)
        {
            CapsuleCharacterControllerComponent* capsuleCharacterControllerComponent = static_cast<CapsuleCharacterControllerComponent*>(component);

            physx::PxCapsuleControllerDesc desc;
            desc.position = PhysicsMath::Vector3ToPxExtendedVec3(entity->GetLocalTransform().GetTranslationVector());
            desc.radius = capsuleCharacterControllerComponent->GetRadius();
            desc.height = capsuleCharacterControllerComponent->GetHeight();
            desc.material = physics->GetDefaultMaterial();
            desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
            DVASSERT(desc.isValid());

            controller = controllerManager->createController(desc);
        }

        DVASSERT(controller != nullptr);
        component->controller = controller;

        characterControllerComponents.push_back(component);
    }
    pendingAddCharacterControllerComponents.clear();
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

void PhysicsSystem::AttachShapesRecursively(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale)
{
    for (uint32 type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            AttachShape(bodyComponent, static_cast<CollisionShapeComponent*>(entity->GetComponent(type, i)), scale);
        }
    }

    const int32 childrenCount = entity->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        if (child->GetComponent(Component::DYNAMIC_BODY_COMPONENT) || child->GetComponent(Component::STATIC_BODY_COMPONENT))
        {
            continue;
        }

        AttachShapesRecursively(child, bodyComponent, scale);
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

        // It's user's responsibility to setup drivable surfaces to work with vehicles
        // For simplicity do it for every landscape by default
        if (component->GetType() == Component::HEIGHT_FIELD_SHAPE_COMPONENT)
        {
            vehiclesSubsystem->SetupDrivableSurface(component);
        }
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

                // Update local pose for nested shapes

                CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                if (shapeComponent->GetEntity() != e)
                {
                    PhysicsSystemDetail::UpdateShapeLocalPose(shape, shapeComponent->GetEntity()->GetLocalTransform());
                }

                // Update geometry scale

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

void PhysicsSystem::ScheduleUpdate(CharacterControllerComponent* component)
{
    characterControllerComponentsUpdatePending.insert(component);
}

bool PhysicsSystem::Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback)
{
    using namespace physx;

    return physicsScene->raycast(PhysicsMath::Vector3ToPxVec3(origin), PhysicsMath::Vector3ToPxVec3(Normalize(direction)),
                                 static_cast<PxReal>(distance), callback);
}

PhysicsVehiclesSubsystem* PhysicsSystem::GetVehiclesSystem()
{
    return vehiclesSubsystem;
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

        // Recalculate mass
        // Ignore vehicles, VehiclesSubsystem is responsible for setting correct values

        Entity* entity = bodyComponent->GetEntity();
        if (entity->GetComponent(Component::VEHICLE_CAR_COMPONENT) == nullptr &&
            entity->GetComponent(Component::VEHICLE_TANK_COMPONENT) == nullptr)
        {
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
    }

    for (CharacterControllerComponent* controllerComponent : characterControllerComponentsUpdatePending)
    {
        physx::PxController* controller = controllerComponent->controller;
        if (controller != nullptr)
        {
            // Update geometry if needed
            if (controllerComponent->geometryChanged)
            {
                if (controllerComponent->GetType() == Component::BOX_CHARACTER_CONTROLLER_COMPONENT)
                {
                    BoxCharacterControllerComponent* boxComponent = static_cast<BoxCharacterControllerComponent*>(controllerComponent);
                    physx::PxBoxController* boxController = static_cast<physx::PxBoxController*>(controller);

                    boxController->setHalfHeight(boxComponent->GetHalfHeight());
                    boxController->setHalfForwardExtent(boxComponent->GetHalfForwardExtent());
                    boxController->setHalfSideExtent(boxComponent->GetHalfSideExtent());
                }
                else if (controllerComponent->GetType() == Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT)
                {
                    CapsuleCharacterControllerComponent* capsuleComponent = static_cast<CapsuleCharacterControllerComponent*>(controllerComponent);
                    physx::PxCapsuleController* capsuleController = static_cast<physx::PxCapsuleController*>(controller);

                    capsuleController->setRadius(capsuleComponent->GetRadius());
                    capsuleController->setHeight(capsuleComponent->GetHeight());
                }
                controllerComponent->geometryChanged = false;
            }

            // Teleport if neeeded
            if (controllerComponent->teleported)
            {
                controller->setPosition(PhysicsMath::Vector3ToPxExtendedVec3(controllerComponent->teleportDestination));
                controllerComponent->teleported = false;
            }
        }
    }

    collisionComponentsUpdatePending.clear();
    physicsComponensUpdatePending.clear();
    characterControllerComponentsUpdatePending.clear();
}

void PhysicsSystem::MoveCharacterControllers(float32 timeElapsed)
{
    for (CharacterControllerComponent* controllerComponent : characterControllerComponents)
    {
        physx::PxController* controller = controllerComponent->controller;
        if (controller != nullptr)
        {
            // Apply movement
            physx::PxControllerCollisionFlags collisionFlags;
            if (controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Flying)
            {
                collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(controllerComponent->totalDisplacement), 0.0f, timeElapsed, physx::PxControllerFilters());
            }
            else
            {
                DVASSERT(controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Walking);

                // Ignore displacement along z axis
                Vector3 displacement = controllerComponent->totalDisplacement;
                displacement.z = 0.0f;

                // Apply gravity
                displacement += PhysicsMath::PxVec3ToVector3(physicsScene->getGravity()) * timeElapsed;

                collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(displacement), 0.0f, timeElapsed, physx::PxControllerFilters());
            }

            controllerComponent->grounded = (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
            controllerComponent->totalDisplacement = Vector3::Zero;

            // Sync entity's transform

            Entity* entity = controllerComponent->GetEntity();
            DVASSERT(entity != nullptr);

            Matrix4 transform = entity->GetLocalTransform();
            transform.SetTranslationVector(PhysicsMath::PxExtendedVec3ToVector3(controller->getPosition()));
            entity->SetLocalTransform(transform);
        }
    }
}

} // namespace DAVA
