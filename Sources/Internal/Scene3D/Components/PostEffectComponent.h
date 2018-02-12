#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class PostEffectComponent : public Component
{
public:
    PostEffectComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SaveToYaml(const FilePath& presetPath, YamlNode* parentNode);
    void LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode);

    void SetUseEv(bool value);
    bool GetUseEv() const;

    void SetAperture(const float32& value);
    float32 GetAperture() const;

    void SetShutterTimeInv(const float32& value);
    float32 GetShutterTimeInv() const;

    void SetISO(const float32& value);
    float32 GetISO() const;

    void SetDynamicRange(const Vector2& value);
    Vector2 GetDynamicRange() const;

    void SetAdaptationRange(const Vector2& value);
    Vector2 GetAdaptationRange() const;

    void SetAdaptationSpeed(const Vector2& value);
    Vector2 GetAdaptationSpeed() const;

    void SetBaseEV(const float32& value);
    float32 GetBaseEV() const;

    void SetBloomColor(const Color& value);
    Color GetBloomColor() const;

    void SetBloomEVC(const float32& value);
    float32 GetBloomEVC() const;

    void SetColorGradingTable(const FilePath&);
    const FilePath& GetColorGradingTable() const;

    void SetHeatmapTable(const FilePath&);
    const FilePath& GetHeatmapTable() const;

    void SetLightMeterTable(const FilePath&);
    const FilePath& GetLightMeterTable() const;

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    void SetColorGradingTexture(Texture* texture);
    void SetHeatmapTexture(Texture* texture);
    void SetLightMeterTexture(Texture* texture);

    bool GetToneMapping() const;
    void SetToneMapping(bool value);

    bool GetColorGrading() const;
    void SetColorGrading(bool value);

    bool GetHeapMapEnabled() const;
    void SetHeapMapEnabled(bool value);

    void SetResetHistory(bool reset);
    bool GetResetHistory() const;

private:
    ScopedPtr<NMaterial> material;
    float32 aperture = 16.0f; // 1/aperture
    float32 shutterTimeInv = 100.0f; // 1/shutterTime sec
    float32 ISO = 100.0f;
    float32 baseEV = 12.0f; // manual EV, works if overrideEV = true
    Vector2 dynamicRange = Vector2(3.0f, 3.0f);
    Vector2 adaptationRange = Vector2(1.0f, 1.0);
    Vector2 adaptationSpeed = Vector2(2.0f, 2.f);

    Color bloomColor = Color::White;
    float32 bloomEVC = 0.0f; // bloom EV compensation
    FilePath colorGradingTable;
    FilePath heatmapTable;
    FilePath lightMeterTable;
    bool useEv = true;
    bool enableToneMapping = false;
    bool enableColorGrading = false;
    bool enableHeatMap = false;
    bool resetHistory = true;

public:
    DAVA_VIRTUAL_REFLECTION(PostEffectComponent, Component);
};
};
