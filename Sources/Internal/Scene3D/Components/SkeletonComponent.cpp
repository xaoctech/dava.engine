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



#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
    
REGISTER_CLASS(SkeletonComponent)


SkeletonComponent::SkeletonComponent()
{
    rootJoints.push_back(JointConfig(FastName("root0")));
    rootJoints.push_back(JointConfig(FastName("root1")));
    rootJoints[0].children.push_back(JointConfig(FastName("root0.bone0")));
    rootJoints[0].children[0].children.push_back(JointConfig(FastName("root0.bone0.bone0")));
    rootJoints[0].children[0].children.push_back(JointConfig(FastName("root0.bone0.bone1")));    

    rootJoints[1].children.push_back(JointConfig(FastName("root1.bone0")));
    rootJoints[1].children.push_back(JointConfig(FastName("root1.bone1")));
    rootJoints[1].children[0].children.push_back(JointConfig(FastName("root1.bone0.bone0")));
    rootJoints[1].children[0].children.push_back(JointConfig(FastName("root1.bone0.bone1")));
    rootJoints[1].children[1].children.push_back(JointConfig(FastName("root1.bone1.bone0")));
    rootJoints[1].children[1].children.push_back(JointConfig(FastName("root1.bone1.bone1")));
    rootJoints[1].children[1].children.push_back(JointConfig(FastName("root1.bone1.bone2")));
    rootJoints[1].children[1].children[2].children.push_back(JointConfig(FastName("root1.bone1.bone2.bone0")));
    rootJoints[1].children[1].children[2].children.push_back(JointConfig(FastName("root1.bone1.bone2.bone1")));
}

SkeletonComponent::~SkeletonComponent()
{

}

SkeletonComponent::JointConfig::JointConfig() : position(0.0f,0.0f,0.0f), orientation(0.0f,0.0f,0.0f,1.0f), scale(1.0f)
{    
}
SkeletonComponent::JointConfig::JointConfig(const FastName& _name, const Vector3& _position, const Quaternion& _orientation, float32 _scale, const AABBox3& _bbox): 
                                            name(_name), position(_position), orientation(_orientation), scale(_scale), bbox(_bbox)
{
}

SkeletonComponent::JointConfig::JointConfig(const FastName& _name) : name(_name), position(0.0f,0.0f,0.0f), orientation(0.0f,0.0f,0.0f,1.0f), scale(1.0f)
{
}


uint16 SkeletonComponent::GetConfigJointsCount()
{
    uint16 res = 0;
    for (int32 i=0, sz = rootJoints.size(); i<sz; ++i)
        res+=GetConfigJointsCountRecursively(rootJoints[i]);
    return res;
}
uint16 SkeletonComponent::GetConfigJointsCountRecursively(const JointConfig &joint)
{
    uint16 res = 1; //this joint;
    for (int32 i=0, sz = joint.children.size(); i<sz; ++i)
        res+=GetConfigJointsCountRecursively(joint.children[i]);
    return res;
}

void SkeletonComponent::RebuildFromConfig()
{
    GlobalEventSystem::Instance()->Event(GetEntity(), EventSystem::SKELETON_CONFIG_CHANGED);	
}

Component * SkeletonComponent::Clone(Entity * toEntity)
{
    SkeletonComponent * newComponent = new SkeletonComponent();    

    return newComponent;
}
void SkeletonComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);
}
void SkeletonComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

}