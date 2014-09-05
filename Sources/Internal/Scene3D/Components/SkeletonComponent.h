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



#ifndef __DAVAENGINE_SKELETON_COMPONENT_H__
#define __DAVAENGINE_SKELETON_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{

class Entity;
class SkeletonComponent : public Component
{
    friend class SkeletonSystem;
public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_COMPONENT);

    const static uint16 INVALID_BONE_INDEX = -1;

    struct Joint
    {
        struct JointTransform
        {
            Quaternion q;
            Vector3 position;
            float32 scale;
        };
        enum {BF_UPDATED_THIS_FRAME = 1<<0 ,
              BS_MARKED_FOR_UPDATE  = 1<<1
        };
        uint16 flags;
        uint16 parent;

        JointTransform localTransform, worldTransform, inverseBindTransform;
    };
    

    virtual Component * Clone(Entity * toEntity);
    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    void SetJointPosition(int32 jointId, const Vector3 &position);
    void SetJointOrientation(int32 jointId, const Quaternion &orientation);
    void SetJointScale(int32 jointId, float32 scale);
    
    uint16 GetJointId(const FastName& name);

    SkeletonComponent();
    ~SkeletonComponent();

private:

    Vector<Joint> joints; //stores joint information

    Vector<Vector4> resultPositions; //stores final results
    Vector<Vector4> resultQuaternions;

    Map<FastName, uint16> jointMap;
        
    uint16 startJoint; //first joint in the list that was updated this frame

};

} //ns

#endif