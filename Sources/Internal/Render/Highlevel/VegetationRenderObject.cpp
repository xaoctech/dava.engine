/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include <cfloat>

#include "Render/Image.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/VegetationRenderObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Utils/Random.h"
#include "Render/ImageLoader.h"

namespace DAVA
{
    
static const FastName UNIFORM_TILEPOS = FastName("tilePos");
static const FastName UNIFORM_WORLD_SIZE = FastName("worldSize");
static const FastName UNIFORM_CLUSTER_SCALE_DENSITY_MAP = FastName("clusterScaleDensityMap[0]");
static const FastName UNIFORM_HEIGHTMAP_SCALE = FastName("heightmapScale");

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
static const FastName UNIFORM_CLUSTER_SCALE_DENSITY_MAP_ARRAY[] =
{
    FastName("clusterScaleDensityMap_0"),
    FastName("clusterScaleDensityMap_1"),
    FastName("clusterScaleDensityMap_2"),
    FastName("clusterScaleDensityMap_3"),
    FastName("clusterScaleDensityMap_4"),
    FastName("clusterScaleDensityMap_5"),
    FastName("clusterScaleDensityMap_6"),
    FastName("clusterScaleDensityMap_7"),
    FastName("clusterScaleDensityMap_8"),
    FastName("clusterScaleDensityMap_9"),
    FastName("clusterScaleDensityMap_10"),
    FastName("clusterScaleDensityMap_11"),
    FastName("clusterScaleDensityMap_12"),
    FastName("clusterScaleDensityMap_13"),
    FastName("clusterScaleDensityMap_14"),
    FastName("clusterScaleDensityMap_15")
};
#endif

static const FastName UNIFORM_SAMPLER_VEGETATIONMAP = FastName("vegetationmap");
    
static const uint32 MAX_VERTEX_PER_CLUSTER = 12;
static const uint32 MAX_INDEX_PER_CLUSTER = 18;
static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
static const float32 CLUSTER_SCALE_NORMALIZATION_VALUE = 15.0f;
    
static const uint32 MAX_BRUSH_DIVERSITY = 4;
    
static const size_t MAX_RENDER_CELLS = 256;
static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 80.0f * 80.0f; //meters * meters (square length)
static const float32 MAX_VISIBLE_SCALING_DISTANCE = 60.0f * 60.0f;
    
static const float32 MAX_ROTATION_ANGLE = 0.0f;
//static const Vector3 MAX_DISPLACEMENT = Vector3(0.0f, 0.0f, 0.0f);

static const uint32 FULL_BRUSH_VALUE = 0xFFFFFFFF;

//static const float32 MAX_ROTATION_ANGLE = 0.0f;
static const Vector3 MAX_DISPLACEMENT = Vector3(1.6f, 1.6f, 0.0f);
    
static const uint32 DENSITY_ROW_INDEX = 0;
static const uint32 SCALE_ROW_INDEX = 1;
static const uint32 CAMERA_DIRECTION_INDEX = 2;
    
static const Vector3 CLUSTER_TYPE_0[] =
{
    Vector3(-0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 0.0f),
    Vector3(-0.5f, 0.0f, 0.0f),
    
    Vector3(-0.35f, -0.35f, 1.0f),
    Vector3(0.35f, 0.35f, 1.0f),
    Vector3(0.35f, 0.35f, 0.0f),
    Vector3(-0.35f, -0.35f, 0.0f),

    Vector3(-0.35f, 0.35f, 1.0f),
    Vector3(0.35f, -0.35f, 1.0f),
    Vector3(0.35f, -0.35f, 0.0f),
    Vector3(-0.35f, 0.35f, 0.0f),
};

static const Vector3 CLUSTER_TYPE_1[] =
{
    Vector3(-0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 0.0f),
    Vector3(-0.5f, 0.1f, 0.0f),
    
    Vector3(-0.15f, -0.53f, 1.0f),
    Vector3(0.35f, 0.33f, 1.0f),
    Vector3(0.35f, 0.33f, 0.0f),
    Vector3(-0.15f, -0.53f, 0.0f),
    
    Vector3(-0.35f, 0.33f, 1.0f),
    Vector3(0.15f, -0.53f, 1.0f),
    Vector3(0.15f, -0.53f, 0.0f),
    Vector3(-0.35f, 0.33f, 0.0f),
};
    
static const int16 CLUSTER_INDICES[] =
{
    0, 3,  1, 1, 3,  2,
    4, 7,  5, 5, 7,  6,
    8, 11, 9, 9, 11, 10
};
    
static const Vector3* VEGETATION_CLUSTER[] =
{
    CLUSTER_TYPE_0,
    CLUSTER_TYPE_1
};
    
static const int16* VEGETATION_CLUSTER_INDICES[] =
{
    CLUSTER_INDICES,
    CLUSTER_INDICES
};
    
static const uint32 VEGETATION_CLUSTER_SIZE[] =
{
    COUNT_OF(CLUSTER_TYPE_0),
    COUNT_OF(CLUSTER_TYPE_1)
};

static const uint32 VEGETATION_CLUSTER_INDEX_SIZE[] =
{
    COUNT_OF(CLUSTER_INDICES),
    COUNT_OF(CLUSTER_INDICES)
};

static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 1.0f, 11.0f);

