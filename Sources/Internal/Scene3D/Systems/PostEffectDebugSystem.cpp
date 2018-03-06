#include "Scene3D/Systems/PostEffectDebugSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/PostEffectDebugComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "UI/UIEvent.h"

namespace DAVA
{
PostEffectDebugSystem::PostEffectDebugSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<PostEffectDebugComponent>())
{
    if (scene)
    {
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
    }
}

void PostEffectDebugSystem::Process(float32 timeElapsed)
{
}

void PostEffectDebugSystem::ImmediateEvent(Component* component, uint32 event)
{
    if ((component->GetType() == Type::Instance<PostEffectDebugComponent>()) && (event == EventSystem::POSTEFFECT_DEBUG_CHANGED))
    {
        PostEffectDebugComponent* debugComponent = static_cast<PostEffectDebugComponent*>(component);
        PostEffectRenderer::DebugRenderer* debugRenderer = GetScene()->GetRenderSystem()->GetPostEffectRenderer()->GetDebugRenderer();
        debugRenderer->drawHDRTarget = debugComponent->GetDrawHDRTarget();
        debugRenderer->drawLuminance = debugComponent->GetDrawLuminance();
        debugRenderer->debugRectOffset = Size2i(int32(debugComponent->GetDebugRectOffset().dx), int32(debugComponent->GetDebugRectOffset().dy));
        debugRenderer->debugRectSize = debugComponent->GetDebugRectSize();
        debugRenderer->drawAdaptation = debugComponent->GetDrawAdaptataion();
        debugRenderer->drawHistogram = debugComponent->GetDrawHistogram();
        debugRenderer->drawBloom = debugComponent->GetDrawBloom();
        debugRenderer->pointOfInterest = lastPointOfInterest;
        debugRenderer->enableLightMeter = debugComponent->GetLightMeterEnabled();
        debugRenderer->drawLightMeterMask = debugComponent->GetLightMeterMaskEnabled();
    }
}

void PostEffectDebugSystem::AddEntity(Entity* entity)
{
    Component* c = entity->GetComponent<PostEffectDebugComponent>();
    if (c)
    {
        postEffectDebugComponent = c;
        ImmediateEvent(c, EventSystem::POSTEFFECT_DEBUG_CHANGED);
    }
}

void PostEffectDebugSystem::RemoveEntity(Entity* entity)
{
    Component* c = entity->GetComponent<PostEffectDebugComponent>();
    if (c == postEffectDebugComponent)
    {
        postEffectDebugComponent = nullptr;
    }
}

void PostEffectDebugSystem::PrepareForRemove()
{
}

bool PostEffectDebugSystem::Input(UIEvent* uie)
{
    if ((uie->phase == UIEvent::Phase::MOVE) && (postEffectDebugComponent != nullptr))
    {
        lastPointOfInterest = uie->point;
        ImmediateEvent(postEffectDebugComponent, EventSystem::POSTEFFECT_DEBUG_CHANGED);
    }

    return false;
}
};
