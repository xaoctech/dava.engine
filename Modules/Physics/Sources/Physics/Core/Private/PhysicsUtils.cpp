#include "Physics/Core/PhysicsUtils.h"
#include "Physics/PhysicsModule.h"
#include "Physics/Core/Private/PhysicsMath.h"
#include "Physics/Core/StaticBodyComponent.h"
#include "Physics/Core/DynamicBodyComponent.h"
#include "Physics/Core/CollisionShapeComponent.h"
#include "Physics/Controllers/CharacterControllerComponent.h"
#include "Physics/Controllers/CapsuleCharacterControllerComponent.h"
#include "Physics/Controllers/BoxCharacterControllerComponent.h"
#include "Physics/Core/CapsuleShapeComponent.h"
#include "Physics/Core/BoxShapeComponent.h"

#include <Engine/Engine.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SkeletonComponent.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
static void SetShapeGeometryScale(physx::PxShape* shape, const Vector3& scale)
{
    DVASSERT(shape != nullptr);

    const physx::PxVec3 newPxScale = PhysicsMath::Vector3ToPxVec3(scale);

    const physx::PxGeometryHolder geometryHolder = shape->getGeometry();
    const physx::PxGeometryType::Enum geometryType = geometryHolder.getType();

    if (geometryType == physx::PxGeometryType::eTRIANGLEMESH)
    {
        physx::PxTriangleMeshGeometry geometry;
        const bool extracted = shape->getTriangleMeshGeometry(geometry);
        DVASSERT(extracted);

        if (geometry.scale.scale != newPxScale)
        {
            geometry.scale.scale = newPxScale;
            shape->setGeometry(geometry);
        }
    }
    else if (geometryType == physx::PxGeometryType::eCONVEXMESH)
    {
        physx::PxConvexMeshGeometry geometry;
        const bool extracted = shape->getConvexMeshGeometry(geometry);
        DVASSERT(extracted);

        if (geometry.scale.scale != newPxScale)
        {
            geometry.scale.scale = newPxScale;
            shape->setGeometry(geometry);
        }
    }
}

PhysicsComponent* PhysicsUtils::GetBodyComponent(Entity* entity)
{
    PhysicsComponent* resultComponent = entity->GetComponent<StaticBodyComponent>();
    if (resultComponent == nullptr)
    {
        resultComponent = entity->GetComponent<DynamicBodyComponent>();
    }

    return resultComponent;
}

void PhysicsUtils::ForEachShapeComponent(Entity* entity, Function<void(CollisionShapeComponent*)> callback)
{
    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();

    for (const Type* shapeType : shapeComponents)
    {
        const uint32 shapesCount = entity->GetComponentCount(shapeType);
        if (shapesCount > 0)
        {
            for (uint32 i = 0; i < shapesCount; ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(shapeType, i));
                DVASSERT(component != nullptr);

                callback(component);
            }
        }
    }
}

Vector<CollisionShapeComponent*> PhysicsUtils::GetShapeComponents(Entity* entity)
{
    Vector<CollisionShapeComponent*> shapes;
    ForEachShapeComponent(entity, [&shapes](CollisionShapeComponent* component) { shapes.push_back(component); });
    return shapes;
}

CharacterControllerComponent* PhysicsUtils::GetCharacterControllerComponent(Entity* entity)
{
    DVASSERT(entity != nullptr);

    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(module != nullptr);

    const Vector<const Type*>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (const Type* controllerType : characterControllerComponents)
    {
        CharacterControllerComponent* component = static_cast<CharacterControllerComponent*>(entity->GetComponent(controllerType));
        if (component != nullptr)
        {
            return component;
        }
    }

    return nullptr;
}

