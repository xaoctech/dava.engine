#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsModule.h"
#include "Physics/StaticBodyComponent.h"
#include "Physics/DynamicBodyComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/CharacterControllerComponent.h"
#include "Physics/CapsuleCharacterControllerComponent.h"
#include "Physics/BoxCharacterControllerComponent.h"
#include "Physics/CapsuleShapeComponent.h"
#include "Physics/BoxShapeComponent.h"

#include <Engine/Engine.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
namespace PhysicsUtils
{
PhysicsComponent* GetBodyComponent(Entity* entity)
{
    PhysicsComponent* resultComponent = entity->GetComponent<StaticBodyComponent>();
    if (resultComponent == nullptr)
    {
        resultComponent = entity->GetComponent<DynamicBodyComponent>();
    }

    return resultComponent;
}

Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity)
{
    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();

    Vector<CollisionShapeComponent*> shapes;
    for (const Type* shapeType : shapeComponents)
    {
        const uint32 shapesCount = entity->GetComponentCount(shapeType);
        if (shapesCount > 0)
        {
            for (uint32 i = 0; i < shapesCount; ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(shapeType, i));
                DVASSERT(component != nullptr);

                shapes.push_back(component);
            }
        }
    }

    return shapes;
}

CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity)
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

Entity* CreateCharacterMirror(CharacterControllerComponent* controllerComponent)
{
    Entity* mirror = new Entity();
    mirror->SetName("Character's mirror");

    DynamicBodyComponent* dynamicBodyComponent = new DynamicBodyComponent();
    dynamicBodyComponent->SetIsKinematic(true);
    mirror->AddComponent(dynamicBodyComponent);

    const float32 scaleFactor = controllerComponent->GetScaleCoeff();

    if (controllerComponent->GetType()->Is<CapsuleCharacterControllerComponent>())
    {
        CapsuleCharacterControllerComponent* capsuleControllerComponent = static_cast<CapsuleCharacterControllerComponent*>(controllerComponent);

        CapsuleShapeComponent* capsuleShapeComponent = new CapsuleShapeComponent();
        capsuleShapeComponent->SetHalfHeight(scaleFactor * capsuleControllerComponent->GetHeight() * 0.5f);
        capsuleShapeComponent->SetRadius(scaleFactor * capsuleControllerComponent->GetRadius());
        capsuleShapeComponent->SetLocalPose(Matrix4::MakeRotation(Vector3::UnitY, PI / 2.0f) * Matrix4::MakeTranslation(Vector3::UnitZ * (capsuleControllerComponent->GetContactOffset() + capsuleControllerComponent->GetRadius() + capsuleControllerComponent->GetHeight() * 0.5f)));
        mirror->AddComponent(capsuleShapeComponent);
    }
    else if (controllerComponent->GetType()->Is<BoxCharacterControllerComponent>())
    {
        BoxCharacterControllerComponent* boxControllerComponent = static_cast<BoxCharacterControllerComponent*>(controllerComponent);

        BoxShapeComponent* boxShapeComponent = new BoxShapeComponent();
        boxShapeComponent->SetHalfSize(Vector3(scaleFactor * boxControllerComponent->GetHalfSideExtent(),
                                               scaleFactor * boxControllerComponent->GetHalfForwardExtent(),
                                               scaleFactor * boxControllerComponent->GetHalfHeight()));
        boxShapeComponent->SetLocalPose(Matrix4::MakeRotation(Vector3::UnitY, PI / 2.0f) * Matrix4::MakeTranslation(Vector3::UnitZ * (controllerComponent->GetContactOffset() + boxControllerComponent->GetHalfHeight())));
        mirror->AddComponent(boxShapeComponent);
    }
    else
    {
        DVASSERT(false, "No mirror for this controller component.");
    }

    return mirror;
}
}
}