static Vector2 RESOLUTION_RANGES[] = //squared
{
    Vector2(0.0f, 0.0f),
    Vector2(0.0, 0.0f),
    Vector2(0.0f, 0.0f)
};


static float32 RESOLUTION_SCALE[] =
{
    1.0f,
    2.0f,
    4.0f,
};

static uint32 RESOLUTION_INDEX[] =
{
    0,
    1,
    2
};

static uint32 RESOLUTION_CELL_SQUARE[] =
{
    1,
    4,
    16
};

static float32 RESOLUTION_DISTANCE_SCALE_COMPENSATION[] =
{
    1.0f,
    1.4f,
    1.4f
};

static const float32 MAX_DISTANCE_CELL_INDEX = 15.0f;
    
int32 RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}

inline uint32 MapToResolution(float32 squareDistance)
{
    uint32 resolutionId = 0;
    
    uint32 rangesCount = COUNT_OF(RESOLUTION_RANGES);
    for(uint32 i = 0; i < rangesCount; ++i)
    {
        if(squareDistance > RESOLUTION_RANGES[i].x &&
           squareDistance <= RESOLUTION_RANGES[i].y)
        {
            resolutionId = i;
            break;
        }
    }
    
    return resolutionId;
}

inline uint32 MapCellSquareToResolutionIndex(uint32 cellSquare)
{
    uint32 index = 0;
    uint32 resolutionCount = COUNT_OF(RESOLUTION_CELL_SQUARE);
    for(uint32 i = 0; i < resolutionCount; ++i)
    {
        if(cellSquare == RESOLUTION_CELL_SQUARE[i])
        {
            index = RESOLUTION_INDEX[i];
            break;
        }
    }
    
    return index;
}
    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    heightmap(NULL),
    clusterLimit(0),
    halfWidth(0),
    halfHeight(0)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_VEGETATION;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
    
    unitWorldSize.resize(COUNT_OF(RESOLUTION_SCALE));
    
    uint32 maxParams = (uint32)(RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1] * RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1]);
    shaderScaleDensityParams.resize(maxParams);
    
    renderBatchPoolLine = 0;
    
    vegetationMaterial = NMaterial::CreateMaterial(FastName("Vegetation_Material"),
                                                           NMaterialName::GRASS,
                                                           NMaterial::DEFAULT_QUALITY_NAME);
    vegetationMaterial->AddNodeFlags(DataNode::NodeRuntimeFlag);
    
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

    vegetationMaterial->SetFlag(FastName("MATERIAL_GRASS_PLAINUNIFORMS"), NMaterial::FlagOn);
    
#endif

    maxVisibleQuads = MAX_RENDER_CELLS;
    lodRanges = LOD_RANGES_SCALE;
    ResetVisibilityDistance();
}

VegetationRenderObject::~VegetationRenderObject()
{
    SafeRelease(vegetationMap);
    SafeRelease(heightmap);
    
    size_t brushCount = clusterBrushes.size();
    for(size_t i = 0; i < brushCount; ++i)
    {
        SafeRelease(clusterBrushes[i]);
    }
    
    size_t poolCount = renderBatchPool.size();
    for(size_t i = 0; i < poolCount; ++i)
    {
        SafeRelease(renderBatchPool[i]);
    }
    
    SafeRelease(vegetationMaterial);
}
    
