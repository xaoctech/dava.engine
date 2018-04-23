#include "DecorationData.h"

#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"

#include "Logger/Logger.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
void DecorationData::ReleaseInternalData()
{
    for (LayerData& layer : layersData)
    {
        for (VariationData& var : layer.variations)
            SafeRelease(var.geometry);
        layer.variations.clear();

        SafeRelease(layer.material);
    }
}

void DecorationData::ReloadDecoration()
{
    ReleaseInternalData();

    ScopedPtr<Scene> scene(new Scene(Scene::SceneSystemsPolicy::WithoutSystems));
    if (!decorationPath.IsEmpty() && SceneFileV2::ERROR_NO_ERROR != scene->LoadScene(decorationPath))
        return;

    layersCount = scene->GetChildrenCount();
    layersData.resize(layersCount);
    layersParams.resize(layersCount);
    for (uint32 l = 0; l < layersCount; ++l)
    {
        Entity* layerEntity = scene->GetChild(l);
        LayerData& layerData = layersData[l];
        DVASSERT(layerData.variations.empty());

        uint32 variationCount = layerEntity->GetChildrenCount();
        NMaterial* layerMaterial = nullptr;
        for (uint32 v = 0; v < variationCount; ++v)
        {
            PolygonGroup* varGeometry = nullptr;

            Entity* varEntity = layerEntity->GetChild(v);
            RenderObject* ro = GetRenderObject(varEntity);
            if (ro)
            {
                if (ro->GetRenderBatchCount() > 0)
                {
                    varGeometry = ro->GetRenderBatch(0)->GetPolygonGroup();
                    layerMaterial = ro->GetRenderBatch(0)->GetMaterial();
                }
                else
                {
                    Logger::Error("[Decoration loading] Error: RenderObject of variation entity must have RenderBatch");
                }
            }
            else
            {
                Logger::Error("[Decoration loading] Error: each variation entity must have RenderObject");
            }

            if (varGeometry)
            {
                layerData.variations.emplace_back();
                VariationData& variationData = layerData.variations.back();

                variationData.geometry = SafeRetain(varGeometry);
                variationData.name = varEntity->GetName();
            }
        }

        if (layerMaterial != nullptr)
        {
            while (layerMaterial->GetParent())
                layerMaterial = layerMaterial->GetParent();

            layerData.material = layerMaterial->Clone();
            layerData.material->SetRuntime(true);
        }
        layerData.name = layerEntity->GetName();

        layersParams[l].variations.resize(layerData.variations.size());
    }

    SetLevelCount(levelCount);
}

void DecorationData::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    archive->SetString("decoration.scenepath", decorationPath.GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetUInt32("decoration.layersCount", layersCount);
    archive->SetUInt32("decoration.baseLevel", baseLevel);
    archive->SetUInt32("decoration.levelCount", levelCount);

    for (uint32 l = 0; l < layersCount; ++l)
    {
        LayerParams& layer = layersParams[l];

        archive->SetBool(Format("decoration.layer%d.cullface", l), layer.cullface);
        archive->SetBool(Format("decoration.layer%d.orient", l), layer.orient);
        archive->SetBool(Format("decoration.layer%d.collisionDetection", l), layer.collisionDetection);
        archive->SetBool(Format("decoration.layer%d.tint", l), layer.tint);
        archive->SetFloat(Format("decoration.layer%d.orientValue", l), layer.orientValue);
        archive->SetFloat(Format("decoration.layer%d.tintHeight", l), layer.tintHeight);

        archive->SetUInt32(Format("decoration.layer%d.index", l), uint32(layer.index));

        uint32 varCount = uint32(layer.variations.size());
        archive->SetUInt32(Format("decoration.layer%d.variationsCount", l), varCount);

        for (uint32 v = 0; v < varCount; ++v)
        {
            VariationParams& var = layer.variations[v];

            archive->SetFloat(Format("decoration.layer%d.var%d.density", l, v), var.density);
            archive->SetFloat(Format("decoration.layer%d.var%d.scaleMin", l, v), var.scaleMin);
            archive->SetFloat(Format("decoration.layer%d.var%d.scaleMax", l, v), var.scaleMax);
            archive->SetFloat(Format("decoration.layer%d.var%d.pitchMax", l, v), var.pitchMax);
            archive->SetUInt32(Format("decoration.layer%d.var%d.collisionGroup", l, v), var.collisionGroup);
            archive->SetFloat(Format("decoration.layer%d.var%d.collisionRadius", l, v), var.collisionRadius);

            archive->SetBool(Format("decoration.layer%d.var%d.distanceScale", l, v), var.distanceScale);
            archive->SetFloat(Format("decoration.layer%d.var%d.nearDistance", l, v), var.nearDistance);
            archive->SetFloat(Format("decoration.layer%d.var%d.nearScale", l, v), var.nearScale);
            archive->SetFloat(Format("decoration.layer%d.var%d.farDistance", l, v), var.farDistance);
            archive->SetFloat(Format("decoration.layer%d.var%d.farScale", l, v), var.farScale);

            for (uint32 level = 1; level < levelCount; ++level) // level#0 always 1.0
                archive->SetFloat(Format("decoration.layer%d.var%d.levelDensity%d", l, v, level), var.levelDensity[level]);
        }
    }
}

