#pragma once

#include "Reflection/Reflection.h"
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class Landscape;
class PolygonGroup;
class SerializationContext;
class NMaterial;

class DecorationData
{
public:
    void Save(KeyedArchive* archive, SerializationContext* serializationContext);
    void Load(KeyedArchive* archive, SerializationContext* serializationContext);

    void CopyParameters(const DecorationData* from);

    //// Getters/Setters
    uint32 GetLayersCount() const;
    const FastName& GetLayerName(uint32 layer) const;
    NMaterial* GetLayerMaterial(uint32 layer);

    uint32 GetVariationCount(uint32 layer) const;
    const FastName& GetVariationName(uint32 layer, uint32 var) const;
    PolygonGroup* GetVariationGeometry(uint32 layer, uint32 var);

    uint32 GetBaseLevel() const;
    void SetBaseLevel(uint32 level);

    uint32 GetLevelCount() const;
    void SetLevelCount(uint32 count);

    const FilePath& GetDecorationPath() const;
    void SetDecorationPath(const FilePath& path);

    uint8 GetLayerMaskIndex(uint32 layer) const;
    void SetLayerMaskIndex(uint32 layer, uint8 index);

    bool GetLayerCullface(uint32 layer) const;
    void SetLayerCullface(uint32 layer, bool cullface);

    bool GetLayerCollisionDetection(uint32 layer) const;
    void SetLayerCollisionDetection(uint32 layer, bool enable);

    bool GetLayerOrientOnLandscape(uint32 layer) const;
    void SetLayerOrientOnLandscape(uint32 layer, bool orient);

    float32 GetLayerOrientValue(uint32 layer) const;
    void SetLayerOrientValue(uint32 layer, float32 value);

    bool GetLayerTint(uint32 layer) const;
    void SetLayerTint(uint32 layer, bool tint);

    float32 GetLayerTintHeight(uint32 layer) const;
    void SetLayerTintHeight(uint32 layer, float32 tintHeigth);

    bool GetVariationEnabled(uint32 layer, uint32 var) const;
    void SetVariationEnabled(uint32 layer, uint32 var, bool value);

    float32 GetVariationDensity(uint32 layer, uint32 var) const;
    void SetVariationDensity(uint32 layer, uint32 var, float32 density);

    float32 GetVariationScaleMin(uint32 layer, uint32 var) const;
    void SetVariationScaleMin(uint32 layer, uint32 var, float32 min);

    float32 GetVariationScaleMax(uint32 layer, uint32 varn) const;
    void SetVariationScaleMax(uint32 layer, uint32 var, float32 max);

    float32 GetVariationPitchMax(uint32 layer, uint32 var) const;
    void SetVariationPitchMax(uint32 layer, uint32 var, float32 max);

    uint32 GetVariationCollisionGroup(uint32 layer, uint32 var) const;
    void SetVariationCollisionGroup(uint32 layer, uint32 var, uint32 group);

    float32 GetVariationCollisionRadius(uint32 layer, uint32 var) const;
    void SetVariationCollisionRadius(uint32 layer, uint32 var, float32 radius);

    float32 GetLevelDensity(uint32 layer, uint32 var, uint32 level) const;
    void SetLevelDensity(uint32 layer, uint32 var, uint32 level, float32 density);

protected:
    struct VariationParams
    {
        float32 density = 0.f; //per square meter
        float32 scaleMin = 0.8f;
        float32 scaleMax = 1.0f;
        float32 pitchMax = 0.f;
        Vector<float32> levelDensity; //[0...1] density for level
        uint32 collisionGroup = 0;
        float32 collisionRadius = 0.f;
        bool enabled = true;
    };

    struct LayerParams
    {
        Vector<VariationParams> variations;
        float32 orientValue = 0.f;
        float32 tintHeight = 0.f;
        uint8 index = 0;
        bool cullface = false;
        bool orient = false;
        bool collisionDetection = false;
        bool tint = false;
    };

    struct VariationData
    {
        PolygonGroup* geometry = nullptr;
        FastName name;
    };

    struct LayerData
    {
        Vector<VariationData> variations;
        NMaterial* material = nullptr;
        FastName name;
    };

    void ReloadDecoration();
    void ReleaseInternalData();

    FilePath decorationPath;
    uint32 layersCount = 0;
    uint32 baseLevel = 7;
    uint32 levelCount = 3;
    Vector<LayerData> layersData;
    Vector<LayerParams> layersParams;

    bool paramsChanged = true;

    //TODO: kostyl. 'paramsChanged' used by Landscape to rebuild decoration
    friend class Landscape;
};

} //ns
