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

#ifndef __DAVAENGINE_VEGETATIONRENDEROBJECT_H__
#define __DAVAENGINE_VEGETATIONRENDEROBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/AbstractQuadTree.h"
#include "Render/Image.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFile/SerializationContext.h"

#include "Render/Highlevel/Vegetation/RenderBatchPool.h"
#include "Render/Highlevel/Vegetation/VegetationRenderData.h"
#include "Render/Highlevel/Vegetation/VegetationSpatialData.h"
#include "Render/Highlevel/Vegetation/VegetationCustomGeometrySerializationData.h"

namespace DAVA
{
    
typedef Image VegetationMap;
class Heightmap;
class VegetationGeometry;
class VegetationCustomGeometrySerializationData;

struct VegetationMetrics
{
    Vector<uint32> visibleInstanceCountPerLayer;
    Vector<uint32> visibleInstanceCountPerLOD;
    Vector<uint32> instanceCountPerLOD;
    Vector<uint32> instanceCountPerLayer;
    
    Vector<uint32> visiblePolyCountPerLayer;
    Vector<uint32> visiblePolyCountPerLOD;
    Vector<uint32> polyCountPerLOD;
    Vector<uint32> polyCountPerLayer;
    
    Vector<Vector<uint32> > polyCountPerLayerPerLod; //layer-lod

    uint32 totalQuadTreeLeafCount;
    Vector<uint32> quadTreeLeafCountPerLOD;
    
    uint32 renderBatchCount;
};

class VegetationRenderObject : public RenderObject
{
public:
        
    VegetationRenderObject();
    virtual ~VegetationRenderObject();
        
    RenderObject * Clone(RenderObject *newObject);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);
    void PrepareRenderData();
    
    virtual void PrepareToRender(Camera *camera);
    virtual void RecalcBoundingBox();
    
    void SetHeightmap(Heightmap* _heightmap);
    Heightmap* GetHeightmap() const;
    const FilePath& GetHeightmapPath() const;
    void SetHeightmapPath(const FilePath& path);

    void SetVegetationMap(VegetationMap* map);
    void SetVegetationMap(const FilePath& path);
    VegetationMap* GetVegetationMap() const;
    void SetVegetationMapPath(const FilePath& path);
    const FilePath& GetVegetationMapPath() const;
    
    void SetLightmap(const FilePath& filePath);
    const FilePath& GetLightmapPath() const;
    
    void SetTextureSheet(const FilePath& path);
    const FilePath& GetTextureSheetPath() const;
    
    void SetVegetationTexture(const FilePath& texture);
    const FilePath& GetVegetationTexture() const;
    
    void SetClusterLimit(const uint32& maxClusters);
    uint32 GetClusterLimit() const;
    
    void SetWorldSize(const Vector3 size);
    const Vector3& GetWorldSize() const;
    
    void SetVisibilityDistance(const Vector2& distances);
    const Vector2& GetVisibilityDistance() const;
    void ResetVisibilityDistance();
    
    void SetLodRange(const Vector3& distances);
    const Vector3& GetLodRange() const;
    void ResetLodRanges();
    
    void SetMaxVisibleQuads(const uint32& _maxVisibleQuads);
    const uint32& GetMaxVisibleQuads() const;
    
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    
    void SetPerturbation(const Vector3& point, const Vector3& force, float32 distance);
    float32 GetPerturbationDistance() const;
    const Vector3& GetPerturbationForce() const;
    void SetPerturbationPoint(const Vector3& point);
    const Vector3& GetPerturbationPoint() const;
    
    void SetLayerVisibilityMask(const uint8& mask);
    const uint8& GetLayerVisibilityMask() const;
    
    void SetVegetationVisible(bool show);
    bool GetVegetationVisible() const;
    
    void DebugDrawVisibleNodes();
    
    const FilePath& GetCustomGeometryPath() const;
    void SetCustomGeometryPath(const FilePath& path);
    
    void SetCameraBias(const float32& bias) {cameraBias = bias;}
    float32 GetCameraBias() const {return cameraBias;}
    
    void SetLayerClusterLimit(const Vector4& maxClusters);
    Vector4 GetLayerClusterLimit() const;
    
    void CollectMetrics(VegetationMetrics& metrics);
    
private:
    
    bool IsValidGeometryData() const;
    bool IsValidSpatialData() const;
    
