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


#ifndef __DAVAENGINE_SPEEDTREE_OBJECT_H__
#define __DAVAENGINE_SPEEDTREE_OBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Render/Highlevel/Mesh.h"

namespace DAVA 
{

class SpeedTreeUpdateSystem;
class SpeedTreeObject: public Mesh
{
public:

    SpeedTreeObject();
    virtual ~SpeedTreeObject();

    virtual void RecalcBoundingBox();
    virtual RenderObject * Clone(RenderObject *newObject);

	virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);

	virtual void PrepareToRender(Camera *camera);

    static bool IsTreeLeafBatch(RenderBatch * batch);

    inline const Color & GetLeafColorDark() const;
    inline const Color & GetLeafColorLight() const;
    inline const float32 & GetLeafColorMultiplier() const;
    void SetLeafColorDark(const Color & color);
    void SetLeafColorLight(const Color & color);
    void SetLeafColorMultiplier(const float32 & mul);

protected:
    static const FastName FLAG_WIND_ANIMATION;

    void SetTreeAnimationParams(const Vector2 & trunkOscillationParams, const Vector2 & leafOscillationParams);
    void SetAnimationFlag(bool flagOn);
    
    AABBox3 CalcBBoxForSpeedTreeGeometry(RenderBatch * rb);
    void CollectMaterials();

    void SetLeafMaterialPropertyValue(const FastName & keyName, Shader::eUniformType type, const void * data);

    bool animationFlagOn;
    
    Vector<NMaterial *> allMaterials;
    Vector<NMaterial *> leafMaterials;
    
    Color leafColorDark;
    Color leafColorLight;
    float32 leafColorMultiplier;

public:

	INTROSPECTION_EXTEND(SpeedTreeObject, Mesh, 
		PROPERTY("leafColorDark", "leafColorDark", GetLeafColorDark, SetLeafColorDark, I_VIEW | I_SAVE | I_EDIT)
		PROPERTY("leafColorLight", "leafColorLight", GetLeafColorLight, SetLeafColorLight, I_VIEW | I_SAVE | I_EDIT)
		PROPERTY("leafColorMultiplier", "leafColorMultiplier", GetLeafColorMultiplier, SetLeafColorMultiplier, I_VIEW | I_SAVE | I_EDIT)
	);

friend class SpeedTreeUpdateSystem;
};

inline const Color & SpeedTreeObject::GetLeafColorDark() const
{
    return leafColorDark;
}

inline const Color & SpeedTreeObject::GetLeafColorLight() const
{
    return leafColorLight;
}
    
inline const float32 & SpeedTreeObject::GetLeafColorMultiplier() const
{
    return leafColorMultiplier;
}

};

#endif // __DAVAENGINE_SPEEDTREE_OBJECT_H__
