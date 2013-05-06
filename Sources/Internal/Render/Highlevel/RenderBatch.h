/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_SCENE3D_RENDER_BATCH_H__
#define	__DAVAENGINE_SCENE3D_RENDER_BATCH_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{

/*
class RenderCallInstance
{
public:
    VBO *
    IBO *
    NMaterialInstance *
    NMaterial *
    uint32 start;
    uint32 count;
    uint32 primitiveType;
};
*/
class RenderLayer;
class Camera;
class RenderObject;
class RenderBatch;
class NMaterial;
class NMaterialInstance;

    
/*
    Not finished. We'll return to it when we start batching functionality.
 */
class IRenderBatchDataSource
{
public:
    virtual void InitBatch(RenderBatch * renderBatch) = 0;
    virtual void FillData(RenderBatch * renderBatch) = 0;
    virtual void ReleaseBatch(RenderBatch * renderBatch) = 0;
};

class RenderBatch : public BaseObject
{
public:
    RenderBatch();
    virtual ~RenderBatch();
    
    const FastName & GetOwnerLayerName();
    void SetOwnerLayerName(const FastName & fastname);
    
    void SetPolygonGroup(PolygonGroup * _polygonGroup);
    inline PolygonGroup * GetPolygonGroup();
    
    void SetRenderDataObject(RenderDataObject * _renderDataObject);
    inline RenderDataObject * GetRenderDataObject();
    
    void SetMaterial(NMaterial * _material);
    void SetMaterialInstance(NMaterialInstance * _materialInstance);
    
    inline NMaterial * GetMaterial();
    inline NMaterialInstance * GetMaterialInstance();
    
	void SetRenderObject(RenderObject * renderObject);
	inline RenderObject * GetRenderObject();
    
    inline void SetStartIndex(uint32 _startIndex);
    inline void SetIndexCount(uint32 _indexCount);
    
    inline void SetRemoveIndex(RenderLayer * _ownerLayer, uint32 _removeIndex);
    inline uint32 GetRemoveIndex();
    inline RenderLayer * GetOwnerLayer();

    virtual void Draw(const FastName & ownerRenderPass, Camera * camera);
    
    const AABBox3 & GetBoundingBox() const;

	virtual void GetDataNodes(Set<DataNode*> & dataNodes);
	virtual RenderBatch * Clone(RenderBatch * destination = 0);
	virtual void Save(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Load(KeyedArchive *archive, SceneFileV2 *sceneFile);
    
    /*
        \brief This is additional sorting key. It should be from 0 to 15.
     */
    void SetSortingKey(uint32 key);
    inline uint32 GetSortingKey();

	void SetVisibilityCriteria(uint32 criteria);

	virtual void UpdateAABBoxFromSource();

protected:
    PolygonGroup * dataSource;
    RenderDataObject * renderDataObject;   // Probably should be replaced to VBO / IBO, but not sure
    NMaterial * material;                    // Should be replaced to NMaterial
    NMaterialInstance * materialInstance; // Should be replaced by NMaterialInstance
    
    
	RenderObject * renderObject;
    FastName ownerLayerName;
    
    uint32 startIndex;
    uint32 indexCount;
    
//    ePrimitiveType type; //TODO: waiting for enums at introspection
    uint32 type;
    uint32 sortingKey;
    uint32 visiblityCriteria;
    RenderLayer * ownerLayer;
    uint32 removeIndex;

	AABBox3 aabbox;

	void InsertDataNode(DataNode *node, Set<DataNode*> & dataNodes);
    
public:
    
    INTROSPECTION_EXTEND(RenderBatch, BaseObject,
        MEMBER(dataSource, "Data Source", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(renderDataObject, "Render Data Object", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR | INTROSPECTION_EDITOR_READONLY)
//        MEMBER(renderObject, "Render Object", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR | INTROSPECTION_EDITOR_READONLY)

        MEMBER(startIndex, "Start Index", INTROSPECTION_SERIALIZABLE)
        MEMBER(indexCount, "Index Count", INTROSPECTION_SERIALIZABLE)
        MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(aabbox, "AABBox",  INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR )
        MEMBER(material, "Material", INTROSPECTION_EDITOR)
                         
        PROPERTY("ownerLayerName", "Owner Layer", GetOwnerLayerName, SetOwnerLayerName, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR | INTROSPECTION_EDITOR_READONLY)
        PROPERTY("sortingKey", "Key for the sorting inside render layer", GetSortingKey, SetSortingKey, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)

        MEMBER(materialInstance, "Material Instance", INTROSPECTION_EDITOR)
    );
};

inline PolygonGroup * RenderBatch::GetPolygonGroup()
{
    return dataSource;
}
    
inline RenderDataObject * RenderBatch::GetRenderDataObject()
{
    return renderDataObject;
}

inline NMaterial * RenderBatch::GetMaterial()
{
    return material;
}
    
inline NMaterialInstance * RenderBatch::GetMaterialInstance()
{
    return materialInstance;
}

inline RenderObject * RenderBatch::GetRenderObject()
{
	return renderObject;
}

inline void RenderBatch::SetStartIndex(uint32 _startIndex)
{
    startIndex = _startIndex;
}
    
inline void RenderBatch::SetIndexCount(uint32 _indexCount)
{
    indexCount = _indexCount;
}
    
    
    
inline uint32 RenderBatch::GetRemoveIndex()
{
    return removeIndex;
}
    
inline RenderLayer * RenderBatch::GetOwnerLayer()
{
    return ownerLayer;
}

inline void RenderBatch::SetRemoveIndex(RenderLayer * _ownerLayer, uint32 _removeIndex)
{
    ownerLayer = _ownerLayer;
    removeIndex = _removeIndex;
}
    
inline uint32 RenderBatch::GetSortingKey()
{
    return sortingKey;
}

    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDER_BATCH_H__ */

