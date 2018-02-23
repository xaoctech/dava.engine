#pragma once

#include "Base/Vector.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class Entity;
class VTSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(VTSingleComponent, SingletonComponent);

public:
    Vector<Entity*> vtDecalChanged;
    Vector<Entity*> vtSplineChanged;

    void Clear();
    void EraseEntity(Entity* entity);
};
}
