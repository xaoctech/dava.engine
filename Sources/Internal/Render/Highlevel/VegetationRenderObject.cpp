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

#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/VegetationRenderObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Utils/Random.h"
#include "Render/ImageLoader.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
    
static const FastName UNIFORM_TILEPOS = FastName("tilePos");
static const FastName UNIFORM_WORLD_SIZE = FastName("worldSize");
static const FastName UNIFORM_CLUSTER_SCALE_DENSITY_MAP = FastName("clusterScaleDensityMap[0]");
static const FastName UNIFORM_HEIGHTMAP_SCALE = FastName("heightmapScale");
static const FastName UNIFORM_SWITCH_LOD_SCALE = FastName("lodSwitchScale");
static const FastName UNIFORM_PERTURBATION_FORCE = FastName("perturbationForce");
static const FastName UNIFORM_PERTURBATION_POINT = FastName("perturbationPoint");
static const FastName UNIFORM_PERTURBATION_FORCE_DISTANCE = FastName("perturbationForceDistance");
static const FastName UNIFORM_BILLBOARD_DIRECTION = FastName("billboardDirection");

static const FastName FLAG_FRAMEBUFFER_FETCH = FastName("FRAMEBUFFER_FETCH");
static const FastName FLAG_BILLBOARD_DRAW = FastName("MATERIAL_GRASS_BILLBOARD");

static const FastName VEGETATION_QUALITY_NAME_HIGH = FastName("HIGH");
static const FastName VEGETATION_QUALITY_NAME_LOW = FastName("LOW");
static const FastName VEGETATION_QUALITY_GROUP_NAME = FastName("Vegetation");

static const FastName UNIFORM_SAMPLER_VEGETATIONMAP = FastName("vegetationmap");
    
static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
static const float32 CLUSTER_SCALE_NORMALIZATION_VALUE = 15.0f;
    
    
static const size_t MAX_RENDER_CELLS = 512;
static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 70.0f * 70.0f; //meters * meters (square length)
static const float32 MAX_VISIBLE_SCALING_DISTANCE = 50.0f * 50.0f;

//static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 130.0f * 130.0f; //meters * meters (square length)
//static const float32 MAX_VISIBLE_SCALING_DISTANCE = 100.0f * 100.0f;

    
static const uint32 FULL_BRUSH_VALUE = 0xFFFFFFFF;

static const Vector3 MAX_DISPLACEMENT = Vector3(5.6f, 5.6f, 0.0f);

