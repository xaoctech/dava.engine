#pragma once

#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/RenderObject.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class DebugRenderComponent : public Component
{
protected:
    virtual ~DebugRenderComponent();

public:
    DebugRenderComponent(RenderObject* debugRenderObject = nullptr);

    void SetRenderObject(RenderObject* object);
    RenderObject* GetRenderObject() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    RenderObject* renderObject = nullptr;

    DAVA_VIRTUAL_REFLECTION(DebugRenderComponent, Component);
};
};
