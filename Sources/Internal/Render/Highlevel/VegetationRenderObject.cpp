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
static const FastName UNIFORM_VEGETATIONLAYERS = FastName("vegetationLayers");
    
static const FastName UNIFORM_SAMPLER_VEGETATIONMAP = FastName("vegetationmap");
    
static const uint32 MAX_VEGETATION_LAYERS = 3;
static const uint32 MAX_VERTEX_PER_CLUSTER = 12;
static const uint32 MAX_INDEX_PER_CLUSTER = 18;
static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
    
static const uint32 MAX_BRUSH_DIVERSITY = 4;
    
static const float32 MAX_ROTATION_ANGLE = 30.0f;
static const Vector3 MAX_DISPLACEMENT = Vector3(0.5f, 0.5f, 0.0f);
    
static const Vector3 CLUSTER_TYPE_0[] =
{
    Vector3(-0.5f, 0.0f, 0.5f),
    Vector3(0.5f, 0.0f, 0.5f),
    Vector3(0.5f, 0.0f, -0.5f),
    Vector3(-0.5f, 0.0f, -0.5f),
    
    Vector3(-0.35f, -0.35f, 0.5f),
    Vector3(0.35f, 0.35f, 0.5f),
    Vector3(0.35f, 0.35f, -0.5f),
    Vector3(-0.35f, -0.35f, -0.5f),

    Vector3(-0.35f, 0.35f, 0.5f),
    Vector3(0.35f, -0.35f, 0.5f),
    Vector3(0.35f, -0.35f, -0.5f),
    Vector3(-0.35f, 0.35f, -0.5f),
};

static const Vector3 CLUSTER_TYPE_1[] =
{
    Vector3(-0.5f, 0.1f, 0.5f),
    Vector3(0.5f, 0.1f, 0.5f),
    Vector3(0.5f, 0.1f, -0.5f),
    Vector3(-0.5f, 0.1f, -0.5f),
    
    Vector3(-0.15f, -0.53f, 0.5f),
    Vector3(0.35f, 0.33f, 0.5f),
    Vector3(0.35f, 0.33f, -0.5f),
    Vector3(-0.15f, -0.53f, -0.5f),
    
    Vector3(-0.35f, 0.33f, 0.5f),
    Vector3(0.15f, -0.53f, 0.5f),
    Vector3(0.15f, -0.53f, -0.5f),
    Vector3(-0.35f, 0.33f, -0.5f),
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

    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    clusterLimit(0)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_CUSTOM_DRAW;
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
    
    Vector4 visibleArea = GetVisibleArea(camera);
    
    int32 visibleX = (int32)visibleArea.x;
    int32 visibleY = (int32)visibleArea.y;
    int32 visibleWidth = (int32)visibleArea.z;
    int32 visibleHeight = (int32)visibleArea.w;
    
    int32 requestedBatchCount = (int32)(visibleWidth * visibleHeight);
    int32 currentBatchCount = GetRenderBatchCount();
    
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

    int32 renderBatchIndex = 0;
    Vector2 vegetationUnitSize = GetVegetationUnitWorldSize();
    for(int32 y = visibleY; y < (visibleY + visibleHeight); ++y)
    {
        for(int32 x = visibleX; x < (visibleX + visibleWidth); ++x)
        {
            RenderBatch* rb = GetRenderBatch(renderBatchIndex);
            NMaterial* mat = rb->GetMaterial();
            
            Vector4 posScale(x * vegetationUnitSize.x,
                             y * vegetationUnitSize.y,
                             worldSize.x,
                             worldSize.y);
            mat->SetPropertyValue(UNIFORM_TILEPOS, Shader::UT_FLOAT_VEC4, 1, &posScale);
            
            renderBatchIndex++;
        }
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
    }
}
    
