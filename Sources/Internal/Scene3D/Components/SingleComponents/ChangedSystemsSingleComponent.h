#pragma once

#include "Base/Vector.h"
#include "Entity/SortedEntityContainer.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class SceneSystem;

class ChangedSystemsSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(ChangedSystemsSingleComponent, SingletonComponent);

    Vector<SceneSystem*> addedSystems;
    Vector<SceneSystem*> removedSystems;

    void Clear() override;
};
}
