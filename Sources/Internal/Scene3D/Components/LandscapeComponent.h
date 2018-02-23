#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class LandscapeComponent : public Component
{
protected:
    virtual ~LandscapeComponent();

public:
    LandscapeComponent(Landscape* landscape_ = nullptr);

    void SetLandscape(Landscape* landscape_);
    Landscape* GetLandscape() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    Landscape* landscape = nullptr;

    DAVA_VIRTUAL_REFLECTION(LandscapeComponent, Component);
};
}
