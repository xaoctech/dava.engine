#include "NetworkDebugResimulationDrawSystem.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderSystem.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDebugResimulationDrawSystem)
{
    ReflectionRegistrator<NetworkDebugResimulationDrawSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkDebugResimulationDrawSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 13.3f)] // should be after NetworkResimulationSystem
    .Method("Process", &NetworkDebugResimulationDrawSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 0.1f)]
    .End();
}

NetworkDebugResimulationDrawSystem::NetworkDebugResimulationDrawSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    networkResimulationSingleComponent = scene->GetSingleComponentForRead<NetworkResimulationSingleComponent>(this);
    networkEntitiesSingleComponent = scene->GetSingleComponentForRead<NetworkEntitiesSingleComponent>(this);
    networkTimeSingleComponent = scene->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
}

void NetworkDebugResimulationDrawSystem::Process(float32 timeElapsed)
{
    const auto& debugDraw = networkResimulationSingleComponent->debugDraw;
    if (!debugDraw.enabled || GetPrimaryWindow() == nullptr)
    {
        return;
    }

    const float32 fadeTime = debugDraw.fadeTimeInSeconds;

    for (auto it = elements.begin(); it != elements.end();)
    {
        Entity* entity = networkEntitiesSingleComponent->FindByID(it->first);

        DebugInfo& info = it->second;

        if (info.fadeTime > fadeTime)
        {
            if (entity != nullptr)
            {
                // restore color
            }
            it = elements.erase(it);
            continue;
        }

        if (entity != nullptr)
        {
            // color stuff
        }

        info.fadeTime += timeElapsed;
        ++it;
    }

    if (debugDraw.drawBbs)
    {
        const auto& bbResimulation = networkResimulationSingleComponent->boundingBoxResimulation;
        uint32 currentFrameId = networkTimeSingleComponent->GetFrameId();

        UnorderedMap<NetworkID, AABBox3> boxes;

        for (uint32 frameId = currentFrameId - bbResimulation.HistorySize; frameId < currentFrameId; ++frameId)
        {
            for (const auto& p : bbResimulation.GetHistoryForFrame(frameId))
            {
                boxes[p.first].AddAABBox(p.second);
            }
        }

        for (const auto& p : boxes)
        {
            Color boxColor = Color::Blue;
            boxColor.a = 0.4f;

            auto it = elements.find(p.first);

            if (it != elements.end())
            {
                float32 red = Clamp(1.f - it->second.fadeTime / fadeTime, 0.f, 1.f);
                boxColor.b = Clamp(boxColor.b - red, 0.f, 1.f);
                boxColor.r = red;
            }

            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(p.second, boxColor, RenderHelper::eDrawType::DRAW_SOLID_DEPTH);
        }
    }
}

void NetworkDebugResimulationDrawSystem::ProcessFixed(float32 timeElapsed)
{
    const auto& debugDraw = networkResimulationSingleComponent->debugDraw;
    if (!debugDraw.enabled || GetPrimaryWindow() == nullptr)
    {
        return;
    }

    const float32 fadeTime = debugDraw.fadeTimeInSeconds;

    for (const auto& p : networkResimulationSingleComponent->GetResimulatingEntities())
    {
        NetworkID id = NetworkCoreUtils::GetEntityId(p.first);

        DVASSERT(id != NetworkID::INVALID && id != NetworkID::SCENE_ID);

        elements[id].fadeTime = 0.f;
    }
}
} // namespace DAVA
