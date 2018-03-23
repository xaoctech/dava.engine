#pragma once

#include "Entity/SingletonComponent.h"

namespace DAVA
{
class RenderObject;
class Entity;
class RenderObjectSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(RenderObjectSingleComponent, SingletonComponent);

    UnorderedMap<RenderObject*, Entity*> changedRenderObjects;
};
} // namespace DAVA