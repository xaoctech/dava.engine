#include "Systems/ShooterMirroredCharacterSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>

#include <Physics/CapsuleCharacterControllerComponent.h>
#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMirroredCharacterSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterMirroredCharacterSystem>::Begin()[M::Tags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterMirroredCharacterSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 18.0f)]
    .End();
}

ShooterMirroredCharacterSystem::ShooterMirroredCharacterSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, 0)
    , ccts(scene->AquireComponentGroup<ShooterMirroredCharacterComponent, ShooterMirroredCharacterComponent>())
{
}

void ShooterMirroredCharacterSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    CharacterMirrorsSingleComponent* mirrorsSingleComponent = GetScene()->GetSingletonComponent<CharacterMirrorsSingleComponent>();
    DVASSERT(mirrorsSingleComponent != nullptr);

    for (ShooterMirroredCharacterComponent* mirrorCctComponent : ccts->components)
    {
        DAVA::Entity* mirror = mirrorsSingleComponent->GetMirrorForCharacter(mirrorCctComponent->GetEntity());
        DVASSERT(mirror != nullptr);

        TransformComponent* cctTransformComponent = mirrorCctComponent->GetEntity()->GetComponent<TransformComponent>();
        TransformComponent* mirrorTrasnformComponent = mirror->GetComponent<TransformComponent>();

        if (!mirrorCctComponent->GetMirrorIsMaster())
        {
            mirrorTrasnformComponent->SetLocalTransform(cctTransformComponent->GetPosition(), cctTransformComponent->GetRotation(), cctTransformComponent->GetScale());
        }
        else
        {
            // TODO: Calc 0.3 offset using CCT params
            cctTransformComponent->SetLocalTransform(mirrorTrasnformComponent->GetPosition() + Vector3(0.0f, 0.0f, 0.3f), mirrorTrasnformComponent->GetRotation(), mirrorTrasnformComponent->GetScale());
        }
    }
}

void ShooterMirroredCharacterSystem::PrepareForRemove()
{
}
