#pragma once

#include <Base/BaseTypes.h>
#include <Math/Vector.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

class ExplosionEffectComponent : public DAVA::Component
{
    DAVA_VIRTUAL_REFLECTION(ExplosionEffectComponent, DAVA::Component);

public:
    ExplosionEffectComponent();
    ~ExplosionEffectComponent() override;

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::float32 duration = 1.f;
    int effectType = 0;
    bool effectStarted = false;
    bool linkedEffect = false;
};