RenderObject* VegetationRenderObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        newObject = new VegetationRenderObject();
    }
    else
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(newObject), "Can clone only to VegetationRenderObject");
    }
    
    VegetationRenderObject* vegetationRenderObject = static_cast<VegetationRenderObject*>(newObject);
    
    vegetationRenderObject->SetHeightmap(GetHeightmap());
    vegetationRenderObject->SetVegetationMap(GetVegetationMap());
    vegetationRenderObject->SetTextureSheet(GetTextureSheetPath());
    vegetationRenderObject->SetClusterLimit(GetClusterLimit());
    vegetationRenderObject->SetVegetationMapPath(GetVegetationMapPath());
    vegetationRenderObject->SetHeightmapPath(GetHeightmapPath());
    vegetationRenderObject->SetLightmap(GetLightmapPath());
    vegetationRenderObject->SetVegetationTexture(GetVegetationTexture());
    vegetationRenderObject->SetWorldSize(GetWorldSize());

    return vegetationRenderObject;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    //VI: need to remove render batches since they are temporary
    int32 batchesToRemove = GetRenderBatchCount();
    for(int32 i = 0; i < batchesToRemove; ++i)
    {
        RemoveRenderBatch(GetRenderBatchCount() - 1);
    }
    ReturnToPool(batchesToRemove);
    
    RenderObject::Save(archive, serializationContext);
    
    archive->SetUInt32("vro.clusterLimit", GetClusterLimit());
    archive->SetString("vro.vegmap", GetVegetationMapPath().GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetString("vro.texturesheet", GetTextureSheetPath().GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetString("vro.vegtexture", vegetationMaterial->GetTexturePath(NMaterial::TEXTURE_ALBEDO).GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetString("vro.lightmap", vegetationMaterial->GetTexturePath(UNIFORM_SAMPLER_VEGETATIONMAP).GetRelativePathname(serializationContext->GetScenePath()));
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    //VI: need to remove render batches since they are temporary
    int32 batchesToRemove = GetRenderBatchCount();
    for(int32 i = 0; i < batchesToRemove; ++i)
    {
        RemoveRenderBatch(GetRenderBatchCount() - 1);
    }
    ReturnToPool(batchesToRemove);
    
    RenderObject::Load(archive, serializationContext);
    
    SetClusterLimit(archive->GetUInt32("vro.clusterLimit"));
    SetVegetationMap(serializationContext->GetScenePath() + archive->GetString("vro.vegmap"));
    SetTextureSheet(serializationContext->GetScenePath() + archive->GetString("vro.texturesheet"));
    SetVegetationTexture(serializationContext->GetScenePath() + archive->GetString("vro.vegtexture"));
    SetLightmap(serializationContext->GetScenePath() + archive->GetString("vro.lightmap"));
}
  
void VegetationRenderObject::PrepareToRender(Camera *camera)
{
    if(clusterBrushes.size() == 0)
    {
        return;
    }
    
    visibleCells.clear();
    BuildVisibleCellList(camera->GetPosition(), camera->GetFrustum(), visibleCells);
    
    std::sort(visibleCells.begin(), visibleCells.end(), CellByDistanceCompareFunction);
    
    uint32 requestedBatchCount = Min(visibleCells.size(), (size_t)maxVisibleQuads);
    uint32 currentBatchCount = GetRenderBatchCount();
    
    if(requestedBatchCount > currentBatchCount)
    {
        int32 batchesToAdd = requestedBatchCount - currentBatchCount;
        
        for(int32 i = 0; i < batchesToAdd; ++i)
        {
            RenderBatch* rb = GetRenderBatchFromPool(vegetationMaterial);
            
            AddRenderBatch(rb);
        }
    }
    else if (requestedBatchCount < currentBatchCount)
    {
        int32 batchesToRemove = currentBatchCount - requestedBatchCount;
        
        for(int32 i = 0; i < batchesToRemove; ++i)
        {
            RemoveRenderBatch(GetRenderBatchCount() - 1);
        }
        
        ReturnToPool(batchesToRemove);
    }
    
    Vector4 posScale(0.0f,
                     0.0f,
                     worldSize.x,
                     worldSize.y);
    
    for(size_t i = 0; i < requestedBatchCount; ++i)
    {
        SpatialData* spatialData = visibleCells[i];
        
        RenderBatch* rb = GetRenderBatch(i);
        
        NMaterial* mat = rb->GetMaterial();
        
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(spatialData->width * spatialData->height);
        uint32 renderBrushIndex = ((spatialData->x + spatialData->y) % MAX_BRUSH_DIVERSITY) + resolutionIndex * MAX_BRUSH_DIVERSITY;
        rb->SetPolygonGroup(clusterBrushes[renderBrushIndex]);
        
        float32 distanceScale = (spatialData->cameraDistance > MAX_VISIBLE_SCALING_DISTANCE && resolutionIndex == RESOLUTION_INDEX[COUNT_OF(RESOLUTION_INDEX) - 1]) ? (1.0f - (spatialData->cameraDistance - MAX_VISIBLE_SCALING_DISTANCE) / MAX_VISIBLE_SCALING_DISTANCE) : 1.0f;
        
        for(uint32 y = 0; y < spatialData->height; ++y)
        {
            for(uint32 x = 0; x < spatialData->width; ++x)
            {
                uint32 paramIndex = y * spatialData->width + x;
                
                Matrix4& scaleDensityMap = shaderScaleDensityParams[paramIndex];
                scaleDensityMap.Zero();
                
                int32 mapX = spatialData->x + halfWidth + x;
                int32 mapY = spatialData->y + halfHeight + y;
                uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;

                uint8 *vegetationMapValuePtr = (vegetationMap->data + cellDescriptionIndex * 4);
                
                for(uint32 clusterType = 0; clusterType < MAX_CLUSTER_TYPES; ++clusterType)
                {
                    uint8 cellLayerData = vegetationMapValuePtr[clusterType];
            
                    float32 clusterScale = (1.0f * ((cellLayerData >> 4) & 0xF)) / CLUSTER_SCALE_NORMALIZATION_VALUE;
                    float32 density = (1.0f * (cellLayerData & 0xF)) + 1.0f; //step function uses "<" so we need to emulate "<="
                    
                    scaleDensityMap._data[DENSITY_ROW_INDEX][clusterType] = density;
                    scaleDensityMap._data[SCALE_ROW_INDEX][clusterType] = distanceScale * RESOLUTION_DISTANCE_SCALE_COMPENSATION[resolutionIndex] * clusterScale;
                }
            }
        }
        
        posScale.x = spatialData->bbox.min.x;
        posScale.y = spatialData->bbox.min.y;
        
        mat->SetPropertyValue(UNIFORM_TILEPOS, Shader::UT_FLOAT_VEC4, 1, posScale.data);
        
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

        for(uint32 mapArrayIndex = 0; mapArrayIndex < COUNT_OF(UNIFORM_CLUSTER_SCALE_DENSITY_MAP_ARRAY); ++mapArrayIndex)
        {
            mat->SetPropertyValue(UNIFORM_CLUSTER_SCALE_DENSITY_MAP_ARRAY[mapArrayIndex],
                                  Shader::UT_FLOAT_MAT4,
                                  1,
                                  shaderScaleDensityParams[mapArrayIndex]._data);
        }

#else
        
        mat->SetPropertyValue(UNIFORM_CLUSTER_SCALE_DENSITY_MAP,
                              Shader::UT_FLOAT_MAT4,
                              shaderScaleDensityParams.size(),
                              shaderScaleDensityParams[0]._data);
        
#endif
    }
}
    
void VegetationRenderObject::SetVegetationMap(VegetationMap* map)
{
    if(map != vegetationMap)
    {
        Texture* tx = Texture::CreateFromData(map->GetPixelFormat(),
                                              map->GetData(),
                                              map->GetWidth(),
                                              map->GetHeight(),
                                              false);
        vegetationMaterial->SetTexture(UNIFORM_SAMPLER_VEGETATIONMAP, tx);
        SafeRelease(tx);
        
        SafeRelease(vegetationMap);
        vegetationMap = SafeRetain(map);
        
        UpdateVegetationSetup();
    }
}

void VegetationRenderObject::SetVegetationMap(const FilePath& path)
{
    if(path.Exists())
    {
        Vector<Image*> images = ImageLoader::CreateFromFileByExtension(path);
            
        DVASSERT(images.size());
            
        if(images.size())
        {
            VegetationMap* vegMap = images[0];
            
            SetVegetationMap(vegMap);
            SetVegetationMapPath(path);
                
            for(size_t i = 0; i < images.size(); ++i)
            {
                SafeRelease(images[i]);
            }
        }
    }
}
    
VegetationMap* VegetationRenderObject::GetVegetationMap() const
{
    return vegetationMap;
}
    
void VegetationRenderObject::SetVegetationMapPath(const FilePath& path)
{
    vegetationMapPath = path;
}

const FilePath& VegetationRenderObject::GetVegetationMapPath() const
{
    return vegetationMapPath;
}

void VegetationRenderObject::SetTextureSheet(const FilePath& path)
{
    textureSheetPath = path;
    textureSheet.Load(path);
    
    UpdateVegetationSetup();
}

void VegetationRenderObject::SetVegetationTexture(const FilePath& texturePath)
{
    vegetationMaterial->SetTexture(NMaterial::TEXTURE_ALBEDO,
                                   texturePath);
}

const FilePath& VegetationRenderObject::GetVegetationTexture() const
{
    return vegetationMaterial->GetTexturePath(NMaterial::TEXTURE_ALBEDO);
}
    
const TextureSheet& VegetationRenderObject::GetTextureSheet() const
{
    return textureSheet;
}
    
const FilePath& VegetationRenderObject::GetTextureSheetPath() const
{
    return textureSheetPath;
}

void VegetationRenderObject::SetClusterLimit(const uint32& maxClusters)
{
    clusterLimit = maxClusters;
    
    UpdateVegetationSetup();
}

uint32 VegetationRenderObject::GetClusterLimit() const
{
    return clusterLimit;
}

void VegetationRenderObject::SetHeightmap(Heightmap* _heightmap)
{
    if(heightmap != _heightmap)
    {
        SafeRelease(heightmap);
        heightmap = (_heightmap->Data()) ? SafeRetain(_heightmap) : NULL;
        
        if(heightmap)
        {
            InitHeightTextureFromHeightmap(heightmap);
        }
        
        UpdateVegetationSetup();
    }
}

Heightmap* VegetationRenderObject::GetHeightmap() const
{
    return heightmap;
}
    
const FilePath& VegetationRenderObject::GetHeightmapPath() const
{
    return heightmapPath;
}

void VegetationRenderObject::SetHeightmapPath(const FilePath& path)
{
    heightmapPath = path;
}
    
void VegetationRenderObject::SetLightmap(const FilePath& filePath)
{
    vegetationMaterial->SetTexture(UNIFORM_SAMPLER_VEGETATIONMAP, filePath);
}
    
Texture* VegetationRenderObject::GetLightmap() const
{
    return vegetationMaterial->GetTexture(UNIFORM_SAMPLER_VEGETATIONMAP);
}

const FilePath& VegetationRenderObject::GetLightmapPath() const
{
    return vegetationMaterial->GetTexturePath(UNIFORM_SAMPLER_VEGETATIONMAP);
}
    
void VegetationRenderObject::BuildVegetationBrush(uint32 maxClusters)
{
    DVASSERT(maxClusters > 0);
    DVASSERT(textureSheet.cells.size() > 0);
    
    InitLodRanges();
    
    uint32 totalBrushes = MAX_BRUSH_DIVERSITY * COUNT_OF(RESOLUTION_SCALE);
    
    if(clusterBrushes.size() < totalBrushes)
    {
        uint32 difference = totalBrushes - clusterBrushes.size();
        for(uint32 i = 0; i < difference; ++i)
        {
            clusterBrushes.push_back(new PolygonGroup());
        }
    }
    
    uint32 clusterTypeCount = textureSheet.cells.size();
    uint32 totalClustersPerUnit = (maxClusters * maxClusters);
    Vector3 normal(0.0f, 0.0f, 1.0f); //up
    
    for(uint32 resolutionIndex = 0;
        resolutionIndex < COUNT_OF(RESOLUTION_SCALE);
        ++resolutionIndex)
    {
        Vector2 unitSize = GetVegetationUnitWorldSize(RESOLUTION_SCALE[resolutionIndex]);
        
        Vector2 clusterOffset(unitSize.x / maxClusters,
                              unitSize.y / maxClusters);
        
        Vector2 clusterTypeOffset(clusterOffset.x / clusterTypeCount,
                                  clusterOffset.y / clusterTypeCount);
        
        Vector<uint32> shuffleDensity;
        shuffleDensity.reserve(totalClustersPerUnit);
        for(uint32 i = 0; i < totalClustersPerUnit; ++i)
        {
            shuffleDensity.push_back((i % MAX_DENSITY_LEVELS) + 1);
        }
        
        for(uint32 brushIndex = 0; brushIndex < MAX_BRUSH_DIVERSITY; ++brushIndex)
        {
            PolygonGroup* clusterBrush = clusterBrushes[brushIndex + (MAX_BRUSH_DIVERSITY * resolutionIndex)];
            clusterBrush->ReleaseData();
            
            
            clusterBrush->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_BINORMAL | EVF_TANGENT | EVF_TEXCOORD0,
                                       MAX_VERTEX_PER_CLUSTER * totalClustersPerUnit * clusterTypeCount,
                                       MAX_INDEX_PER_CLUSTER * totalClustersPerUnit * clusterTypeCount);
            
            uint32 vertexIndex = 0;
            uint32 indexIndex = 0;
            
            for(uint32 clusterTypeIndex = 0; clusterTypeIndex < clusterTypeCount; ++clusterTypeIndex)
            {
                uint32 geometryType = textureSheet.cells[clusterTypeIndex].geometryId;
                std::random_shuffle(shuffleDensity.begin(), shuffleDensity.end(), RandomShuffleFunc);
                
                for(uint32 clusterIndex = 0; clusterIndex < totalClustersPerUnit; ++clusterIndex)
                {
                    float32 clusterX = (clusterIndex % maxClusters) * clusterOffset.x + clusterTypeIndex * clusterTypeOffset.x;
                    float32 clusterY = (clusterIndex / maxClusters) * clusterOffset.y + clusterTypeIndex * clusterTypeOffset.y;
                    
                    uint32 densityId = shuffleDensity[clusterIndex];
                    
                    float32 randomRotation = MAX_ROTATION_ANGLE * (0.5f - (float32)Random::Instance()->RandFloat());
                    float32 randomDisplacementX = MAX_DISPLACEMENT.x * (0.5f - Random::Instance()->RandFloat());
                    float32 randomDisplacementY = MAX_DISPLACEMENT.y * (0.5f - Random::Instance()->RandFloat());
                    float32 randomDisplacementZ = MAX_DISPLACEMENT.z * (0.5f - Random::Instance()->RandFloat());
                    
                    Matrix4 transform = Matrix4::MakeRotation(normal, DegToRad(randomRotation)) *
                    Matrix4::MakeTranslation(Vector3(randomDisplacementX,
                                                     randomDisplacementY,
                                                     randomDisplacementZ));
                    
                    const int16* indexData = VEGETATION_CLUSTER_INDICES[geometryType];
                    uint32 indexDataSize = VEGETATION_CLUSTER_INDEX_SIZE[geometryType];
                    for(uint32 i = 0; i < indexDataSize; ++i)
                    {
                        clusterBrush->SetIndex(indexIndex, vertexIndex + indexData[i]);
                        indexIndex++;
                    }
                    
                    const Vector3* vertexData = VEGETATION_CLUSTER[geometryType];
                    uint32 vertexDataSize = VEGETATION_CLUSTER_SIZE[geometryType];
                    
                    Vector3 clusterCenter = Vector3(clusterX,
                                                    clusterY,
                                                    0.0);
                    
                    clusterCenter = clusterCenter * transform;
                    
                    float32 cX = floor(clusterCenter.x / unitWorldSize[0].x);
                    float32 cY = floor(clusterCenter.y / unitWorldSize[0].y);
                    
                    for(uint32 i = 0; i < vertexDataSize; ++i)
                    {
                        Vector3 vertexCoord = Vector3(vertexData[i].x * textureSheet.cells[clusterTypeIndex].geometryScale.x + clusterCenter.x,
                                                      vertexData[i].y * textureSheet.cells[clusterTypeIndex].geometryScale.x + clusterCenter.y,
                                                      vertexData[i].z * textureSheet.cells[clusterTypeIndex].geometryScale.y + clusterCenter.z);
                        
                        clusterBrush->SetCoord(vertexIndex, vertexCoord);
                        clusterBrush->SetBinormal(vertexIndex, clusterCenter);
                        clusterBrush->SetTexcoord(0, vertexIndex, textureSheet.cells[clusterTypeIndex].coords[i % MAX_CELL_TEXTURE_COORDS]);
                        float32 matrixIndex = Clamp((float32)floor(cX + cY * RESOLUTION_SCALE[resolutionIndex]), 0.0f, MAX_DISTANCE_CELL_INDEX);
                        DVASSERT(matrixIndex >= 0.0f && matrixIndex <= 15.0f);
                        DVASSERT(clusterTypeIndex >= 0.0f && clusterTypeIndex <= 3.0f);
                        
                        clusterBrush->SetTangent(vertexIndex, Vector3(Clamp((float32)floor(cX + cY * RESOLUTION_SCALE[resolutionIndex]), 0.0f, MAX_DISTANCE_CELL_INDEX), clusterTypeIndex, densityId));
                        clusterBrush->SetNormal(vertexIndex, normal);
                        
                        vertexIndex++;
                    }
                }
            }
        }
    }
}
    
RenderBatch* VegetationRenderObject::GetRenderBatchFromPool(NMaterial* material)
{
    RenderBatch* rb = NULL;
    
    size_t currentPoolSize = renderBatchPool.size();
    if(currentPoolSize <= renderBatchPoolLine)
    {
        rb = new RenderBatch();
        
        NMaterial* batchMaterial = NMaterial::CreateMaterialInstance();
        batchMaterial->AddNodeFlags(DataNode::NodeRuntimeFlag);
        batchMaterial->SetParent(vegetationMaterial);
        
        rb->SetMaterial(batchMaterial);
        
        SafeRelease(batchMaterial);
        
        renderBatchPool.push_back(rb);
    }
    else
    {
        rb = renderBatchPool[renderBatchPoolLine];
    }
    
    renderBatchPoolLine++;
    
    return rb;
}
    
void VegetationRenderObject::ReturnToPool(int32 batchCount)
{
    renderBatchPoolLine -= batchCount;
    DVASSERT(renderBatchPoolLine >= 0);
}

void VegetationRenderObject::SetWorldSize(const Vector3 size)
{
    worldSize = size;
    
    vegetationMaterial->SetPropertyValue(UNIFORM_WORLD_SIZE,
                                         Shader::UT_FLOAT_VEC3,
                                         1,
                                         &worldSize);

    UpdateVegetationSetup();
}
    
const Vector3& VegetationRenderObject::GetWorldSize() const
{
    return worldSize;
}

Vector2 VegetationRenderObject::GetVegetationUnitWorldSize(float32 resolution) const
{
    DVASSERT(vegetationMap);
    return Vector2((worldSize.x / vegetationMap->width) * resolution,
                   (worldSize.y / vegetationMap->height) * resolution);
}
    
void VegetationRenderObject::BuildSpatialStructure(VegetationMap* vegMap)
{
    DVASSERT(vegMap);
    DVASSERT(heightmap);
    DVASSERT(IsPowerOf2(vegMap->GetWidth()));
    DVASSERT(IsPowerOf2(vegMap->GetHeight()));
    DVASSERT(vegMap->GetWidth() == vegMap->GetHeight());
    
    uint32 mapSize = vegMap->GetWidth();
    uint32 heightmapSize = heightmap->Size();
    
    halfWidth = mapSize / 2;
    halfHeight = mapSize / 2;
    
    heightmapToVegetationMapScale = Vector2((1.0f * heightmapSize) / mapSize,
                                            (1.0f * heightmapSize) / mapSize);
    
    uint32 treeDepth = FastLog2(mapSize);
    
    quadTree.Init(treeDepth);
    AbstractQuadTreeNode<SpatialData>* node = quadTree.GetRoot();
    
    uint32 halfSize = mapSize >> 1;
    BuildSpatialQuad(node, -1 * halfSize, -1 * halfSize, mapSize, mapSize, node->data.bbox);
}
    
void VegetationRenderObject::BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          int16 x, int16 y,
                          uint16 width, uint16 height,
                          AABBox3& parentBox)
{
    DVASSERT(node);
    
    node->data.x = x;
    node->data.y = y;
    
    if(width <= RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1])
    {
        node->data.width = width;
        node->data.height = height;
    }
    else
    {
        node->data.width = -1;
        node->data.height = -1;
    }
    
    if(node->IsTerminalLeaf())
    {
        int32 mapX = x + halfWidth;
        int32 mapY = y + halfHeight;

        float32 heightmapHeight = SampleHeight(mapX, mapY);
        node->data.bbox.AddPoint(Vector3(x * unitWorldSize[0].x, y * unitWorldSize[0].y, (heightmapHeight - 0.5f)));
        node->data.bbox.AddPoint(Vector3((x + width) * unitWorldSize[0].x, (y + height) * unitWorldSize[0].y, (heightmapHeight + 0.5f)));
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
    else
    {
        int16 cellHalfWidth = width >> 1;
        int16 cellHalfHeight = height >> 1;
        
        BuildSpatialQuad(node->children[0], x, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[1], x + cellHalfWidth, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[2], x, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[3], x + cellHalfWidth, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  Vector<SpatialData*>& cellList)
{
    uint8 planeMask = 0x3F;
    Vector3 cameraPosXY = cameraPoint;
    cameraPosXY.z = 0.0f;
    BuildVisibleCellList(cameraPosXY, frustum, planeMask, quadTree.GetRoot(), cellList);
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  uint8 planeMask,
                                                  AbstractQuadTreeNode<SpatialData>* node,
                                                  Vector<SpatialData*>& cellList)
{
    static Vector3 corners[8];
    if(node)
    {
        Frustum::eFrustumResult result = frustum->Classify(node->data.bbox, planeMask, node->data.clippingPlane);
        if(Frustum::EFR_OUTSIDE != result)
        {
            if(node->data.IsRenderable())
            {
                float32 cameraDistance = FLT_MAX;
                node->data.bbox.GetCorners(corners);
                for(uint32 i = 0; i < 8; ++i)
                {
                    corners[i].z = 0;
                    
                    Vector3 cameraVector = cameraPoint - corners[i];
                    float32 curDistance = cameraVector.SquareLength();
                    
                    cameraDistance = Min(cameraDistance, curDistance);
                }
                
                node->data.cameraDistance = cameraDistance;
                
                uint32 resolutionId = MapToResolution(node->data.cameraDistance);
                if(node->IsTerminalLeaf() ||
                   RESOLUTION_CELL_SQUARE[resolutionId] >= node->data.GetResolutionId())
                {
                    int32 mapX = node->data.x + halfWidth;
                    int32 mapY = node->data.y + halfHeight;
                    uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;

                    uint32 vegetationMapValue = (node->IsTerminalLeaf()) ? (*(((uint32*)vegetationMap->data) + cellDescriptionIndex)) : FULL_BRUSH_VALUE;
                    AddVisibleCell(&(node->data), MAX_VISIBLE_CLIPPING_DISTANCE,
                                   vegetationMapValue, cellList);
                }
                else if(!node->IsTerminalLeaf())
                {
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList);
                }
            }
            else
            {
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList);
            }
        }
    }
}
        