const VegetationMap* VegetationRenderObject::GetVegetationMap() const
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
    
    uint32 clusterPerRow = sqrt(maxClusters) * MAX_VEGETATION_LAYERS;
    uint32 totalRows = clusterPerRow; //maybe: clusterLimits / clusterPerRow
    uint32 maxClusterCount = clusterPerRow * totalRows;

    for(uint32 brushIndex = 0; brushIndex < MAX_BRUSH_DIVERSITY; ++brushIndex)
    {
        PolygonGroup* clusterBrush = clusterBrushes[brushIndex];
        clusterBrush->ReleaseData();
        
        
        clusterBrush->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_BINORMAL | EVF_TEXCOORD0 | EVF_TEXCOORD1,
                                   MAX_VERTEX_PER_CLUSTER * maxClusterCount ,
                                   MAX_INDEX_PER_CLUSTER * maxClusterCount);
        
        uint32 indicesIndex = 0;
        uint32 vertexIndex = 0;
        Vector3 normal(0.0f, 0.0f, 1.0f); //up
        
        float32 clusterOffsetX = unitSize.x / clusterPerRow;
        float32 clusterOffsetY = unitSize.y / totalRows;
        
        for(uint32 clusterIndex = 0; clusterIndex < maxClusterCount; ++clusterIndex)
        {
            uint32 layerId = clusterIndex % MAX_VEGETATION_LAYERS;
            uint32 clusterType = clusterIndex % clusterTypeCount;
            uint32 clusterBaseLine = clusterIndex / clusterPerRow; //y
            uint32 clusterBaselinePosition = clusterIndex - clusterPerRow * clusterBaseLine; //x
            uint32 clusterOffsetFromBaseline = clusterBaselinePosition % 2; //y additional offset
            
            float32 randomRotation = MAX_ROTATION_ANGLE * (float32)Random::Instance()->RandFloat();
            float32 randomDisplacementX = (float32)Random::Instance()->RandFloat();
            float32 randomDisplacementY = (float32)Random::Instance()->RandFloat();
            float32 randomDisplacementZ = (float32)Random::Instance()->RandFloat();
            
            Matrix4 transform = Matrix4::MakeRotation(normal, DegToRad(randomRotation)) *
                                Matrix4::MakeTranslation(Vector3(MAX_DISPLACEMENT.x * randomDisplacementX,
                                                                 MAX_DISPLACEMENT.y * randomDisplacementY,
                                                                 MAX_DISPLACEMENT.z * randomDisplacementZ));
            
            const int16* indexData = VEGETATION_CLUSTER_INDICES[clusterOffsetFromBaseline];
            uint32 indexDataSize = VEGETATION_CLUSTER_INDEX_SIZE[clusterOffsetFromBaseline];
            for(uint32 i = 0; i < indexDataSize; ++i)
            {
                clusterBrush->SetIndex(indicesIndex, vertexIndex + indexData[i]);
                indicesIndex++;
            }
            
            const Vector3* vertexData = VEGETATION_CLUSTER[clusterOffsetFromBaseline];
            uint32 vertexDataSize = VEGETATION_CLUSTER_SIZE[clusterOffsetFromBaseline];
            for(uint32 i = 0; i < vertexDataSize; ++i)
            {
                Vector3 clusterCenter = Vector3(clusterBaselinePosition * clusterOffsetX,
                                                clusterBaseLine * clusterOffsetY + clusterOffsetFromBaseline * clusterOffsetY * 0.5f,
                                                0.0f);
                
                Vector3 vertexCoord0 = Vector3(vertexData[i].x + clusterBaselinePosition * clusterOffsetX,
                                               vertexData[i].y + clusterBaseLine * clusterOffsetY + clusterOffsetFromBaseline * clusterOffsetY * 0.5f,
                                               vertexData[i].z);
                vertexCoord0 = vertexCoord0 * transform;
                
                
                clusterBrush->SetCoord(vertexIndex, vertexCoord0);
                clusterBrush->SetBinormal(vertexIndex, clusterCenter);
                clusterBrush->SetTexcoord(0, vertexIndex, textureSheet.cells[clusterType].coords[i % MAX_CELL_TEXTURE_COORDS]);
                clusterBrush->SetTexcoord(1, vertexIndex, Vector2(layerId, clusterIndex % MAX_DENSITY_LEVELS));
                clusterBrush->SetNormal(vertexIndex, clusterCenter);
                
                vertexIndex++;
            }
        }
        
        clusterBrush->BuildBuffers();
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
    return Vector4(-64, -64, 128, 128);
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
    
};