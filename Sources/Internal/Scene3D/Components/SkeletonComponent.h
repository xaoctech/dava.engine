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

    
    struct JointTransform
    {
        Quaternion orientation;
        Vector3 position;
        float32 scale;

        inline JointTransform MultiplyByParent(const JointTransform& parent) const;
        inline Vector3 TransformVector(const Vector3 &inVec) const;
    };                    

    virtual Component * Clone(Entity * toEntity);
    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    inline void SetJointPosition(uint16 jointId, const Vector3 &position);
    inline void SetJointOrientation(uint16 jointId, const Quaternion &orientation);
    inline void SetJointScale(uint16 jointId, float32 scale);
    
    inline uint16 GetJointId(const FastName& name) const;

    inline uint16 GetJointsCount() const;

    SkeletonComponent();
    ~SkeletonComponent();

private:
    const static uint32 INFO_PARENT_MASK=0xff;
    const static uint32 FLAG_UPDATED_THIS_FRAME=0x1+INFO_PARENT_MASK;
    const static uint32 FLAG_MARKED_FOR_UPDATED=0x2+INFO_PARENT_MASK;

    Vector<uint32> jointInfo; //flags and parent
    
    //transforms info
    Vector<JointTransform> localSpaceTransforms;
    Vector<JointTransform> objectSpaceTransforms;
    
    Vector<JointTransform> inverseBindTransforms;

    //bounding boxes for bone
    Vector<AABBox3> jointSpaceBoxes;
    Vector<AABBox3> objectSpaceBoxes;

    Vector<Vector4> resultPositions; //stores final results
    Vector<Vector4> resultQuaternions;

    Map<FastName, uint16> jointMap;
            
    uint16 startJoint; //first joint in the list that was updated this frame - cache this value to optimize processing

};



inline void SkeletonComponent::SetJointPosition(uint16 jointId, const Vector3 &position)
{
    DVASSERT(jointId<GetJointsCount());
    localSpaceTransforms[jointId].position = position;
    startJoint = Min(startJoint, jointId);
}
inline void SkeletonComponent::SetJointOrientation(uint16 jointId, const Quaternion &orientation)
{
    DVASSERT(jointId<GetJointsCount());
    localSpaceTransforms[jointId].orientation = orientation;
    startJoint = Min(startJoint, jointId);
}
inline void SkeletonComponent::SetJointScale(uint16 jointId, float32 scale)
{
    DVASSERT(jointId<GetJointsCount());
    localSpaceTransforms[jointId].scale = scale;
    startJoint = Min(startJoint, jointId);
}

inline uint16 SkeletonComponent::GetJointId(const FastName& name) const
{
    Map<FastName, uint16>::const_iterator it = jointMap.find(name);
    if (jointMap.end()!=it)
        return it->second;
    else
        return INVALID_BONE_INDEX;
}

inline uint16 SkeletonComponent::GetJointsCount() const
{
    return localSpaceTransforms.size(); //use local transforms size as it is the only one modifiable from outside
}

inline Vector3 SkeletonComponent::JointTransform::TransformVector(const Vector3 &inVec) const
{    
    return position + orientation.ApplyToVectorFast(inVec)*scale; 
}

} //ns

#endif