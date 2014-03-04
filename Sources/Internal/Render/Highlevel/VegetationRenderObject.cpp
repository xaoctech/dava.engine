//
//  GrassRenderObject.cpp
//  Framework
//
//  Created by Valentine Ivanov on 2/24/14.
//
//

#include "Render/Image.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/VegetationRenderObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Utils/Random.h"

namespace DAVA
{
    
static const FastName UNIFORM_TILEPOS = FastName("tilePos");
static const FastName UNIFORM_WORLD_SIZE = FastName("worldSize");
static const FastName UNIFORM_CLUSTER_SCALE_MAP = FastName("clusterScaleMap");
static const FastName UNIFORM_CLUSTER_DENSITY_MAP = FastName("clusterDensityMap");

static const FastName UNIFORM_SAMPLER_VEGETATIONMAP = FastName("vegetationmap");
    
static const uint32 MAX_VEGETATION_LAYERS = 3;
static const uint32 MAX_VERTEX_PER_CLUSTER = 12;
static const uint32 MAX_INDEX_PER_CLUSTER = 18;
static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
static const float32 CLUSTER_SCALE_NORMALIZATION_VALUE = 3.0f;
    
static const uint32 MAX_BRUSH_DIVERSITY = 4;
    
static const size_t MAX_RENDER_CELLS = 40;
static const float32 MAX_VISIBLE_DISTANCE = 180.0f * 180.0f; //meters * meters (square length)
    
static const float32 MAX_ROTATION_ANGLE = 0.0f;
static const Vector3 MAX_DISPLACEMENT = Vector3(0.0f, 0.0f, 0.0f);
    
    
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
    
int32 RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}

    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    clusterLimit(0)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_VEGETATION;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
    
    renderBatchPoolLine = 0;
    
    vegetationMaterial = NMaterial::CreateMaterial(FastName("Vegetation_Material"),
                                                           NMaterialName::GRASS,
                                                           NMaterial::DEFAULT_QUALITY_NAME);
}