static const uint32 SORT_DIRECTION_COUNT = 8;
    
    
/*static const Vector3 CLUSTER_TYPE_0[] =
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

static const Vector3 CLUSTER_TYPE_0_NORMALS[] =
{
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
};

static const Vector3 CLUSTER_TYPE_1_NORMALS[] =
{
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
        
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
        
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
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

static const Vector3* VEGETATION_CLUSTER_NORMALS[] =
{
    CLUSTER_TYPE_0_NORMALS,
    CLUSTER_TYPE_1_NORMALS
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
};*/

    static const Vector3 CLUSTER_TYPE_0[] =
    {
        Vector3(-0.5f, 0.0f, 1.0f),
        Vector3(0.5f, 0.0f, 1.0f),
        Vector3(0.5f, 0.0f, 0.0f),
        Vector3(-0.5f, 0.0f, 0.0f),
    };
    
    
    static const Vector3 CLUSTER_TYPE_1[] =
    {
        Vector3(-0.5f, 0.1f, 1.0f),
        Vector3(0.5f, 0.1f, 1.0f),
        Vector3(0.5f, 0.1f, 0.0f),
        Vector3(-0.5f, 0.1f, 0.0f),
    };
    
    static const Vector3 CLUSTER_TYPE_0_NORMALS[] =
    {
        Vector3(0.0f, 0.0f, 1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
        Vector3(0.0f, 0.0f, 1.0f),
    };
    
    static const Vector3 CLUSTER_TYPE_1_NORMALS[] =
    {
        Vector3(0.0f, 0.0f, 1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
        Vector3(0.0f, 0.0f, 1.0f),
    };
    
    
    static const int16 CLUSTER_INDICES[] =
    {
        0, 3,  1, 1, 3,  2
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
    
    static const Vector3* VEGETATION_CLUSTER_NORMALS[] =
    {
        CLUSTER_TYPE_0_NORMALS,
        CLUSTER_TYPE_1_NORMALS
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

static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 6.0f);
//static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 12.0f);

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

static uint32 RESOLUTION_TILES_PER_ROW[] =
{
    4,
    2,
    1
};

static float32 RESOLUTION_DISTANCE_SCALE_COMPENSATION[] =
{
    1.0f,
    1.4f,
    1.4f
};

//#define VEGETATION_DRAW_LOD_COLOR

static Color RESOLUTION_COLOR[] =
{
    Color(0.5f, 0.0f, 0.0f, 1.0f),
    Color(0.0f, 0.5f, 0.0f, 1.0f),
    Color(0.0f, 0.0f, 0.5f, 1.0f),
};

#ifdef VEGETATION_DRAW_LOD_COLOR
static const FastName UNIFORM_LOD_COLOR = FastName("lodColor");
static const FastName FLAG_VEGETATION_DRAW_LOD_COLOR = FastName("VEGETATION_DRAW_LOD_COLOR");
#endif
    
int32 RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
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

    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    heightmap(NULL),
    clusterLimit(0),
    halfWidth(0),
    halfHeight(0),
    vertexRenderDataObject(NULL),
    maxPerturbationDistance(1000000.0f),
    layerVisibilityMask(0xFF),
    vegetationVisible(true),
    maxLayerHeight(1.0f),
    useLowCameraScale(false)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_VEGETATION;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
    
    unitWorldSize.resize(COUNT_OF(RESOLUTION_SCALE));
    resolutionRanges.resize(COUNT_OF(RESOLUTION_INDEX));
    
    uint32 maxParams = 4 * 2 * RESOLUTION_CELL_SQUARE[COUNT_OF(RESOLUTION_CELL_SQUARE) - 1];
    shaderScaleDensityUniforms.resize(maxParams);
    
    renderBatchPoolLine = 0;
    
    vegetationMaterial = NMaterial::CreateMaterial(FastName("Vegetation_Material"),
                                                           NMaterialName::GRASS,
                                                           NMaterial::DEFAULT_QUALITY_NAME);
    vegetationMaterial->AddNodeFlags(DataNode::NodeRuntimeFlag);
    if(RenderManager::Instance()->GetCaps().isFramebufferFetchSupported)
    {
        vegetationMaterial->SetFlag(FLAG_FRAMEBUFFER_FETCH, NMaterial::FlagOn);
    }
    
    vegetationMaterial->SetFlag(FLAG_BILLBOARD_DRAW, NMaterial::FlagOn);
    
#ifdef VEGETATION_DRAW_LOD_COLOR

    vegetationMaterial->SetFlag(FLAG_VEGETATION_DRAW_LOD_COLOR, NMaterial::FlagOn);
    
#endif
    
    maxVisibleQuads = MAX_RENDER_CELLS;
    lodRanges = LOD_RANGES_SCALE;
    ResetVisibilityDistance();
}

VegetationRenderObject::~VegetationRenderObject()
{
    SafeRelease(vegetationMap);
    SafeRelease(heightmap);
    
    ReleaseRenderData();
    
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
    vegetationRenderObject->SetUseLowCameraScale(GetUseLowCameraScale());
    
    vegetationRenderObject->AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    vegetationRenderObject->AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);


    return vegetationRenderObject;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    //VI: need to remove render batches since they are temporary
    int32 batchesToRemove = GetRenderBatchCount();
    while(GetRenderBatchCount())
    {
        RemoveRenderBatch(GetRenderBatchCount() - 1);
    }
    ReturnToPool(batchesToRemove);
    
    RenderObject::Save(archive, serializationContext);
    
    archive->SetUInt32("vro.clusterLimit", GetClusterLimit());
    archive->SetBool("vro.useLowCameraScale", GetUseLowCameraScale());

	if(vegetationMapPath.IsEmpty() == false)
	{
		archive->SetString("vro.vegmap", vegetationMapPath.GetRelativePathname(serializationContext->GetScenePath()));
	}

	if(textureSheetPath.IsEmpty() == false)
	{
		archive->SetString("vro.texturesheet", textureSheetPath.GetRelativePathname(serializationContext->GetScenePath()));
	}

	const FilePath & albedoPath = vegetationMaterial->GetTexturePath(NMaterial::TEXTURE_ALBEDO);
	if(albedoPath.IsEmpty() == false)
	{
		archive->SetString("vro.vegtexture", albedoPath.GetRelativePathname(serializationContext->GetScenePath()));
	}

	const FilePath & mapPath = vegetationMaterial->GetTexturePath(UNIFORM_SAMPLER_VEGETATIONMAP);
	if(mapPath.IsEmpty() == false)
	{
		archive->SetString("vro.lightmap", mapPath.GetRelativePathname(serializationContext->GetScenePath()));
	}
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderObject::Load(archive, serializationContext);
    
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    
    bool shouldLoadData = deviceCaps.isVertexTextureUnitsSupported;
    
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    shouldLoadData = shouldLoadData && qualityAllowsVegetation;
    
    RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::VEGETATION_DRAW, shouldLoadData);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WIN32__)
    shouldLoadData = true;
#endif
    
    if(shouldLoadData)
    {
        SetClusterLimit(archive->GetUInt32("vro.clusterLimit"));

		String vegmap = archive->GetString("vro.vegmap");
		if(vegmap.empty() == false)
		{
			SetVegetationMap(serializationContext->GetScenePath() + vegmap);
		}

		String texturesheet = archive->GetString("vro.texturesheet");
		if(texturesheet.empty() == false)
		{
			SetTextureSheet(serializationContext->GetScenePath() + texturesheet);
		}

		String vegtexture = archive->GetString("vro.vegtexture");
		if(vegtexture.empty() == false)
		{
			SetVegetationTexture(serializationContext->GetScenePath() + vegtexture);
		}

		String lightmap = archive->GetString("vro.lightmap");
		if(lightmap.empty() == false)
		{
			SetLightmap(serializationContext->GetScenePath() + lightmap);
		}
        
        bool useScale = archive->GetBool("vro.useLowCameraScale", false);
        SetUseLowCameraScale(useScale);
    }
    
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
}

void VegetationRenderObject::PrepareToRender(Camera *camera)
{
    bool renderFlag = RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::VEGETATION_DRAW);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WIN32__)
