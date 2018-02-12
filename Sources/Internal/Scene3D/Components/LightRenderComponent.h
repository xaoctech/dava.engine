#pragma once

#include "Entity/Component.h"
#include "Render/Highlevel/LightRenderObject.h"

namespace DAVA
{
class LightRenderComponent : public Component
{
public:
    LightRenderComponent();

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    Component* Clone(Entity* toEntity) override;

    void SetLight(Light* l);

    LightRenderObject* GetRenderObject() const;

private:
    Light* light = nullptr;
    ScopedPtr<LightRenderObject> object;

    DAVA_VIRTUAL_REFLECTION(LightRenderComponent, Component);
};
};