VegetationRenderObject::~VegetationRenderObject()
{
    SafeRelease(vegetationMap);
    
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
    
    vegetationRenderObject->SetHeightmap(vegetationMaterial->GetTexture(NMaterial::TEXTURE_DETAIL));
    vegetationRenderObject->SetVegetationMap(vegetationMap);
    vegetationRenderObject->SetTextureSheet(textureSheet);
    vegetationRenderObject->SetClusterLimit(clusterLimit);

    return vegetationRenderObject;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
  
void VegetationRenderObject::PrepareToRender(Camera *camera)
{
    DVASSERT(clusterBrushes.size() > 0);
    
    visibleCells.clear();
    BuildVisibleCellList(camera->GetPosition(), camera->GetFrustum(), visibleCells);
    std::stable_sort(visibleCells.begin(), visibleCells.end(), CellByDistanceCompareFunction);
    
    uint32 requestedBatchCount = Min(visibleCells.size(), MAX_RENDER_CELLS);
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
    
    Matrix4 scaleMap;
    Matrix4 densityMap;
    
    for(size_t i = 0; i < requestedBatchCount; ++i)
    {
        RenderBatch* rb = GetRenderBatch(i);
        NMaterial* mat = rb->GetMaterial();
        SpatialData* spatialData = visibleCells[i];
        
        Vector4 posScale(spatialData->bbox.min.x,
                         spatialData->bbox.min.y,
                         worldSize.x,
                         worldSize.y);

        scaleMap.Zero();
        densityMap.Zero();

        for(uint32 layerIndex = 0; layerIndex < MAX_VEGETATION_LAYERS; ++layerIndex)
        {
            uint32 cellLayerData = 0x000000FF & (spatialData->cellDescription >> ((MAX_VEGETATION_LAYERS - layerIndex) * 8));
            
            int32 clusterType = (cellLayerData >> 6);
            float32 clusterScale = (1.0f * ((cellLayerData >> 4) & 0x00000003)) / CLUSTER_SCALE_NORMALIZATION_VALUE;
            float32 density = (1.0f * (cellLayerData & 0x0000000F)) + 1.0f; //step function uses "<" so we need to emulate "<="

            densityMap._data[layerIndex][clusterType] = density;
            scaleMap._data[layerIndex][clusterType] = clusterScale;
        }
        
        mat->SetPropertyValue(UNIFORM_TILEPOS, Shader::UT_FLOAT_VEC4, 1, posScale.data);
        mat->SetPropertyValue(UNIFORM_CLUSTER_SCALE_MAP, Shader::UT_FLOAT_MAT4, 1, scaleMap.data);
        mat->SetPropertyValue(UNIFORM_CLUSTER_DENSITY_MAP, Shader::UT_FLOAT_MAT4, 1, densityMap.data);
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
        
        if(IsValidData())
        {
            unitWorldSize = GetVegetationUnitWorldSize();
            
            if(vegetationMap)
            {
                BuildSpatialStructure(vegetationMap);
            }
        }
    }
}
    
VegetationMap* VegetationRenderObject::GetVegetationMap() const
{
    return vegetationMap;
}
    
void VegetationRenderObject::SetTextureSheet(const TextureSheet& sheet)
{
    textureSheet = sheet;
    
    vegetationMaterial->SetTexture(NMaterial::TEXTURE_ALBEDO,
                                   textureSheet.GetTexture());
    
    if(IsValidData())
    {
        BuildVegetationBrush(clusterLimit);
    }
}
    
const TextureSheet& VegetationRenderObject::GetTextureSheet() const
{
    return textureSheet;
}
    
void VegetationRenderObject::SetClusterLimit(uint32 maxClusters)
{
    clusterLimit = maxClusters;
    
    if(IsValidData())
    {
        BuildVegetationBrush(clusterLimit);
    }
}

uint32 VegetationRenderObject::GetClusterLimit() const
{
    return clusterLimit;
}

void VegetationRenderObject::SetHeightmap(Texture* _heightmap)
{
    vegetationMaterial->SetTexture(NMaterial::TEXTURE_DETAIL, _heightmap);
}
    
void VegetationRenderObject::BuildVegetationBrush(uint32 maxClusters)
{
    DVASSERT(maxClusters > 0);
    DVASSERT(textureSheet.cells.size() > 0);
    
    if(clusterBrushes.size() < MAX_BRUSH_DIVERSITY)
    {
        uint32 difference = MAX_BRUSH_DIVERSITY - clusterBrushes.size();
        for(uint32 i = 0; i < difference; ++i)
        {
            clusterBrushes.push_back(new PolygonGroup());
        }
    }
    
    uint32 clusterTypeCount = textureSheet.cells.size();
    Vector2 unitSize = GetVegetationUnitWorldSize();
    
    uint32 totalClustersPerUnit = (maxClusters * maxClusters);
    
    Vector2 clusterOffset(unitSize.x / maxClusters,
                          unitSize.y / maxClusters);
    
    Vector2 clusterTypeOffset(clusterOffset.x / clusterTypeCount,
                              clusterOffset.y / clusterTypeCount);
    
    Vector3 normal(0.0f, 0.0f, 1.0f); //up
    
    Vector<uint32> shuffleDensity;
    shuffleDensity.reserve(totalClustersPerUnit);
    for(uint32 i = 0; i < totalClustersPerUnit; ++i)
    {
        shuffleDensity.push_back((i % MAX_DENSITY_LEVELS) + 1);
    }
    
    for(uint32 brushIndex = 0; brushIndex < MAX_BRUSH_DIVERSITY; ++brushIndex)
    {
        PolygonGroup* clusterBrush = clusterBrushes[brushIndex];
        clusterBrush->ReleaseData();
        
        
        clusterBrush->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_BINORMAL | EVF_TANGENT | EVF_TEXCOORD0,
                                   MAX_VERTEX_PER_CLUSTER * totalClustersPerUnit * MAX_VEGETATION_LAYERS * clusterTypeCount,
                                   MAX_INDEX_PER_CLUSTER * totalClustersPerUnit * MAX_VEGETATION_LAYERS * clusterTypeCount);
        
        uint32 vertexIndex = 0;
        uint32 indexIndex = 0;
        
        for(uint32 layerIndex = 0; layerIndex < MAX_VEGETATION_LAYERS; ++layerIndex)
        {
            for(uint32 clusterTypeIndex = 0; clusterTypeIndex < clusterTypeCount; ++clusterTypeIndex)
            {
                uint32 geometryType = textureSheet.cells[clusterTypeIndex].geometryId;
                std::random_shuffle(shuffleDensity.begin(), shuffleDensity.end(), RandomShuffleFunc);
                
                for(uint32 clusterIndex = 0; clusterIndex < totalClustersPerUnit; ++clusterIndex)
                {
                    float32 clusterX = (clusterIndex % maxClusters) * clusterOffset.x + clusterTypeIndex * clusterTypeOffset.x;
                    float32 clusterY = (clusterIndex / maxClusters) * clusterOffset.y + clusterTypeIndex * clusterTypeOffset.y;
                    
                    uint32 densityId = shuffleDensity[clusterIndex];
                    
                    float32 randomRotation = MAX_ROTATION_ANGLE * (float32)Random::Instance()->RandFloat();
                    float32 randomDisplacementX = MAX_DISPLACEMENT.x * Random::Instance()->RandFloat();
                    float32 randomDisplacementY = MAX_DISPLACEMENT.y * Random::Instance()->RandFloat();
                    float32 randomDisplacementZ = MAX_DISPLACEMENT.z * Random::Instance()->RandFloat();
                    
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
                    
                    for(uint32 i = 0; i < vertexDataSize; ++i)
                    {
                        Vector3 vertexCoord = Vector3(vertexData[i].x * textureSheet.cells[clusterTypeIndex].geometryScale.x + clusterCenter.x,
                                                       vertexData[i].y * textureSheet.cells[clusterTypeIndex].geometryScale.x + clusterCenter.y,
                                                       vertexData[i].z * textureSheet.cells[clusterTypeIndex].geometryScale.y + clusterCenter.z);
                        
                        
                        clusterBrush->SetCoord(vertexIndex, vertexCoord);
                        clusterBrush->SetBinormal(vertexIndex, clusterCenter);
                        clusterBrush->SetTexcoord(0, vertexIndex, textureSheet.cells[clusterTypeIndex].coords[i % MAX_CELL_TEXTURE_COORDS]);
                        clusterBrush->SetTangent(vertexIndex, Vector3(layerIndex, clusterTypeIndex, densityId));
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
        
        uint32 randNum = Random::Instance()->Rand();
        rb->SetPolygonGroup(clusterBrushes[randNum % clusterBrushes.size()]);
        
        NMaterial* batchMaterial = NMaterial::CreateMaterialInstance();
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
    
bool VegetationRenderObject::IsValidData() const
{
    return (clusterLimit > 0) &&
            (textureSheet.cells.size() > 0) &&
            (worldSize.Length() > 0);
}
    
Vector4 VegetationRenderObject::GetVisibleArea(Camera* cam)
{
    return Vector4(0, 0, 1, 1);
}

void VegetationRenderObject::SetWorldSize(const Vector3 size)
{
    worldSize = size;
    
    vegetationMaterial->SetPropertyValue(UNIFORM_WORLD_SIZE,
                                         Shader::UT_FLOAT_VEC3,
                                         1,
                                         &worldSize);
    
    if(IsValidData())
    {
        BuildVegetationBrush(clusterLimit);
        
        unitWorldSize = GetVegetationUnitWorldSize();
        
        if(vegetationMap)
        {
            BuildSpatialStructure(vegetationMap);
        }
    }
}
    
const Vector3& VegetationRenderObject::GetWorldSize() const
{
    return worldSize;
}

Vector2 VegetationRenderObject::GetVegetationUnitWorldSize() const
{
    DVASSERT(vegetationMap);
    return Vector2(worldSize.x / vegetationMap->width, worldSize.y / vegetationMap->height);
}
    
void VegetationRenderObject::BuildSpatialStructure(VegetationMap* vegMap)
{
    DVASSERT(vegMap);
    DVASSERT(IsPowerOf2(vegMap->GetWidth()));
    DVASSERT(IsPowerOf2(vegMap->GetHeight()));
    DVASSERT(vegMap->GetWidth() == vegMap->GetHeight());
    
    uint32 mapSize = vegMap->GetWidth();
    uint32 treeDepth = FastLog2(mapSize);
    
    quadTree.Init(treeDepth);
    AbstractQuadTreeNode<SpatialData>* node = quadTree.GetRoot();
    
    uint32 halfSize = mapSize >> 1;
    BuildSpatialQuad(node, -1 * halfSize, -1 * halfSize, mapSize, mapSize);
}
    
void VegetationRenderObject::BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          int16 x, int16 y,
                          uint16 width, uint16 height)
{
    DVASSERT(node);
    
    node->data.bbox.AddPoint(Vector3(-1.0f * x * unitWorldSize.x, -1.0f * y * unitWorldSize.y, -1.0f * worldSize.z));
    node->data.bbox.AddPoint(Vector3(-1.0f * (x + width) * unitWorldSize.x, -1.0f * (y + height) * unitWorldSize.y, worldSize.z));
    node->data.refPoint = node->data.bbox.GetCenter();
    
    node->data.x = (1 == width) ? x : -1;
    node->data.y = (1 == height) ? y : -1;
    node->data.cellDescription = (1 == width) ? 0x3F7FBFFF : 0; //TODO: init with density from map
    
    if(width > 1 && height > 1)
    {
        int16 halfWidth = width >> 1;
        int16 halfHeight = height >> 1;
        
        BuildSpatialQuad(node->children[0], x, y, halfWidth, halfHeight);
        BuildSpatialQuad(node->children[1], x + halfWidth, y, halfWidth, halfHeight);
        BuildSpatialQuad(node->children[2], x, y + halfHeight, halfWidth, halfHeight);
        BuildSpatialQuad(node->children[3], x + halfWidth, y + halfHeight, halfWidth, halfHeight);
    }
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  Vector<SpatialData*>& cellList)
{
    uint8 planeMask = 0x3F;
    BuildVisibleCellList(cameraPoint, frustum, planeMask, quadTree.GetRoot(), cellList);
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  uint8& planeMask,
                                                  AbstractQuadTreeNode<SpatialData>* node,
                                                  Vector<SpatialData*>& cellList)
{
    if(node)
    {
        Frustum::eFrustumResult result = frustum->Classify(node->data.bbox, planeMask, node->data.clippingPlane);
        if(Frustum::EFR_OUTSIDE != result)
        {
            if(node->IsTerminalLeaf())
            {
                AddVisibleCell(cameraPoint, &(node->data), MAX_VISIBLE_DISTANCE, cellList);
            }
            else
            {
                if(frustum->IsFullyInside(node->data.bbox))
                {
                    AddAllVisibleCells(cameraPoint, node, cellList);
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
}
    
void VegetationRenderObject::AddAllVisibleCells(const Vector3& cameraPoint,
                                                AbstractQuadTreeNode<SpatialData>* node,
                                                Vector<SpatialData*>& cellList)
{
    if(node)
    {
        if(node->IsTerminalLeaf())
        {
            AddVisibleCell(cameraPoint, &(node->data), MAX_VISIBLE_DISTANCE, cellList);
        }
        else
        {
            AddAllVisibleCells(cameraPoint, node->children[0], cellList);
            AddAllVisibleCells(cameraPoint, node->children[1], cellList);
            AddAllVisibleCells(cameraPoint, node->children[2], cellList);
            AddAllVisibleCells(cameraPoint, node->children[3], cellList);
        }
    }
}
    
bool VegetationRenderObject::CellByDistanceCompareFunction(const SpatialData* a,
                                                           const SpatialData* b)
{
    return (a->cameraDistance < b->cameraDistance);
}

};