//VI: case when vegetation was turned off and then qualit changed from low t high is not a real-world scenario
//VI: real-world scenario is in resource editor when quality has been changed.
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    renderFlag = (renderFlag && qualityAllowsVegetation);
#endif

    visibleCells.clear();
    uint32 currentBatchCount = GetRenderBatchCount();
    
    if(!ReadyToRender(renderFlag))
    {
        if(currentBatchCount > 0)
        {
            while(currentBatchCount > 0)
            {
                RemoveRenderBatch(GetRenderBatchCount() - 1);
                currentBatchCount = GetRenderBatchCount();
            }
            
            ReturnToPool(currentBatchCount);
        }
        
        return;
    }
    
    BuildVisibleCellList(camera->GetPosition(), camera->GetFrustum(), visibleCells);
    
    std::sort(visibleCells.begin(), visibleCells.end(), CellByDistanceCompareFunction);
    
    uint32 requestedBatchCount = Min(visibleCells.size(), (size_t)maxVisibleQuads);

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
    Vector2 switchLodScale;
    
    //Vector3 billboardDirection = -1.0f * camera->GetLeft();
    //billboardDirection.Normalize();
    //vegetationMaterial->SetPropertyValue(UNIFORM_BILLBOARD_DIRECTION,
    //                                     Shader::UT_FLOAT_VEC3,
    //                                     1,
    //                                     billboardDirection.data);
    
    for(size_t i = 0; i < requestedBatchCount; ++i)
    {
        AbstractQuadTreeNode<SpatialData>* treeNode = visibleCells[i];
        
        RenderBatch* rb = GetRenderBatch(i);
        
        NMaterial* mat = rb->GetMaterial();
        
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);
        
        Vector<Vector<SortedBufferItem> >& rdoVector = indexRenderDataObject[resolutionIndex];
        
        uint32 indexBufferIndex = treeNode->data.rdoIndex;
        
        DVASSERT(indexBufferIndex >= 0 && indexBufferIndex < rdoVector.size());
        
        size_t directionIndex = SelectDirectionIndex(camera, rdoVector[indexBufferIndex]);
        rb->SetRenderDataObject(rdoVector[indexBufferIndex][directionIndex].rdo);
        
        float32 cameraLowScale = 1.0f;
        bool cameraLowPosition = false;
        
        if(useLowCameraScale)
        {
            Vector3 nodeCenter = treeNode->data.bbox.GetCenter();
            float32 cameraHeight = Abs(camera->GetPosition().z - nodeCenter.z);
            cameraLowPosition = (cameraHeight < maxLayerHeight);
            if(cameraLowPosition)
            {
                cameraLowScale = cameraHeight / maxLayerHeight;
            }
        }
        
        SetupNodeUniforms(treeNode, treeNode, treeNode->data.cameraDistance, cameraLowPosition, cameraLowScale, shaderScaleDensityUniforms);
        
        posScale.x = treeNode->data.bbox.min.x - unitWorldSize[resolutionIndex].x * (indexBufferIndex % RESOLUTION_TILES_PER_ROW[resolutionIndex]);
        posScale.y = treeNode->data.bbox.min.y - unitWorldSize[resolutionIndex].y * (indexBufferIndex / RESOLUTION_TILES_PER_ROW[resolutionIndex]);
        
        switchLodScale.x = resolutionIndex;
        switchLodScale.y = Clamp(1.0f - (treeNode->data.cameraDistance / resolutionRanges[resolutionIndex].y), 0.0f, 1.0f);
        
        
        mat->SetPropertyValue(UNIFORM_SWITCH_LOD_SCALE,
                              Shader::UT_FLOAT_VEC2,
                              1,
                              switchLodScale.data);
        
        mat->SetPropertyValue(UNIFORM_TILEPOS,
                              Shader::UT_FLOAT_VEC4,
                              1,
                              posScale.data);
        
        mat->SetPropertyValue(UNIFORM_CLUSTER_SCALE_DENSITY_MAP,
                              Shader::UT_FLOAT,
                              shaderScaleDensityUniforms.size(),
                              &shaderScaleDensityUniforms[0]);
        
#ifdef VEGETATION_DRAW_LOD_COLOR
        mat->SetPropertyValue(UNIFORM_LOD_COLOR, Shader::UT_FLOAT_VEC3, 1, &RESOLUTION_COLOR[resolutionIndex]);
#endif
    }

}

void VegetationRenderObject::SetVegetationMap(VegetationMap* map)
{
    if(map != vegetationMap)
    {
        SafeRelease(vegetationMap);
        vegetationMap = SafeRetain(map);
        
        UpdateVegetationSetup();
    }
}

