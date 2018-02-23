#include "VisualScript/Nodes/VisualWaitNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
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
}