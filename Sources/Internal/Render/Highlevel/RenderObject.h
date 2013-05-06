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
#ifndef __DAVAENGINE_SCENE3D_RENDEROBJECT_H__
#define	__DAVAENGINE_SCENE3D_RENDEROBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Animation/AnimatedObject.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Scene.h"

namespace DAVA
{

/*
class RenderCallInstance
{
public:
    VBO *
    IBO *
    NMaterial *
    uint32 start;
    uint32 count;
    uint32 primitiveType;
};
*/
    
/*
    Types of possible render objects
 
    
    - Mesh(Static)
    - Mesh(Skinned)
 
 */

class RenderBatch;
class ShadowVolume;
class RenderObject : public AnimatedObject
{
public:
    enum eType
    {
        TYPE_RENDEROBJECT = 0,  // Base Render Object
        TYPE_MESH,              // Normal mesh
        TYPE_SKINNED_MESH,      // Animated mesh for skinned animations
        TYPE_LANDSCAPE,         // Landscape object
        TYPE_CUSTOM_DRAW,       // Custom drawn object
		TYPE_SPRITE,			// Sprite Node
		TYPE_PARTICLE_EMTITTER  // Particle Emitter
    };
    
	enum eFlags
	{
		VISIBLE = 1 << 0,
		VISIBLE_LOD = 1 << 1,
		VISIBLE_SWITCH = 1 << 2,
        TRANSFORM_UPDATED = 1 << 15,
	};

	static const uint32 VISIBILITY_CRITERIA = VISIBLE | VISIBLE_LOD | VISIBLE_SWITCH;

    RenderObject();
    virtual ~RenderObject();
    
    
    inline void SetRemoveIndex(uint32 removeIndex);
    inline uint32 GetRemoveIndex();
    
    void AddRenderBatch(RenderBatch * batch);
    void RemoveRenderBatch(RenderBatch * batch);
    void RecalcBoundingBox();
    
    uint32 GetRenderBatchCount();
    RenderBatch * GetRenderBatch(uint32 batchIndex);
    
    inline void SetFlags(uint32 _flags) { flags = _flags; }
    inline uint32 GetFlags() { return flags; }
    inline void AddFlag(uint32 _flag) { flags |= _flag; }
    inline void RemoveFlag(uint32 _flag) { flags &= ~_flag; }
    
    inline void SetAABBox(const AABBox3 & bbox);
    inline void SetWorldAABBox(const AABBox3 & bbox);
    inline void SetBSphere(const Sphere & sphere);
    
    inline const AABBox3 & GetBoundingBox() const;
    inline AABBox3 & GetWorldBoundingBox();
    
    inline void SetWorldTransformPtr(Matrix4 * _worldTransform);
    inline Matrix4 * GetWorldTransformPtr() const;
    
    inline eType GetType() { return (eType)type; }

	virtual RenderObject * Clone(RenderObject *newObject);
	virtual void Save(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Load(KeyedArchive *archive, SceneFileV2 *sceneFile);

    void SetOwnerDebugInfo(const String & str) { ownerDebugInfo = str; };

	void SetRenderSystem(RenderSystem * renderSystem);
	RenderSystem * GetRenderSystem();

	virtual void BakeTransform(const Matrix4 & transform) {}
	virtual ShadowVolume * CreateShadow() {return 0;}
    
protected:
//    eType type; //TODO: waiting for enums at introspection
	RenderSystem * renderSystem;
    uint32 type;

    uint32 flags;
    uint32 debugFlags;
    uint32 removeIndex;
    AABBox3 bbox;
    AABBox3 worldBBox;
    Matrix4 * worldTransform;                    // temporary - this should me moved directly to matrix uniforms
    String ownerDebugInfo;
//    Sphere bsphere;
    
    Vector<RenderBatch*> renderBatchArray;

public:
	INTROSPECTION_EXTEND(RenderObject, AnimatedObject,
        MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(flags, "Flags", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(debugFlags, "Debug Flags", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(removeIndex, "Remove index", INTROSPECTION_SERIALIZABLE)
        MEMBER(bbox, "Box", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(worldBBox, "World Box", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)

        MEMBER(worldTransform, "World Transform", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                 
        COLLECTION(renderBatchArray, "Render Batch Array", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

inline uint32 RenderObject::GetRemoveIndex()
{
    return removeIndex;
}
    
inline void RenderObject::SetRemoveIndex(uint32 _removeIndex)
{
    removeIndex = _removeIndex;
}
    
inline void RenderObject::SetAABBox(const AABBox3 & _bbox)
{
    bbox = _bbox;
}

inline void RenderObject::SetWorldAABBox(const AABBox3 & _bbox)
{
    worldBBox = _bbox;
}

inline const AABBox3 & RenderObject::GetBoundingBox() const
{
    return bbox;
}

inline AABBox3 & RenderObject::GetWorldBoundingBox()
{
    return worldBBox;
}
    
inline void RenderObject::SetWorldTransformPtr(Matrix4 * _worldTransform)
{
    worldTransform = _worldTransform;
    flags |= TRANSFORM_UPDATED;
}
    
inline Matrix4 * RenderObject::GetWorldTransformPtr() const
{
    return worldTransform;
}

    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDEROBJECT_H__ */