void VegetationRenderObject::SetVegetationMap(const FilePath& path)
{
    if(path.Exists())
    {
        Vector<Image*> images;
        ImageLoader::CreateFromFileByExtension(path, images);
            
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
        
        if(false == RenderManager::Instance()->GetCaps().isFramebufferFetchSupported)
        {
            NMaterialHelper::EnableStateFlags(DAVA::PASS_FORWARD,
                                              batchMaterial,
                                              RenderStateData::STATE_BLEND);
        }

        
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
    BuildSpatialQuad(node, NULL, -1 * halfSize, -1 * halfSize, mapSize, mapSize, node->data.bbox);
}
    
void VegetationRenderObject::BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          AbstractQuadTreeNode<SpatialData>* firstRenderableParent,
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
        node->data.isVisible = !IsNodeEmpty(node,
                                            MAX_CLUSTER_TYPES,
                                            CLUSTER_SCALE_NORMALIZATION_VALUE,
                                            *vegetationMap);
        
        if(width == RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1])
        {
            firstRenderableParent = node;
            node->data.rdoIndex = 0;
        }
        else
        {
            int16 offsetX = abs(node->data.x - firstRenderableParent->data.x) / width;
            int16 offsetY = abs(node->data.y - firstRenderableParent->data.y) / height;
            
            node->data.rdoIndex = offsetX + (offsetY * (firstRenderableParent->data.width / width));
        }
    }
    else
    {
        node->data.width = -1;
        node->data.height = -1;
        node->data.rdoIndex = -1;
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
        
        BuildSpatialQuad(node->children[0], firstRenderableParent, x, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[1], firstRenderableParent, x + cellHalfWidth, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[2], firstRenderableParent, x, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[3], firstRenderableParent, x + cellHalfWidth, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
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
                                                  Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
{
    static Vector3 corners[8];
    if(node)
    {
        Frustum::eFrustumResult result = frustum->Classify(node->data.bbox, planeMask, node->data.clippingPlane);
        if(Frustum::EFR_OUTSIDE != result)
        {
            if(node->data.IsRenderable())
            {
                node->data.bbox.GetCorners(corners);
                float32 refDistance = FLT_MAX;
                for(uint32 cornerIndex = 0; cornerIndex < COUNT_OF(corners); ++cornerIndex)
                {
                    corners[cornerIndex].z = 0.0f;
                    float32 cornerDistance = (cameraPoint - corners[cornerIndex]).SquareLength();
                    if(cornerDistance < refDistance)
                    {
                        refDistance = cornerDistance;
                    }
                }
                
                node->data.cameraDistance = refDistance;
                
                uint32 resolutionId = MapToResolution(node->data.cameraDistance);
                if(node->IsTerminalLeaf() ||
                   RESOLUTION_CELL_SQUARE[resolutionId] >= node->data.GetResolutionId())
                {
                    int32 mapX = node->data.x + halfWidth;
                    int32 mapY = node->data.y + halfHeight;
                    uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;

                    uint32 vegetationMapValue = (node->IsTerminalLeaf()) ? (*(((uint32*)vegetationMap->data) + cellDescriptionIndex)) : FULL_BRUSH_VALUE;
                    AddVisibleCell(node, MAX_VISIBLE_CLIPPING_DISTANCE,
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
        
bool VegetationRenderObject::CellByDistanceCompareFunction(const AbstractQuadTreeNode<SpatialData>* a,
                                                           const AbstractQuadTreeNode<SpatialData>*  b)
{
    return (a->data.cameraDistance > b->data.cameraDistance);
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
        tx = Texture::CreateFromData(FORMAT_RGBA4444, croppedImage->GetData(), pow2Size, pow2Size, false);
     
        SafeRelease(croppedImage);
    }
    else
    {
        tx = Texture::CreateFromData(FORMAT_RGBA4444, originalImage->GetData(), pow2Size, pow2Size, false);
    }
    
    SafeRelease(originalImage);
    
    heightmapScale = Vector2((1.0f * heightmap->Size()) / pow2Size,
                             (1.0f * heightmap->Size()) / pow2Size);
    
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &VegetationRenderObject::SetupHeightmapParameters, tx));
	JobInstanceWaiter waiter(job);
	waiter.Wait();
    
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
        CreateRenderData(clusterLimit);
    }
    
    if(IsValidSpatialData())
    {
        BuildSpatialStructure(vegetationMap);
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
    
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}
    
const Vector3& VegetationRenderObject::GetLodRange() const
{
   return lodRanges;
}
    
void VegetationRenderObject::ResetLodRanges()
{
   lodRanges = LOD_RANGES_SCALE;
 
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}

void VegetationRenderObject::InitLodRanges()
{
    Vector2 smallestUnitSize = GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]);
    
    resolutionRanges[0].x = lodRanges.x * smallestUnitSize.x;
    resolutionRanges[0].y = lodRanges.y * smallestUnitSize.x;

    resolutionRanges[1].x = lodRanges.y * smallestUnitSize.x;
    resolutionRanges[1].y = lodRanges.z * smallestUnitSize.x;

    resolutionRanges[2].x = lodRanges.z * smallestUnitSize.x;
    resolutionRanges[2].y = MAX_VISIBLE_CLIPPING_DISTANCE;//RESOLUTION_RANGES[2].x * 1000.0f;

    size_t resolutionCount = resolutionRanges.size();
    for(size_t i = 0; i < resolutionCount; ++i)
    {
        resolutionRanges[i].x *= resolutionRanges[i].x;
        resolutionRanges[i].y *= resolutionRanges[i].y;
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

void VegetationRenderObject::GetDataNodes(Set<DataNode*> & dataNodes)
{
     dataNodes.insert(vegetationMaterial);
}

void VegetationRenderObject::SetupHeightmapParameters(BaseObject * caller,
                                                    void * param,
                                                    void *callerData)
{
    Texture* tx = (Texture*)param;
    tx->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
    tx->SetMinMagFilter(Texture::FILTER_NEAREST, Texture::FILTER_NEAREST);
}

void VegetationRenderObject::CreateRenderData(uint32 maxClusters)
{
    DVASSERT(maxClusters > 0);
    DVASSERT(textureSheet.cells.size() > 0);
    
    InitLodRanges();
    
    ReleaseRenderData();
    
    uint32 tilesPerRow = (uint32)RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1];
    uint32 maxClusterRowSize = (tilesPerRow * maxClusters);
    size_t maxTotalClusters = maxClusterRowSize * maxClusterRowSize;
    
    uint32 layerDataCount = 0;
    uint32 indexDataCount = 0;
    maxLayerHeight = 0.0f;
    for(uint32 layerIndex = 0; layerIndex < MAX_CLUSTER_TYPES; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        layerDataCount += VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        indexDataCount += VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
        
        if(cellData.geometryScale.y > maxLayerHeight)
        {
            maxLayerHeight = cellData.geometryScale.y;
        }
    }
    
    uint32 totalIndexCount = 0;
    for(uint32 i = 0; i < COUNT_OF(RESOLUTION_SCALE); ++i)
    {
        totalIndexCount += indexDataCount * (maxTotalClusters / (uint32)RESOLUTION_SCALE[i]);
    }
    
    totalIndexCount *= SORT_DIRECTION_COUNT;
    
    indexData.resize(totalIndexCount);
    vertexData.resize(maxTotalClusters * layerDataCount);
    
    Vector<uint32> layerOffsets(MAX_CLUSTER_TYPES);
    
    GenerateVertices(maxClusters, maxTotalClusters, maxClusterRowSize, tilesPerRow, GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]), layerOffsets);
    
    GenerateIndices(maxClusters, maxClusterRowSize, layerOffsets);
    
    //VI: need to build vertex & index objects AFTER initialization
    GenerateRenderDataObjects();
}
    
void VegetationRenderObject::ReleaseRenderData()
{
    size_t indexBufferResolutionCount = indexRenderDataObject.size();
    for(size_t indexBufferIndex = 0; indexBufferIndex < indexBufferResolutionCount; ++indexBufferIndex)
    {
        Vector<Vector<SortedBufferItem> >& indexBufferArray = indexRenderDataObject[indexBufferIndex];
        size_t indexObjectCount = indexBufferArray.size();
        for(size_t i = 0; i < indexObjectCount; ++i)
        {
            Vector<SortedBufferItem>& directionArray = indexBufferArray[i];
            
            size_t directionBufferCount = directionArray.size();
            for(size_t directionIndex = 0; directionIndex < directionBufferCount; ++directionIndex)
            {
                directionArray[directionIndex].rdo->DetachVertices();
            }
            
            directionArray.clear();
        }
        
        indexBufferArray.clear();
    }
    indexRenderDataObject.clear();
    
    SafeRelease(vertexRenderDataObject);
    
    vertexData.clear();
    indexData.clear();
}

bool VegetationRenderObject::ReadyToRender(bool externalRenderFlag)
{
    return vegetationVisible && (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::VEGETATION_DRAW)) && (vertexRenderDataObject != NULL);
}

void VegetationRenderObject::SetupNodeUniforms(AbstractQuadTreeNode<SpatialData>* sourceNode,
                                               AbstractQuadTreeNode<SpatialData>* node,
                                               float32 cameraDistance,
                                               bool cameraLowPosition,
                                               float32 cameraLowScale,
                                               Vector<float32>& uniforms)
{
    if(node->IsTerminalLeaf())
    {
        DVASSERT(node->data.rdoIndex >= 0 && node->data.rdoIndex < uniforms.size());
        
        //int32 mapX = node->data.x + halfWidth;
        //int32 mapY = node->data.y + halfHeight;
        //uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;
        
        float32 distanceScale = 1.0f;
        
        if(cameraDistance > MAX_VISIBLE_SCALING_DISTANCE)
        {
            distanceScale = Clamp(1.0f - ((cameraDistance - MAX_VISIBLE_SCALING_DISTANCE) / (MAX_VISIBLE_CLIPPING_DISTANCE - MAX_VISIBLE_SCALING_DISTANCE)), 0.0f, 1.0f);
        }
        
        uint8 *vegetationMapValuePtr = GetCellValue(node->data.x, node->data.y, *vegetationMap);//(vegetationMap->data + cellDescriptionIndex * 4);
        
        for(uint32 clusterType = 0; clusterType < MAX_CLUSTER_TYPES; ++clusterType)
        {
            uint8 cellLayerData = vegetationMapValuePtr[clusterType];
            
            float32 clusterScale = 0.0f;
            float32 density = 0.0f;
            GetLayerDescription(cellLayerData, CLUSTER_SCALE_NORMALIZATION_VALUE, density, clusterScale);
            
            if(cameraLowPosition &&
               sourceNode == node)
            {
                clusterScale *= cameraLowScale;
            }
            
            uniforms[node->data.rdoIndex * 2 * 4 + clusterType] = density;
            uniforms[node->data.rdoIndex * 2 * 4 + 4 + clusterType] = clusterScale;
        }

    }
    else
    {
        SetupNodeUniforms(sourceNode, node->children[0], cameraDistance, cameraLowPosition, cameraLowScale, uniforms);
        SetupNodeUniforms(sourceNode, node->children[1], cameraDistance, cameraLowPosition, cameraLowScale, uniforms);
        SetupNodeUniforms(sourceNode, node->children[2], cameraDistance, cameraLowPosition, cameraLowScale, uniforms);
        SetupNodeUniforms(sourceNode, node->children[3], cameraDistance, cameraLowPosition, cameraLowScale, uniforms);
    }
}

void VegetationRenderObject::SetPerturbation(const Vector3& point,
                                            const Vector3& force,
                                            float32 distance)
{
    perturbationForce = force;
    maxPerturbationDistance = distance;
    perturbationPoint = point;
    
    vegetationMaterial->SetPropertyValue(UNIFORM_PERTURBATION_FORCE, Shader::UT_FLOAT_VEC3, 1, perturbationForce.data);
    vegetationMaterial->SetPropertyValue(UNIFORM_PERTURBATION_FORCE_DISTANCE, Shader::UT_FLOAT, 1, &maxPerturbationDistance);
    vegetationMaterial->SetPropertyValue(UNIFORM_PERTURBATION_POINT, Shader::UT_FLOAT_VEC3, 1, perturbationPoint.data);
}

float32 VegetationRenderObject::GetPerturbationDistance() const
{
   return maxPerturbationDistance;
}

const Vector3& VegetationRenderObject::GetPerturbationForce() const
{
    return perturbationForce;
}

const Vector3& VegetationRenderObject::GetPerturbationPoint() const
{
    return perturbationPoint;
}

void VegetationRenderObject::SetPerturbationPoint(const Vector3& point)
{
    perturbationPoint = point;
    vegetationMaterial->SetPropertyValue(UNIFORM_PERTURBATION_POINT, Shader::UT_FLOAT_VEC3, 1, perturbationPoint.data);
}

void VegetationRenderObject::SetLayerVisibilityMask(const uint8& mask)
{
    layerVisibilityMask = mask;
}
    
const uint8& VegetationRenderObject::GetLayerVisibilityMask() const
{
     return layerVisibilityMask;
}

void VegetationRenderObject::SetVegetationVisible(bool show)
{
    vegetationVisible = show;
}
    
bool VegetationRenderObject::GetVegetationVisible() const
{
    return vegetationVisible;
}

bool VegetationRenderObject::PolygonByDistanceCompareFunction(const PolygonSortData& a, const PolygonSortData&  b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}

size_t VegetationRenderObject::SelectDirectionIndex(Camera* cam, Vector<SortedBufferItem>& buffers)
{
    Vector3 cameraDirection = cam->GetDirection();
    cameraDirection.Normalize();
    
    size_t index = 0;
    float32 currentCosA = 0.0f;
    size_t directionCount = buffers.size();
    for(size_t i = 0; i < directionCount; ++i)
    {
        SortedBufferItem& item = buffers[i];
        //cos (angle) = dotAB / (length A * lengthB)
        //no need to calculate (length A * lengthB) since vectors are normalized
        
        if(item.sortDirection == cameraDirection)
        {
            index = i;
            break;
        }
        else
        {
            float32 cosA = cameraDirection.DotProduct(item.sortDirection);
            if(cosA > currentCosA)
            {
                index = i;
                currentCosA = cosA;
            }
        }
    }
    
    return index;
}

void VegetationRenderObject::SetupCameraPositions(const AABBox3& bbox, Vector<Vector3>& positions)
{
    float32 z = bbox.min.z + (bbox.max.z - bbox.min.z) * 0.5f;

    positions[0] = Vector3(bbox.min.x, bbox.min.y, z);
    positions[1] = Vector3(bbox.min.x + (bbox.max.x - bbox.min.x) * 0.5f, bbox.min.y, z);
    positions[2] = Vector3(bbox.max.x, bbox.min.y, z);
    positions[3] = Vector3(bbox.max.x, bbox.min.y + (bbox.max.y - bbox.min.y) * 0.5f, z);
    positions[4] = Vector3(bbox.max.x, bbox.max.y, z);
    positions[5] = Vector3(bbox.min.x + (bbox.max.x - bbox.min.x) * 0.5f, bbox.max.y, z);
    positions[6] = Vector3(bbox.min.x, bbox.max.y, z);
    positions[7] = Vector3(bbox.min.x, bbox.min.y + (bbox.max.y - bbox.min.y) * 0.5f, z);
}

void VegetationRenderObject::SetUseLowCameraScale(const bool& useScale)
{
    useLowCameraScale = useScale;
}
    
bool VegetationRenderObject::GetUseLowCameraScale() const
{
    return useLowCameraScale;
}

void VegetationRenderObject::GenerateVertices(uint32 maxClusters,
                                              size_t maxTotalClusters,
                                              uint32 maxClusterRowSize,
                                              uint32 tilesPerRow,
                                              Vector2 unitSize,
                                              Vector<uint32>& layerOffsets)
{
    uint32 clustersPerTile = maxClusters * maxClusters;
    float32 atomicOffsetY = unitSize.y / clustersPerTile;
    
    Vector<uint32> shuffleDepth;
    shuffleDepth.reserve(maxTotalClusters);
    
    Vector<uint32> shuffleDensity;
    shuffleDensity.reserve(maxTotalClusters);
    
    Vector<uint32> depthScratchArray;
    depthScratchArray.reserve(clustersPerTile);
    
    Vector<uint32> densityScratchArray;
    densityScratchArray.reserve(clustersPerTile);
    
    for(uint32 i = 0; i < clustersPerTile; ++i)
    {
        depthScratchArray[i] = i;
        densityScratchArray[i] = (i % MAX_DENSITY_LEVELS) + 1;
    }
    
    size_t vertexIndex = 0;
    uint32 totalTiles = tilesPerRow * tilesPerRow;
    for(uint32 layerIndex = 0; layerIndex < MAX_CLUSTER_TYPES; ++layerIndex)
    {
        layerOffsets[layerIndex] = vertexIndex;
        
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const Vector3* clusterVertices = VEGETATION_CLUSTER[cellData.geometryId];
        const Vector3* clusterNormals = VEGETATION_CLUSTER_NORMALS[cellData.geometryId];
        uint32 clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        uint32 clusterCounter = 0;
        for(uint32 i = 0; i < totalTiles; ++i)
        {
            std::random_shuffle(densityScratchArray.begin(), densityScratchArray.end(), RandomShuffleFunc);
            std::random_shuffle(depthScratchArray.begin(), depthScratchArray.end(), RandomShuffleFunc);
            for(uint32 k = 0; k < clustersPerTile; ++k)
            {
                shuffleDensity[clusterCounter] = densityScratchArray[k];
                shuffleDepth[clusterCounter] = depthScratchArray[k];
                clusterCounter++;
            }
        }
        
        DVASSERT(clusterCounter == maxTotalClusters);
        
        AABBox3 clusterBBox;
        for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
        {
            clusterBBox.AddPoint(clusterVertices[clusterVertexIndex]);
        }
        
        float32 clusterVisualSize = cellData.geometryScale.x * (clusterBBox.max.x - clusterBBox.min.x);
        uint32 clustersInRow = unitSize.x / clusterVisualSize;
        if(clustersInRow <= 0)
        {
            clustersInRow = 1;
        }
        
        for(size_t clusterIndex = 0; clusterIndex < maxTotalClusters; ++clusterIndex)
        {
            uint32 clusterIndexX = clusterIndex % maxClusterRowSize;
            uint32 clusterIndexY = clusterIndex / maxClusterRowSize;
            
            uint32 matrixIndex = (clusterIndexX / maxClusters) + tilesPerRow * (clusterIndexY / maxClusters); //0...15
            DVASSERT(matrixIndex >= 0 && matrixIndex < (tilesPerRow * tilesPerRow));
            
            uint32 matrixIndexX = matrixIndex % tilesPerRow;
            uint32 matrixIndexY = matrixIndex / tilesPerRow;
            
            Vector2 matrixCellStart(unitSize.x * matrixIndexX, unitSize.y * matrixIndexY);
            
            float32 randomOffsetX = clusterVisualSize * (clusterIndex % clustersInRow) + clusterVisualSize * (0.5f - Random::Instance()->RandFloat());//unitSize.x * Random::Instance()->RandFloat();
            float32 randomOffsetY = atomicOffsetY * shuffleDepth[clusterIndex]; //unitSize.y * Random::Instance()->RandFloat();
            
            Vector3 clusterCenter(matrixCellStart.x + randomOffsetX, matrixCellStart.y + randomOffsetY, 0.0f);
            
            uint32 densityId = shuffleDensity[clusterIndex];
            
            for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
            {
                DVASSERT(vertexIndex < vertexData.size());
                VegetationVertex& vertex = vertexData[vertexIndex];
                
                vertex.coord.x = clusterCenter.x + clusterVertices[clusterVertexIndex].x * cellData.geometryScale.x;
                vertex.coord.y = clusterCenter.y + clusterVertices[clusterVertexIndex].y * cellData.geometryScale.x;
                vertex.coord.z = clusterCenter.z + clusterVertices[clusterVertexIndex].z * cellData.geometryScale.y;
                
                vertex.normal = clusterNormals[clusterVertexIndex];
                
                vertex.binormal = clusterCenter;
                
                vertex.tangent.x = matrixIndex * 2.0f * 4.0f; //each cluster is described by 2 vectors
                vertex.tangent.y = layerIndex;
                vertex.tangent.z = densityId;
                
                vertex.texCoord0.x = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].x;
                vertex.texCoord0.y = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].y;
                
                vertexIndex++;
            }
        }
    }
}

