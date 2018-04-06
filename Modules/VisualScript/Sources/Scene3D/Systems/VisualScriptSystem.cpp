#include "Scene3D/Systems/VisualScriptSystem.h"
#include "Scene3D/Components/SingleComponents/VisualScriptSingleComponent.h"
#include "Scene3D/Components/VisualScriptComponent.h"
#include "Scene3D/Entity.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptEvents.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SingleComponents/CollisionSingleComponent.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
VisualScriptSystem::VisualScriptSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void VisualScriptSystem::SetScene(Scene* scene)
{
    visualScriptSingleComponent = new VisualScriptSingleComponent();
    scene->AddSingletonComponent(visualScriptSingleComponent);

    collisionSingleComponent = scene->GetSingletonComponent<CollisionSingleComponent>();
}

void VisualScriptSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("VisualScriptSystem::Process");

    for (VisualScriptComponent* scriptComponent : visualScriptSingleComponent->compiledScripts)
    {
        if (scriptComponent && scriptComponent->GetScript())
        {
            CheckAndInsertScript(scriptComponent);
        }
    }

    if (!processVisualScripts.empty())
    {
        for (CollisionInfo& collisionInfo : collisionSingleComponent->collisions)
        {
            VisualScriptComponent* firstScript = collisionInfo.first->GetComponent<VisualScriptComponent>();
            VisualScriptComponent* secondScript = collisionInfo.second->GetComponent<VisualScriptComponent>();

            EntitiesCollideEvent collideEvent;
            collideEvent.collisionInfo = &collisionInfo;

            if (collisionVisualScripts.find(firstScript) != collisionVisualScripts.end())
            {
                VisualScript* script = firstScript->GetScript();
                if (script)
                {
                    script->Execute(FastName("EntitiesCollideEvent"), Reflection::Create(&collideEvent));
                }
            }

            if (collisionVisualScripts.find(secondScript) != collisionVisualScripts.end())
            {
                VisualScript* script = secondScript->GetScript();
                if (script)
                {
                    script->Execute(FastName("EntitiesCollideEvent"), Reflection::Create(&collideEvent));
                }
            }
        }
    }

    if (!processVisualScripts.empty())
    {
        ProcessEvent processEvent;
        processEvent.frameDelta = timeElapsed;
        Reflection processEventRef = Reflection::Create(&processEvent);

        for (VisualScriptComponent* scriptComponent : processVisualScripts)
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

void VisualScriptSystem::AddEntity(Entity* entity)
{
    VisualScriptComponent* component = entity->GetComponent<VisualScriptComponent>();
    if (component)
    {
        CheckAndInsertScript(component);
    }
}

void VisualScriptSystem::RemoveEntity(Entity* entity)
{
    VisualScriptComponent* component = entity->GetComponent<VisualScriptComponent>();
    if (component)
    {
        processVisualScripts.erase(component);
        collisionVisualScripts.erase(component);
    }
}

void VisualScriptSystem::PrepareForRemove()
{
    processVisualScripts.clear();
    collisionVisualScripts.clear();
}

void VisualScriptSystem::CheckAndInsertScript(VisualScriptComponent* scriptComponent)
{
    VisualScript* script = scriptComponent->GetScript();
    if (!script)
    {
        return;
    }

    // Collisions
    if (script->HasEventNode(FastName("EntitiesCollideEvent")))
    {
        collisionVisualScripts.insert(scriptComponent);
    }
    else
    {
        collisionVisualScripts.erase(scriptComponent);
    }

    // Process
    if (script->HasEventNode(FastName("ProcessEvent")))
    {
        processVisualScripts.insert(scriptComponent);
    }
    else
    {
        processVisualScripts.erase(scriptComponent);
    }
}
}
