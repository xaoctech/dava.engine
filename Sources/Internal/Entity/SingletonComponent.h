#pragma once

#include "Component.h"
#include "Scene3D/Entity.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class SingletonComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override final;
    virtual ~SingletonComponent(){};

protected:
    DAVA_VIRTUAL_REFLECTION(SingletonComponent, Component);
};
}