void VegetationRenderObject::GenerateIndices(uint32 maxClusters,
                                             uint32 maxClusterRowSize,
                                             Vector<uint32>& layerOffsets)
{
    Vector<PolygonSortData> sortingArray(1);
    Vector<int16> preparedIndices;
    size_t polygonElementCount = COUNT_OF(sortingArray[0].indices);
    
    //generate indices
    size_t totalResolutionCount = resolutionRanges.size();
    size_t currentIndexIndex = 0;
    
    Vector<Vector3> directionPoints;
    directionPoints.resize(SORT_DIRECTION_COUNT);
    
    for(size_t resolutionIndex = 0; resolutionIndex < totalResolutionCount; ++resolutionIndex)
    {
        uint32 resolutionOffset = (uint32)RESOLUTION_SCALE[resolutionIndex];
        uint32 indexBufferCount = RESOLUTION_CELL_SQUARE[COUNT_OF(RESOLUTION_CELL_SQUARE) - 1] / RESOLUTION_CELL_SQUARE[resolutionIndex];
        
        indexRenderDataObject.push_back(Vector<Vector<SortedBufferItem> >());
        
        Vector<Vector<SortedBufferItem> >& currentResolutionIndexArray = indexRenderDataObject[resolutionIndex];
        
        for(uint32 i = 0; i < indexBufferCount; ++i)
        {
            AABBox3 indexBufferBBox;

            PrepareIndexBufferData(i,
                                   maxClusters,
                                   maxClusterRowSize,
                                   resolutionIndex,
                                   resolutionOffset,
                                   layerOffsets,
                                   preparedIndices,
                                   indexBufferBBox);
            
            SetupCameraPositions(indexBufferBBox, directionPoints);
            
            PrepareSortedIndexBufferVariations(currentIndexIndex,
                                               i,
                                               polygonElementCount,
                                               indexBufferBBox,
                                               directionPoints,
                                               currentResolutionIndexArray,
                                               sortingArray,
                                               preparedIndices);
        }
    }
}

