#pragma once

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class VisualScriptContext final
{
public:
    VisualScriptContext();
    ~VisualScriptContext();

    Map<FastName, Any>& GetUserVariables();

private:
    Entity* entity = nullptr;
    Map<FastName, Any> userVariables;
};

inline Map<FastName, Any>& VisualScriptContext::GetUserVariables()
{
    return userVariables;
}
}
