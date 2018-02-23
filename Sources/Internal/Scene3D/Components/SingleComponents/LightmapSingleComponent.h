#pragma once

#include "Base/Vector.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class Entity;
class LightmapComponent;
class LightmapSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(LightmapSingleComponent, SingletonComponent);

    Vector<LightmapComponent*> shadowReceiverParamChanged;
    Vector<LightmapComponent*> shadowCasterParamChanged;
    Vector<LightmapComponent*> lightmapSizeChanged;

    void Clear();
    void EraseComponent(LightmapComponent* component);
};
}
