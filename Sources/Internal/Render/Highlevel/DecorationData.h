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

    uint32 GetLayersCount() const;
    const FastName& GetLayerName(uint32 layer) const;
    NMaterial* GetLayerMaterial(uint32 layer);

    uint32 GetVariationCount(uint32 layer) const;
    const FastName& GetVariationName(uint32 layer, uint32 var) const;
    PolygonGroup* GetVariationGeometry(uint32 layer, uint32 var);

    uint32 GetLevelCount() const;
    void SetLevelCount(uint32 count);

    uint32 GetBaseLevel() const;
    void SetBaseLevel(uint32 level);

    const FilePath& GetDecorationPath() const;
    void SetDecorationPath(const FilePath& path);

    void ReloadDecoration();
    void ReleaseInternalData();

    //kostyl to rebuild decoration by landscape
    void MarkParamsChanged();
    void MarkParamsUnchanged();
    bool IsParamsChanged();

public:
    struct VariationParams
    {
        float32 density = 0.f; //per square meter
        float32 scaleMin = 0.8f;
        float32 scaleMax = 1.0f;
        float32 pitchMax = 0.f;
        Vector<float32> levelDensity; //[0...1] density for level

        uint32 collisionGroup = 0;
        float32 collisionRadius = 0.f;

        float32 nearScale = 1.f;
        float32 farScale = 1.f;
        float32 nearDistance = 0.f;
        float32 farDistance = 1.f;

        bool distanceScale = false;
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

    Vector<LayerParams> layersParams;

protected:
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

    FilePath decorationPath;
    uint32 layersCount = 0;
    uint32 baseLevel = 7;
    uint32 levelCount = 3;
    Vector<LayerData> layersData;

    bool paramsChanged = true;
};

} //ns