void PhysicsUtils::CopyTransformToPhysics(Entity* entity)
{
    DVASSERT(entity != nullptr);

    // Only root entities can contains physics component
    DVASSERT(entity->GetParent() == entity->GetScene());

    auto updateActorAndShapes = [](const Entity* actorEntity, PhysicsComponent* component)
    {
        DVASSERT(actorEntity != nullptr);
        DVASSERT(component != nullptr);

        physx::PxRigidActor* actor = component->GetPxActor();
        if (actor != nullptr)
        {
            const TransformComponent* transformComponent = actorEntity->GetComponent<TransformComponent>();
            DVASSERT(transformComponent != nullptr);

            const Transform& transform = transformComponent->GetLocalTransform();

            SetActorTransform(actor, transform.GetTranslation(), transform.GetRotation());
            component->currentScale = transform.GetScale();

            physx::PxU32 shapesCount = actor->getNbShapes();
            for (physx::PxU32 i = 0; i < shapesCount; ++i)
            {
                physx::PxShape* shape = nullptr;
                actor->getShapes(&shape, 1, i);

                DVASSERT(shape != nullptr);

                // Update geometry scale
                SetShapeGeometryScale(shape, transform.GetScale());
            }
        }
    };

    CharacterControllerComponent* controllerComponent = PhysicsUtils::GetCharacterControllerComponent(entity);
    PhysicsComponent* bodyComponent = PhysicsUtils::GetBodyComponent(entity);

    if (bodyComponent != nullptr)
    {
        updateActorAndShapes(entity, bodyComponent);
    }

    if (controllerComponent != nullptr)
    {
        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        physx::PxController* pxController = controllerComponent->GetPxController();
        if (pxController != nullptr)
        {
            Vector3 currentPosition = PhysicsMath::PxExtendedVec3ToVector3(pxController->getFootPosition());
            const Vector3& newPosition = transformComponent->GetLocalTransform().GetTranslation();

            if (!FLOAT_EQUAL(currentPosition.x, newPosition.x) ||
                !FLOAT_EQUAL(currentPosition.y, newPosition.y) ||
                !FLOAT_EQUAL(currentPosition.z, newPosition.z))
            {
                pxController->setFootPosition(PhysicsMath::Vector3ToPxExtendedVec3(newPosition));
            }
        }
    }
}

void PhysicsUtils::SyncJointsTransformsWithPhysics(Entity* entity)
{
    DVASSERT(entity != nullptr);

    ForEachShapeComponent(entity, [entity](CollisionShapeComponent* shapeComponent)
                          {
                              DVASSERT(shapeComponent != nullptr);

                              const FastName& jointName = shapeComponent->GetJointName();
                              if (jointName.IsValid() && jointName.size() > 0)
                              {
                                  SkeletonComponent* skeletonComponent = entity->GetComponent<SkeletonComponent>();
                                  if (skeletonComponent != nullptr)
                                  {
                                      uint32 index = skeletonComponent->GetJointIndex(jointName);

                                      if (shapeComponent->GetJointSyncDirection() == CollisionShapeComponent::JointSyncDirection::FromPhysics)
                                      {
                                          skeletonComponent->SetJointPosition(index, shapeComponent->GetLocalPosition());
                                          skeletonComponent->SetJointOrientation(index, shapeComponent->GetLocalOrientation());
                                      }
                                      else
                                      {
                                          const JointTransform& jointTransform = skeletonComponent->GetJointObjectSpaceTransform(index);
                                          shapeComponent->SetLocalPosition(jointTransform.GetPosition() + jointTransform.GetOrientation().ApplyToVectorFast(shapeComponent->GetJointOffset()));
                                          shapeComponent->SetLocalOrientation(jointTransform.GetOrientation());
                                      }
                                  }
                              }
                          });
}

void PhysicsUtils::SetActorTransform(physx::PxRigidActor* actor, const Vector3& position, const Quaternion& orientation)
{
    DVASSERT(actor != nullptr);

    const physx::PxVec3 newPxPosition = PhysicsMath::Vector3ToPxVec3(position);
    const physx::PxQuat newPxOrientation = PhysicsMath::QuaternionToPxQuat(orientation);
    const physx::PxTransform& pxCurrentTransform = actor->getGlobalPose();

    // Avoid updating global pose if it hasn't really changed to not wake up the actor for no reason
    if ((pxCurrentTransform.p != newPxPosition) || !(pxCurrentTransform.q == newPxOrientation))
    {
        actor->setGlobalPose(physx::PxTransform(newPxPosition, newPxOrientation));
    }
}

void PhysicsUtils::SetShapeTransform(physx::PxShape* shape, const Vector3& position, const Quaternion& orientation)
{
    DVASSERT(shape != nullptr);

    physx::PxVec3 newPxPosition = PhysicsMath::Vector3ToPxVec3(position);
    physx::PxQuat newPxOrientation = PhysicsMath::QuaternionToPxQuat(orientation);
    shape->setLocalPose(physx::PxTransform(newPxPosition, newPxOrientation));
}
}