    Vector2 GetVegetationUnitWorldSize(float32 resolution) const;
    
    void BuildSpatialStructure(VegetationMap* vegMap);
    void BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          AbstractQuadTreeNode<SpatialData>* firstRenderableParent,
                          int16 x, int16 y,
                          uint16 width, uint16 height,
                          AABBox3& parentBox);
    
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              Vector<AbstractQuadTreeNode<SpatialData>*>& cellList);
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              uint8 planeMask,
                              AbstractQuadTreeNode<SpatialData>* node,
                              Vector<AbstractQuadTreeNode<SpatialData>*>& cellList,
                              bool evaluateVisibility);
    inline void AddVisibleCell(AbstractQuadTreeNode<SpatialData>* node,
                               float32 refDistance,
                               uint32 cellValue,
                               Vector<AbstractQuadTreeNode<SpatialData>*>& cellList);
    
    static bool CellByDistanceCompareFunction(const AbstractQuadTreeNode<SpatialData>* a, const AbstractQuadTreeNode<SpatialData>*  b);
    
    void InitHeightTextureFromHeightmap(Heightmap* heightMap);
    
    float32 SampleHeight(int16 x, int16 y);
    
    void UpdateVegetationSetup();
    void InitLodRanges();
    
    void SetupHeightmapParameters(BaseObject * caller, void * param, void *callerData);
    
    void CreateRenderData();
    
    bool ReadyToRender(bool externalRenderFlag);
    
    size_t SelectDirectionIndex(Camera* cam, Vector<SortedBufferItem>& buffers);
    
    inline uint32 MapToResolution(float32 squareDistance);
    
    inline bool IsNodeEmpty(AbstractQuadTreeNode<SpatialData>* node,
                            uint32 maxClusterTypes,
                            float32 clusterScaleNormalizationValue,
                            const VegetationMap& map) const;
    inline uint8* GetCellValue(int x, int y, const VegetationMap& map) const;
    inline void GetLayerDescription(uint8 cellLayerData,
                                    float32 clusterScaleNormalizationValue,
                                    float32& density,
                                    float32& scale) const;
    
    void ClearRenderBatches();
    
    void InitWithFixedGeometry(FastNameSet& materialFlags);
    void InitWithCustomGeometry(FastNameSet& materialFlags);
    
    void SetCustomGeometryPathInternal(const FilePath& path);
    void ImportDataFromExternalScene(const FilePath& path);
    VegetationCustomGeometrySerializationData* LoadCustomGeometryData(SerializationContext* context, KeyedArchive* srcArchive);
    void SaveCustomGeometryData(SerializationContext* context, KeyedArchive* dstArchive, VegetationCustomGeometrySerializationData* data);
    
private:
    
    Heightmap* heightmap;
    VegetationMap* vegetationMap;
    Vector3 worldSize;
    Vector<Vector2> unitWorldSize;
    Vector2 heightmapScale;
    Vector2 heightmapToVegetationMapScale;
    uint16 halfWidth;
    uint16 halfHeight;
    
    RenderBatchPool renderBatchPool;
    
    //Vector<float32> shaderScaleDensityUniforms;
    
    Vector<VegetationRenderData*> renderData;
    
    AbstractQuadTree<SpatialData> quadTree;
    Vector<AbstractQuadTreeNode<SpatialData>*> visibleCells;
    
    FilePath heightmapPath;
    FilePath vegetationMapPath;
    FilePath textureSheetPath;
    FilePath albedoTexturePath;
    FilePath lightmapTexturePath;
    
    FilePath customGeometryPath;
    
    Vector2 visibleClippingDistances;
    Vector3 lodRanges;
    uint32 maxVisibleQuads;
    
    Vector3 perturbationForce;
    Vector3 perturbationPoint;
    float32 maxPerturbationDistance;
    
    uint8 layerVisibilityMask;
    
    bool vegetationVisible;
    
    Vector<Vector2> resolutionRanges;
    
    VegetationGeometry* vegetationGeometry;
    
    Texture* heightmapTexture;
    
    float32 cameraBias;
    Vector<uint32> clustersPerLayer;
    
    VegetationCustomGeometrySerializationData* customGeometryData;
    
