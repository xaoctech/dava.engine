#include "Scene3D/Systems/VisualScriptSystem.h"
#include "Scene3D/Components/VisualScriptComponent.h"
#include "Scene3D/Entity.h"
#include "VisualScript/VisualScriptEvents.h"
#include "VisualScript/VisualScript.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/CollisionSingleComponent.h>

namespace DAVA
{
//DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptSystem)
//{
//    ReflectionRegistrator<VisualScriptSystem>::Begin()[M::Order(50)]
//    .ConstructorByPointer<Scene*>()
//    .End();
//}

VisualScriptSystem::VisualScriptSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void VisualScriptSystem::SetScene(Scene* scene)
{
    collisionSingleComponent = scene->GetSingletonComponent<CollisionSingleComponent>();
}

void VisualScriptSystem::Process(float32 timeElapsed)
{
    //DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_VISUAL_SCRIPT_SYSTEM);

    for (CollisionInfo& collisionInfo : collisionSingleComponent->collisions)
    {
        VisualScriptComponent* firstScript = collisionInfo.first->GetComponent<VisualScriptComponent>();
        VisualScriptComponent* secondScript = collisionInfo.second->GetComponent<VisualScriptComponent>();

        EntitiesCollideEvent collideEvent;
        collideEvent.collisionInfo = &collisionInfo;
        if (firstScript)
        {
            VisualScript* script = firstScript->GetScript();
            if (script)
            {
                script->Execute(FastName("EntitiesCollideEvent"), Reflection::Create(&collideEvent));
            }
        }

        if (secondScript)
        {
            VisualScript* script = secondScript->GetScript();
            if (script)
            {
                script->Execute(FastName("EntitiesCollideEvent"), Reflection::Create(&collideEvent));
            }
        }
    }

    if (!visualScripts.empty())
    {
        ProcessEvent processEvent;
        processEvent.frameDelta = timeElapsed;
        Reflection processEventRef = Reflection::Create(&processEvent);

        for (Entity* e : visualScripts)
        {
            VisualScriptComponent* scriptComponent = e->GetComponent<VisualScriptComponent>();
            if (scriptComponent)
            {
                VisualScript* script = scriptComponent->GetScript();
                if (script)
                {
                    processEvent.component = scriptComponent;
                    script->Execute(FastName("ProcessEvent"), processEventRef);
                }
            }
        }
    }
}

void VisualScriptSystem::AddEntity(Entity* entity)
{
    visualScripts.insert(entity);
    VisualScriptComponent* component = entity->GetComponent<VisualScriptComponent>();
}

void VisualScriptSystem::RemoveEntity(Entity* entity)
{
    visualScripts.erase(entity);
}

void VisualScriptSystem::PrepareForRemove()
{
    visualScripts.clear();
    eventToScripts.clear();
}
}