bool VegetationRenderObject::CellByDistanceCompareFunction(const SpatialData* a,
                                                           const SpatialData* b)
{
    return (a->cameraDistance < b->cameraDistance);
}
    
void VegetationRenderObject::InitHeightTextureFromHeightmap(Heightmap* heightMap)
{
    Image* originalImage = Image::CreateFromData(heightMap->Size(),
                                                 heightMap->Size(),
                                                 FORMAT_A16,
                                                 (uint8*)heightMap->Data());
    
    int32 pow2Size = heightmap->Size();
    if(!IsPowerOf2(heightmap->Size()))
    {
        EnsurePowerOf2(pow2Size);
        
        if(pow2Size > heightmap->Size())
        {
            pow2Size = pow2Size >> 1;
        }
    }
    
    Texture* tx = NULL;
    if(pow2Size != heightmap->Size())
    {
        Image* croppedImage = Image::CopyImageRegion(originalImage, pow2Size, pow2Size);
        tx = Texture::CreateFromData(FORMAT_A16, croppedImage->GetData(), pow2Size, pow2Size, false);
     
        SafeRelease(croppedImage);
    }
    else
    {
        tx = Texture::CreateFromData(FORMAT_A16, originalImage->GetData(), pow2Size, pow2Size, false);
    }
    
    SafeRelease(originalImage);
    
    heightmapScale = Vector2((1.0f * heightmap->Size()) / pow2Size,
                             (1.0f * heightmap->Size()) / pow2Size);
    
    vegetationMaterial->SetTexture(NMaterial::TEXTURE_DETAIL, tx);
    vegetationMaterial->SetPropertyValue(UNIFORM_HEIGHTMAP_SCALE, Shader::UT_FLOAT_VEC2, 1, heightmapScale.data);
    
    SafeRelease(tx);
}
    