void VegetationRenderObject::PrepareIndexBufferData(uint32 indexBufferIndex,
                                                    uint32 maxClusters,
                                                    uint32 maxClusterRowSize,
                                                    size_t resolutionIndex,
                                                    uint32 resolutionOffset,
                                                    Vector<uint32>& layerOffsets,
                                                    Vector<int16>& preparedIndices,
                                                    AABBox3& indexBufferBBox)
{
    preparedIndices.clear();
    
    uint32 startX = (indexBufferIndex % RESOLUTION_TILES_PER_ROW[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 startY = (indexBufferIndex / RESOLUTION_TILES_PER_ROW[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 endX = startX + maxClusters * resolutionOffset;
    uint32 endY = startY + maxClusters * resolutionOffset;
    
    for(uint32 layerIndex = 0; layerIndex < MAX_CLUSTER_TYPES; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const int16* clusterIndices = VEGETATION_CLUSTER_INDICES[cellData.geometryId];
        uint32 clusterIndexCount = VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
        uint32 clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        for(uint32 y = startY; y < endY; y += resolutionOffset)
        {
            for(uint32 x = startX; x < endX; x += resolutionOffset)
            {
                uint32 baseIndex = layerOffsets[layerIndex] + (y * maxClusterRowSize + x) * clusterVertexCount;
                
                for(uint32 clusterIndexIndex = 0; clusterIndexIndex < clusterIndexCount; ++clusterIndexIndex)
                {
                    size_t vertexIndex = baseIndex + clusterIndices[clusterIndexIndex];
                    
                    DVASSERT(vertexIndex >= 0 && vertexIndex < vertexData.size());
                    
                    VegetationVertex& vertex = vertexData[vertexIndex];
                    vertex.texCoord1.x = resolutionIndex;
                    
                    indexBufferBBox.AddPoint(vertex.coord);
                    
                    preparedIndices.push_back(vertexIndex);
                }
            }
        }
    }
}

void VegetationRenderObject::PrepareSortedIndexBufferVariations(size_t& currentIndexIndex,
                                                                uint32 indexBufferIndex,
                                                                size_t polygonElementCount,
                                                                AABBox3& indexBufferBBox,
                                                                Vector<Vector3>& directionPoints,
                                                                Vector<Vector<SortedBufferItem> >& currentResolutionIndexArray,
                                                                Vector<PolygonSortData>& sortingArray,
                                                                Vector<int16>& preparedIndices)
{
    size_t sortItemCount = preparedIndices.size() / polygonElementCount;
    sortingArray.resize(sortItemCount);
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        PolygonSortData& sortData = sortingArray[sortItemIndex];
        
        sortData.indices[0] = preparedIndices[sortItemIndex * polygonElementCount + 0];
        sortData.indices[1] = preparedIndices[sortItemIndex * polygonElementCount + 1];
        sortData.indices[2] = preparedIndices[sortItemIndex * polygonElementCount + 2];
    }
    
    currentResolutionIndexArray.push_back(Vector<SortedBufferItem>());
    Vector<SortedBufferItem>& currentDirectionBuffers = currentResolutionIndexArray[indexBufferIndex];
    
    for(uint32 sortDirectionIndex = 0; sortDirectionIndex < SORT_DIRECTION_COUNT; ++sortDirectionIndex)
    {
        Vector3 cameraPosition = directionPoints[sortDirectionIndex];
        
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            sortData.cameraDistance = FLT_MAX;
            
            for(uint32 polygonIndex = 0; polygonIndex < polygonElementCount; ++polygonIndex)
            {
                float32 distance = (vertexData[sortData.indices[polygonIndex]].coord - cameraPosition).SquareLength();
                if(distance < sortData.cameraDistance)
                {
                    sortData.cameraDistance = distance;
                }
            }
        }
        
        std::stable_sort(sortingArray.begin(), sortingArray.end(), PolygonByDistanceCompareFunction);
        
        size_t prevIndexIndex = currentIndexIndex;
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            
            indexData[currentIndexIndex] = sortData.indices[0];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[1];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[2];
            currentIndexIndex++;
        }
        
        RenderDataObject* indexBuffer = new RenderDataObject();
        indexBuffer->SetIndices(EIF_16, (uint8*)(&indexData[prevIndexIndex]), (currentIndexIndex - prevIndexIndex));
        
        SortedBufferItem sortedBufferItem;
        sortedBufferItem.SetRenderDataObject(indexBuffer);
        sortedBufferItem.sortDirection = indexBufferBBox.GetCenter() - cameraPosition;
        sortedBufferItem.sortDirection.Normalize();
        
        currentDirectionBuffers.push_back(sortedBufferItem);
    }
}

void VegetationRenderObject::GenerateRenderDataObjects()
{
    vertexRenderDataObject = new RenderDataObject();
    vertexRenderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].coord));
    vertexRenderDataObject->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].normal));
    vertexRenderDataObject->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].binormal));
    vertexRenderDataObject->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].tangent));
    vertexRenderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord0));
    vertexRenderDataObject->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord1));
    vertexRenderDataObject->BuildVertexBuffer(vertexData.size(), true);
    
    size_t totalIndexObjectArrayCount = indexRenderDataObject.size();
    for(size_t indexArrayIndex = 0; indexArrayIndex < totalIndexObjectArrayCount; ++indexArrayIndex)
    {
        Vector<Vector<SortedBufferItem> >& indexObjectArray = indexRenderDataObject[indexArrayIndex];
        size_t totalIndexObjectCount = indexObjectArray.size();
        
        for(size_t i = 0; i < totalIndexObjectCount; ++i)
        {
            Vector<SortedBufferItem>& directionArray = indexObjectArray[i];
            size_t directionCount = directionArray.size();
            for(size_t directionIndex = 0; directionIndex < directionCount; ++directionIndex)
            {
                directionArray[directionIndex].rdo->BuildIndexBuffer(true);
                directionArray[directionIndex].rdo->AttachVertices(vertexRenderDataObject);
            }
        }
    }
}

void VegetationRenderObject::DebugDrawVisibleNodes()
{
    uint32 requestedBatchCount = Min(visibleCells.size(), (size_t)maxVisibleQuads);
    for(uint32 i = 0; i < requestedBatchCount; ++i)
    {
        AbstractQuadTreeNode<SpatialData>* treeNode = visibleCells[i];
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);
        
        RenderManager::Instance()->SetColor(RESOLUTION_COLOR[resolutionIndex]);
        RenderHelper::Instance()->DrawBox(treeNode->data.bbox, 1.0f, RenderState::RENDERSTATE_3D_OPAQUE);
    }
}

};