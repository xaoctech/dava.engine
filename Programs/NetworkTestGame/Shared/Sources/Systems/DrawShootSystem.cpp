#include "DrawShootSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Scene3D/Components/ComponentHelpers.h>

#include "Scene3D/Components/TransformComponent.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterStateComponent.h"
#include "Components/ShooterAimComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <NetworkCore/NetworkCoreUtils.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(DrawShootSystem)
{
    ReflectionRegistrator<DrawShootSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &DrawShootSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::NORMAL, 1001.0f)]
    .End();
}

DrawShootSystem::DrawShootSystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    stateGroup = scene->AquireComponentGroup<ShooterStateComponent, ShooterStateComponent, ShooterRoleComponent>();
    renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
}

void DrawShootSystem::Process(DAVA::float32 timeElapsed)
{
    for (ShooterStateComponent* stateComponent : stateGroup->components)
    {
        if (stateComponent->raycastAttackFrameId > stateComponent->lastRaycastAttackFrameId)
        {
            stateComponent->lastRaycastAttackFrameId = stateComponent->raycastAttackFrameId;
            Entity* aimingEntity = stateComponent->GetEntity();
            Entity* weaponBarrelEntity = aimingEntity->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
            const Vector3& shootStart = weaponBarrelEntity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();

            Vector3 aimRayOrigin;
            Vector3 aimRayDirection;
            Vector3 aimRayEnd;
            Entity* aimRayEndEntity;
            ShooterAimComponent* aimComponent = aimingEntity->GetComponent<ShooterAimComponent>();
            GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE, aimRayOrigin, aimRayDirection, aimRayEnd, &aimRayEndEntity);

            renderHelper->DrawLine(shootStart, aimRayEnd, Color::White);
        }
    }
}