#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class VisualScriptContext
{
public:
    VisualScriptContext();
    ~VisualScriptContext();

    Map<FastName, Any>& GetUserVariables();

private:
    Entity* entity;
    Map<FastName, Any> userVariables;
};

//
inline Map<FastName, Any>& VisualScriptContext::GetUserVariables()
{
    return userVariables;
}
}