float32 VegetationRenderObject::SampleHeight(int16 x, int16 y)
{
    uint32 hX = heightmapToVegetationMapScale.x * x;
    uint32 hY = heightmapToVegetationMapScale.y * y;
    
    uint16 left = (hX > 0) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX - 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 right = (hX < halfWidth) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX + 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 top = (hY > 0) ? *(heightmap->Data() + (((hY - 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 down = (hY < halfHeight) ? *(heightmap->Data() + (((hY + 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 center = *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    
    uint16 heightmapValue = (left + right + top + down + center) / 5;
    
    float32 height = ((float32)heightmapValue / (float32)Heightmap::MAX_VALUE) * worldSize.z;
    
    return height;
}

bool VegetationRenderObject::IsValidGeometryData() const
{
     return (worldSize.Length() > 0 &&
             heightmap != NULL &&
             vegetationMap != NULL &&
             textureSheet.cells.size() > 0 &&
             clusterLimit > 0);
}
    
bool VegetationRenderObject::IsValidSpatialData() const
{
    return (worldSize.Length() > 0 &&
            heightmap != NULL &&
            vegetationMap != NULL);
}

void VegetationRenderObject::UpdateVegetationSetup()
{
    if(vegetationMap)
    {
        for(size_t i = 0; i < COUNT_OF(RESOLUTION_SCALE); ++i)
        {
            unitWorldSize[i] = GetVegetationUnitWorldSize(RESOLUTION_SCALE[i]);
        }
    }
    
    if(IsValidGeometryData())
    {
        BuildVegetationBrush(clusterLimit);
    }
    
    if(IsValidSpatialData())
    {
        BuildSpatialStructure(vegetationMap);
    }
}

void TextureSheet::Load(const FilePath &yamlPath)
{
    if(yamlPath.Exists())
    {
        YamlParser *parser = YamlParser::Create(yamlPath);
        YamlNode *rootNode = parser->GetRootNode();

        cells.clear();

        if(NULL != rootNode)
        {
            for(int i = 0; i < rootNode->GetCount(); ++i)
            {
                if(rootNode->GetItemKeyName(i) == "cell")
                {
                    const YamlNode *cellNode = rootNode->Get(i);
                    const YamlNode *cellType = cellNode->Get("type");
                    const YamlNode *cellScale = cellNode->Get("scale");
                    const YamlNode *cellCoords = cellNode->Get("coords");

                    TextureSheetCell c;

                    if(NULL != cellType)
                    {
                        c.geometryId = cellType->AsUInt32();
                    }

                    if(NULL != cellScale)
                    {
                        c.geometryScale = cellScale->AsVector2();
                    }

                    for(int j = 0; j < cellCoords->GetCount(); ++j)
                    {
                        if(j < MAX_CELL_TEXTURE_COORDS)
                        {
                            const YamlNode *singleCellCoord = cellCoords->Get(j);
                            c.coords[j] = singleCellCoord->AsVector2();
                        }
                        else
                        {
                            DVASSERT(0 && "Too much vertexes");
                        }
                    }

                    cells.push_back(c);
                }
            }
        }

        parser->Release();
    }
}

void VegetationRenderObject::SetVisibilityDistance(const Vector2& distances)
{
    visibleClippingDistances = distances;
}
    
const Vector2& VegetationRenderObject::GetVisibilityDistance() const
{
    return visibleClippingDistances;
}
    
void VegetationRenderObject::ResetVisibilityDistance()
{
    visibleClippingDistances.x = MAX_VISIBLE_CLIPPING_DISTANCE;
    visibleClippingDistances.y = MAX_VISIBLE_SCALING_DISTANCE;
}
    
void VegetationRenderObject::SetLodRange(const Vector3& distances)
{
    lodRanges = distances;
    
    InitLodRanges();
}
    
const Vector3& VegetationRenderObject::GetLodRange() const
{
   return lodRanges;
}
    
void VegetationRenderObject::ResetLodRanges()
{
   lodRanges = LOD_RANGES_SCALE;
   
   InitLodRanges();
}

void VegetationRenderObject::InitLodRanges()
{
    Vector2 smallestUnitSize = GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]);
    
    RESOLUTION_RANGES[0].x = lodRanges.x * smallestUnitSize.x;
    RESOLUTION_RANGES[0].y = lodRanges.y * smallestUnitSize.x;

    RESOLUTION_RANGES[1].x = lodRanges.y * smallestUnitSize.x;
    RESOLUTION_RANGES[1].y = lodRanges.z * smallestUnitSize.x;

    RESOLUTION_RANGES[2].x = lodRanges.z * smallestUnitSize.x;
    RESOLUTION_RANGES[2].y = RESOLUTION_RANGES[2].x * 1000.0f;

    
    for(uint32 i = 0; i < COUNT_OF(RESOLUTION_RANGES); ++i)
    {
        RESOLUTION_RANGES[i].x *= RESOLUTION_RANGES[i].x;
        RESOLUTION_RANGES[i].y *= RESOLUTION_RANGES[i].y;
    }
}

void VegetationRenderObject::SetMaxVisibleQuads(const uint32& _maxVisibleQuads)
{
    maxVisibleQuads = _maxVisibleQuads;
}

const uint32& VegetationRenderObject::GetMaxVisibleQuads() const
{
    return maxVisibleQuads;
}

};