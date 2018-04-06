#pragma once

#include "Base/Set.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class Entity;
class VisualScriptComponent;

/** Single component providing information about visual scripts on scene. */
class VisualScriptSingleComponent : public SingletonComponent
{
public:
    /** Vector of current collisions */
    Set<VisualScriptComponent*> compiledScripts;
};
}
