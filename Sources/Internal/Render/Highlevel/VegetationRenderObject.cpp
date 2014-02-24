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

namespace DAVA
{
    
static const uint32 MAX_VEGETATION_LAYERS = 3;
static const uint32 MAX_VERTEX_PER_CLUSTER = 12;
static const uint32 MAX_INDEX_PER_CLUSTER = 18;
static const uint32 MAX_CLUSTER_TYPES = 4;
    
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
        
};
    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    clusterLimit(0),
    heightmap(NULL),
    clusterBrush(NULL)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_CUSTOM_DRAW;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
}

VegetationRenderObject::~VegetationRenderObject()
{
    SafeRelease(vegetationMap);
    SafeRelease(heightmap);
    SafeRelease(clusterBrush);
}
    
RenderObject* VegetationRenderObject::Clone(RenderObject *newObject)
{
    return NULL;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
  
void VegetationRenderObject::PrepareToRender(Camera *camera)
{
        
}
    
void VegetationRenderObject::SetVegetationMap(VegetationMap* map)
{
    if(map != vegetationMap)
    {
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
}
    
const TextureSheet& VegetationRenderObject::GetTextureSheet() const
{
    return textureSheet;
}
    
void VegetationRenderObject::SetClusterLimit(uint32 maxClusters)
{
    clusterLimit = maxClusters;
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
        heightmap = SafeRetain(_heightmap);
    }
}

Heightmap* VegetationRenderObject::GetHeightmap() const
{
    return heightmap;
}
    
void VegetationRenderObject::BuildGrassBrush(uint32 maxClusters)
{
    DVASSERT(vegetationMap);
    DVASSERT(clusterLimit > 0);
    
    SafeRelease(clusterBrush);
    
    clusterBrush = new PolygonGroup();
    
    clusterBrush->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_TEXCOORD0,
                               MAX_VEGETATION_LAYERS * MAX_VERTEX_PER_CLUSTER * clusterLimit,
                               MAX_VEGETATION_LAYERS * MAX_INDEX_PER_CLUSTER * clusterLimit);
    
    uint32 clusterPerRow = sqrt(clusterLimit);
    uint32 maxClusterCount = clusterPerRow * MAX_VEGETATION_LAYERS;
    for(uint32 clusterIndex = 0; clusterIndex < maxClusterCount; ++clusterIndex)
    {
        uint32 clusterType = clusterIndex % MAX_CLUSTER_TYPES;
        uint32 clusterBaseLine = clusterIndex / clusterPerRow; //y
        uint32 clusterBaselinePosition = clusterIndex - clusterPerRow * clusterBaseLine; //x
        uint32 clusterOffsetFromBaseline = clusterBaselinePosition % 2; //y offset
        
        
    }
}

};