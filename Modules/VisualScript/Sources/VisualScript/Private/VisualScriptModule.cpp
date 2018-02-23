#include "VisualScript/VisualScriptModule.h"
#include "VisualScript/VisualScriptPin.h"
#include "VisualScript/VisualScriptNode.h"
#include "VisualScript/VisualScriptEvents.h"
#include "VisualScript/VisualScriptPin.h"
#include "Scene3D/Components/VisualScriptComponent.h"
#include "Scene3D/Systems/VisualScriptSystem.h"
#include "UI/VisualScript/Private/UIVisualScriptEvents.h"
#include "UI/VisualScript/UIVisualScriptComponent.h"
#include "UI/VisualScript/UIVisualScriptSystem.h"

#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/UIControlSystem.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptModule)
{
    ReflectionRegistrator<VisualScriptModule>::Begin()
    .End();
}

/*
class WaitNode
{
public:
    DAVA_REFLECTION(WaitNode);
    
    VisualScriptFunctionResult Exec(float32 timeElapsed, float32 timeLimit)
    {
        timer += timeElapsed;
        if (timer >= timeLimit)
        {
            timer -= timeLimit;
            return FIRE_EXEC_OUT;
        }
        
        return NOT_FIRE_EXEC_OUT;
    }
    float32 timer = 0.0f;
};
    

DAVA_REFLECTION_IMPL(WaitNode)
{
    static WaitNode waitNode;
    
    ReflectionRegistrator<WaitNode>::Begin()
    .ConstructorByPointer()
    .Method("Exec", [waitNode] (float32 te, float32 tl) {waitNode.Exec(te, tl);})
    .End();
}
*/

VisualScriptModule::VisualScriptModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptSystem);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptPin);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptForNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptFunctionNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptWhileNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptBranchNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptEventNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptGetVarNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptSetVarNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptAnotherScriptNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptGetMemberNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptSetMemberNode);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ButtonClickEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TimerEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EntitiesCollideEvent);

    //DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaitNode);

    // UI
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIVisualScriptComponent);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIInitEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIReleaseEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIProcessEvent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIEventProcessEvent);
}

void VisualScriptModule::Init()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    ComponentManager* cm = context->componentManager;
    cm->RegisterComponent<UIVisualScriptComponent>();

    UIControlSystem* cs = context->uiControlSystem;
    cs->AddSystem(std::make_unique<UIVisualScriptSystem>(), cs->GetStyleSheetSystem());
}

void VisualScriptModule::Shutdown()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    UIControlSystem* cs = context->uiControlSystem;
    cs->RemoveSystem(cs->GetSystem<UIVisualScriptSystem>());
}
}
