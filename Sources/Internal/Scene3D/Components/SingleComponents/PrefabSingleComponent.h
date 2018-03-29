#pragma once
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class PrefabComponent;
class PrefabSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(PrefabSingleComponent, SingletonComponent);

    UnorderedSet<PrefabComponent*> changedPrefabComponent;
};
} // namespace DAVA
