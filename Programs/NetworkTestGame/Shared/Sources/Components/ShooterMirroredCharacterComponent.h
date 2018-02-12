#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

class ShooterMirroredCharacterComponent final : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterMirroredCharacterComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetMirrorIsMaster(bool value);
    bool GetMirrorIsMaster() const;

private:
    bool mirrorIsMaster = false;
};
