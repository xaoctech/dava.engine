#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class PostEffectDebugComponent : public Component
{
public:
    PostEffectDebugComponent() = default;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetDrawHDRTarget(bool draw);
    bool GetDrawHDRTarget() const;
    void SetDrawLuminance(bool draw);
    bool GetDrawLuminance() const;
    void SetDebugRectOffset(const Vector2& rect);
    Vector2 GetDebugRectOffset() const;
    void SetDebugRectSize(const int32& size);
    int32 GetDebugRectSize();
    void SetDrawAdaptation(bool draw);
    bool GetDrawAdaptataion() const;
    void SetDrawHistogram(bool draw);
    bool GetDrawHistogram() const;
    void SetDrawBloom(bool draw);
    bool GetDrawBloom() const;

    bool GetLightMeterEnabled() const;
    void SetLightMeterEnabled(bool value);

    bool GetLightMeterMaskEnabled() const;
    void SetLightMeterMaskEnabled(bool value);

    int32 GetTAASampleIndex() const;
    void SetTAASampleIndex(int32 value);

private:
    bool drawHDRTarget = false;
    bool drawLuminance = false;
    bool drawAdaptation = false;
    bool drawHistogram = false;
    bool drawBloom = false;
    bool enableLightMeter = false;
    bool drawLightMeterMask = false;
    Vector2 debugRectOffset = Vector2{ 5.f, 5.f };
    int32 debugRectSize = 128;

public:
    DAVA_VIRTUAL_REFLECTION(PostEffectDebugComponent, Component);
};
};