public:
    
    INTROSPECTION_EXTEND(VegetationRenderObject, RenderObject,
                         PROPERTY("density", "Base density", GetLayerClusterLimit, SetLayerClusterLimit, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("densityMap", "Density map", GetVegetationMapPath, SetVegetationMap, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lightmap", "Lightmap", GetLightmapPath, SetLightmap, I_SAVE | I_EDIT | I_VIEW)
                         //PROPERTY("textureSheet", "Texture sheet", GetTextureSheetPath, SetTextureSheet, I_SAVE | I_EDIT | I_VIEW)
                         //PROPERTY("vegetationTexture", "Vegetation texture", GetVegetationTexture, SetVegetationTexture, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lodRanges", "Lod ranges", GetLodRange, SetLodRange, I_EDIT | I_VIEW)
                         PROPERTY("visibilityDistance", "Visibility distances", GetVisibilityDistance, SetVisibilityDistance, I_EDIT | I_VIEW)
                         PROPERTY("maxVisibleQuads", "Max visible quads", GetMaxVisibleQuads, SetMaxVisibleQuads, I_EDIT | I_VIEW)
                         PROPERTY("customGeometry", "Custom geometry", GetCustomGeometryPath, SetCustomGeometryPath, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("cameraBias", "Camera Bias", GetCameraBias, SetCameraBias, I_EDIT | I_VIEW)
                         );
    
};
    
inline void VegetationRenderObject::AddVisibleCell(AbstractQuadTreeNode<SpatialData>* node,
                                                   float32 refDistance,
                                                   uint32 cellValue,
                                                   Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
{
    if(node->data.isVisible && node->data.cameraDistance <= refDistance)
    {
        cellList.push_back(node);
    }
}

inline uint32 VegetationRenderObject::MapToResolution(float32 squareDistance)
{
    uint32 resolutionId = 0;
        
    size_t rangesCount = resolutionRanges.size();
    for(size_t i = 0; i < rangesCount; ++i)
    {
        if(squareDistance > resolutionRanges[i].x &&
            squareDistance <= resolutionRanges[i].y)
        {
            resolutionId = (uint32)i;
            break;
        }
    }
    
    return resolutionId;
}

inline bool VegetationRenderObject::IsNodeEmpty(AbstractQuadTreeNode<SpatialData>* node,
                                                uint32 maxClusterTypes,
                                                float32 clusterScaleNormalizationValue,
                                                const VegetationMap& map) const
{
    if(node->data.x == -1)
    {
        int ddd = 0;
        ddd++;
    }
    
    bool nodeEmpty = true;
    
    int32 maxX = node->data.x + node->data.width;
    int32 maxY = node->data.y + node->data.height;
    
    for(int32 y = node->data.y; y < maxY; ++y)
    {
        for(int32 x = node->data.x; x < maxX; ++x)
        {
            uint8* vegetationMapValuePtr = GetCellValue(x, y, *vegetationMap);
            for(uint32 clusterType = 0; clusterType < maxClusterTypes; ++clusterType)
            {
                uint8 cellLayerData = vegetationMapValuePtr[clusterType];
                
                float32 clusterScale = 0.0f;
                float32 density = 0.0f;
                GetLayerDescription(cellLayerData, clusterScaleNormalizationValue, density, clusterScale);
                
                if(clusterScale > 0.0f &&
                   density > 1.0f)
                {
                    nodeEmpty = false;
                    break;
                }
            }
        }
    }
    
    return nodeEmpty;
}

inline uint8* VegetationRenderObject::GetCellValue(int x, int y, const VegetationMap& map) const
{
    int32 mapX = x + halfWidth;
    int32 mapY = y + halfHeight;
    uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;
    
    uint8* vegetationMapValuePtr = (map.data + cellDescriptionIndex * 4);

    return vegetationMapValuePtr;
    
    //static uint32 dummy = 0xFFFFFFFF;
    //return (uint8*)&dummy;
}

inline void VegetationRenderObject::GetLayerDescription(uint8 cellLayerData,
                                                        float32 clusterScaleNormalizationValue,
                                                        float32& density,
                                                        float32& scale) const
{
    scale = (1.0f * ((cellLayerData >> 4) & 0xF)) / clusterScaleNormalizationValue;
    density = (1.0f * (cellLayerData & 0xF)) + 1.0f; //step function uses "<" so we need to emulate "<="
}

};

#endif
