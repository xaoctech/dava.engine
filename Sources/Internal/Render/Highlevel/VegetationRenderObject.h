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

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFile/SerializationContext.h"

#define MAX_CELL_TEXTURE_COORDS 4

namespace DAVA
{
    
typedef Image VegetationMap;
    
struct TextureSheetCell
{
    Vector2 coords[MAX_CELL_TEXTURE_COORDS];
    uint32 geometryId;
    Vector2 geometryScale;
    
    inline TextureSheetCell();
    
    inline TextureSheetCell& operator=(const TextureSheetCell& src);
};
 
class Heightmap;
class TextureSheet
{
public:
    
    Vector<TextureSheetCell> cells;
    
    inline TextureSheet();
    inline ~TextureSheet();
    
    inline void SetTexture(Texture* tx);
    inline Texture* GetTexture() const;
    
    inline TextureSheet& operator=(const TextureSheet& src);
    
private:
    
    Texture* texture;
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
    
    void SetHeightmap(Texture* _heightmap);

    void SetVegetationMap(VegetationMap* map);
    VegetationMap* GetVegetationMap() const;
    
    void SetTextureSheet(const TextureSheet& sheet);
    const TextureSheet& GetTextureSheet() const;
    
    void SetClusterLimit(uint32 maxClusters);
    uint32 GetClusterLimit() const;
    
    void SetWorldSize(const Vector3 size);
    const Vector3& GetWorldSize() const;
    
private:
    
    struct SpatialData
    {
        int16 x;
        int16 y;
        AABBox3 bbox;
        uint32 cellDescription;
        Vector3 refPoint;
        float32 cameraDistance;
        uint8 clippingPlane;
        
        inline SpatialData();
        inline SpatialData& operator=(const SpatialData& src);
        inline bool IsEmpty() const;
    };
    
    void BuildVegetationBrush(uint32 maxClusters);
    RenderBatch* GetRenderBatchFromPool(NMaterial* material);
    void ReturnToPool(int32 batchCount);

    bool IsValidData() const;
    
    Vector4 GetVisibleArea(Camera* cam);
    Vector2 GetVegetationUnitWorldSize() const;
    
    void BuildSpatialStructure(VegetationMap* vegMap);
    void BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          int16 x, int16 y,
                          uint16 width, uint16 height);
    
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              Vector<SpatialData*>& cellList);
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              uint8& planeMask,
                              AbstractQuadTreeNode<SpatialData>* node,
                              Vector<SpatialData*>& cellList);
    void AddAllVisibleCells(const Vector3& cameraPoint,
                            AbstractQuadTreeNode<SpatialData>* node,
                            Vector<SpatialData*>& cellList);
    inline void AddVisibleCell(const Vector3& cameraPoint,
                               SpatialData* data,
                               float32 refDistance,
                               Vector<SpatialData*>& cellList);
    
    static bool CellByDistanceCompareFunction(const SpatialData* a, const SpatialData*  b);
    
private:
    
    VegetationMap* vegetationMap;
    TextureSheet textureSheet;
    uint32 clusterLimit;
    Vector3 worldSize;
    Vector2 unitWorldSize;
    
    Vector<RenderBatch*> renderBatchPool;
    int32 renderBatchPoolLine;
    
    NMaterial* vegetationMaterial;
    
    Vector<PolygonGroup*> clusterBrushes;
    
    AbstractQuadTree<SpatialData> quadTree;
    Vector<SpatialData*> visibleCells;
};
    
    
inline TextureSheetCell::TextureSheetCell() :
        geometryId(0),
        geometryScale(1.0f, 1.0f)
{
}
    
inline TextureSheetCell& TextureSheetCell::operator=(const TextureSheetCell& src)
{
    coords[0] = src.coords[0];
    coords[1] = src.coords[1];
    coords[2] = src.coords[2];
    coords[3] = src.coords[3];
    
    geometryId = src.geometryId;
    geometryScale = src.geometryScale;
    
    return *this;
}
    
inline TextureSheet::TextureSheet() : texture(NULL)
{
}
    
inline TextureSheet::~TextureSheet()
{
    SafeRelease(texture);
}
    
inline void TextureSheet::SetTexture(Texture* tx)
{
    if(tx != texture)
    {
        SafeRelease(texture);
        texture = SafeRetain(tx);
    }
}

inline Texture* TextureSheet::GetTexture() const
{
    return texture;
}
    
inline TextureSheet& TextureSheet::operator=(const TextureSheet& src)
{
    SetTexture(src.texture);
    cells.resize(src.cells.size());
    
    size_t size = cells.size();
    for(size_t i = 0; i < size; ++i)
    {
        cells[i] = src.cells[i];
    }
    
    return *this;
}
    
inline VegetationRenderObject::SpatialData::SpatialData()  :
        x(-1),
        y(-1),
        cellDescription(0),
        cameraDistance(0.0f),
        clippingPlane(0)
{
}
    
inline VegetationRenderObject::SpatialData& VegetationRenderObject::SpatialData::operator=(const VegetationRenderObject::SpatialData& src)
{
    x = src.x;
    y = src.y;
    bbox = src.bbox;
    cellDescription = src.cellDescription;
    cameraDistance = src.cameraDistance;
    refPoint = src.refPoint;
    clippingPlane = src.clippingPlane;
    
    return *this;
}

inline bool VegetationRenderObject::SpatialData::IsEmpty() const
{
    return (0 == (cellDescription & 0x0F0F0F00));
}
    
inline void VegetationRenderObject::AddVisibleCell(const Vector3& cameraPoint,
                                                   SpatialData* data,
                                                   float32 refDistance,
                                                   Vector<SpatialData*>& cellList)
{
    if(!data->IsEmpty())
    {
        Vector3 cameraVector = cameraPoint - data->refPoint;
        data->cameraDistance = cameraVector.SquareLength();
        
        if(data->cameraDistance <= refDistance)
        {
            cellList.push_back(data);
        }
    }
}

};

#endif
