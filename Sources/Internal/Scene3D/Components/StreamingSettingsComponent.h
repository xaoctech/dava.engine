#pragma once

#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/Reflection.h"
#include "Math/Color.h"
#include "Base/Any.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \brief Add this component if you want to mark object as special purpose object for streaming system. 
    unloadSolidAngle defines the angle when object will be unloaded. 
    For some objects it's requered to add this component to avoid their streaming. 
    It's required for Landscape, global reflection probe, post fx component and probably some others that is required all the time during level.
    
    Also it can be used for some specific purpose objects that should remain visible even when others objects with same properties will be unloaded by streaming system. 
*/

class StreamingSettingsComponent : public Component
{
protected:
    ~StreamingSettingsComponent() override = default;

public:
    StreamingSettingsComponent() = default;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    float32 unloadSolidAngle = 0.0f;

    DAVA_VIRTUAL_REFLECTION(StreamingSettingsComponent, Component);
};
}