void DecorationData::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (!archive->IsKeyExists("decoration.scenepath"))
        return;

    String scenepath = archive->GetString("decoration.scenepath");
    SetDecorationPath(serializationContext->GetScenePath() + scenepath);

    layersCount = std::min(layersCount, archive->GetUInt32("decoration.layersCount"));
    SetBaseLevel(archive->GetUInt32("decoration.baseLevel"));
    SetLevelCount(archive->GetUInt32("decoration.levelCount"));

    for (uint32 l = 0; l < layersCount; ++l)
    {
        LayerParams& layer = layersParams[l];

        layer.cullface = archive->GetBool(Format("decoration.layer%d.cullface", l));
        layer.orient = archive->GetBool(Format("decoration.layer%d.orient", l));
        layer.collisionDetection = archive->GetBool(Format("decoration.layer%d.collisionDetection", l));
        layer.tint = archive->GetBool(Format("decoration.layer%d.tint", l));
        layer.orientValue = archive->GetFloat(Format("decoration.layer%d.orientValue", l));
        layer.tintHeight = archive->GetFloat(Format("decoration.layer%d.tintHeight", l));

        if (archive->IsKeyExists(Format("decoration.layer%d.mask", l))) //backward compatibility
        {
            uint8 mask = uint8(archive->GetUInt32(Format("decoration.layer%d.mask", l)));
            for (uint8 i = 0; i < 4; ++i)
            {
                if (mask & (1 << i))
                    layer.index = i + 1;
            }
        }
        else
        {
            layer.index = uint8(archive->GetUInt32(Format("decoration.layer%d.index", l)));
        }

        uint32 varCount = Min(uint32(layer.variations.size()), archive->GetUInt32(Format("decoration.layer%d.variationsCount", l)));
        for (uint32 v = 0; v < varCount; ++v)
        {
            VariationParams& var = layer.variations[v];

            var.density = archive->GetFloat(Format("decoration.layer%d.var%d.density", l, v));
            var.scaleMin = archive->GetFloat(Format("decoration.layer%d.var%d.scaleMin", l, v));
            var.scaleMax = archive->GetFloat(Format("decoration.layer%d.var%d.scaleMax", l, v));
            var.pitchMax = archive->GetFloat(Format("decoration.layer%d.var%d.pitchMax", l, v));
            var.collisionGroup = archive->GetUInt32(Format("decoration.layer%d.var%d.collisionGroup", l, v));
            var.collisionRadius = archive->GetFloat(Format("decoration.layer%d.var%d.collisionRadius", l, v));

            var.distanceScale = archive->GetBool(Format("decoration.layer%d.var%d.distanceScale", l, v));
            var.nearDistance = archive->GetFloat(Format("decoration.layer%d.var%d.nearDistance", l, v));
            var.nearScale = archive->GetFloat(Format("decoration.layer%d.var%d.nearScale", l, v));
            var.farDistance = archive->GetFloat(Format("decoration.layer%d.var%d.farDistance", l, v));
            var.farScale = archive->GetFloat(Format("decoration.layer%d.var%d.farScale", l, v));

            for (uint32 level = 1; level < levelCount; ++level) // level#0 always 1.0
                var.levelDensity[level] = archive->GetFloat(Format("decoration.layer%d.var%d.levelDensity%d", l, v, level));
        }
    }

    paramsChanged = true;
}

void DecorationData::CopyParameters(const DecorationData* from)
{
    decorationPath = from->decorationPath;
    layersCount = from->layersCount;
    baseLevel = from->baseLevel;
    levelCount = from->levelCount;
    layersParams = from->layersParams;
}

uint32 DecorationData::GetLayersCount() const
{
    return layersCount;
}

const FastName& DecorationData::GetLayerName(uint32 layer) const
{
    DVASSERT(layer < layersCount);
    return layersData[layer].name;
}

NMaterial* DecorationData::GetLayerMaterial(uint32 layer)
{
    DVASSERT(layer < layersCount);
    return layersData[layer].material;
}

uint32 DecorationData::GetVariationCount(uint32 layer) const
{
    return uint32(layersData[layer].variations.size());
}

const FastName& DecorationData::GetVariationName(uint32 layer, uint32 var) const
{
    DVASSERT(layer < layersCount);
    DVASSERT(var < uint32(layersData[layer].variations.size()));
    return layersData[layer].variations[var].name;
}

PolygonGroup* DecorationData::GetVariationGeometry(uint32 layer, uint32 var)
{
    DVASSERT(layer < layersCount);
    DVASSERT(var < uint32(layersData[layer].variations.size()));
    return layersData[layer].variations[var].geometry;
}

uint32 DecorationData::GetBaseLevel() const
{
    return baseLevel;
}

void DecorationData::SetBaseLevel(uint32 level)
{
    baseLevel = level;
    paramsChanged = true;
}

uint32 DecorationData::GetLevelCount() const
{
    return levelCount;
}

void DecorationData::SetLevelCount(uint32 count)
{
    levelCount = count;

    for (uint32 l = 0; l < layersCount; ++l)
    {
        for (VariationParams& var : layersParams[l].variations)
        {
            var.levelDensity.resize(levelCount);
            if (levelCount)
                var.levelDensity[0] = 1.f;
        }
    }

    paramsChanged = true;
}

const FilePath& DecorationData::GetDecorationPath() const
{
    return decorationPath;
}

void DecorationData::SetDecorationPath(const FilePath& path)
{
    decorationPath = path;
    ReloadDecoration();
    paramsChanged = true;
}

void DecorationData::MarkParamsChanged()
{
    paramsChanged = true;
}

void DecorationData::MarkParamsUnchanged()
{
    paramsChanged = false;
}

bool DecorationData::IsParamsChanged()
{
    return paramsChanged;
}

} //ns
