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



#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"


namespace DAVA
{

SkeletonSystem::SkeletonSystem(Scene * scene):SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED); 
}

SkeletonSystem::~SkeletonSystem()
{    
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void SkeletonSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity); 

    SkeletonComponent *component = GetSkeletonComponent(entity);
    DVASSERT(component);    
    RebuildSkeleton(component);    
}

void SkeletonSystem::RemoveEntity(Entity * entity)
{
    uint32 size = entities.size();
    for(uint32 i = 0; i < size; ++i)
    {
        if(entities[i] == entity)
        {
            entities[i] = entities[size-1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void SkeletonSystem::ImmediateEvent(Entity * entity, uint32 event)
{
    SkeletonComponent *component = GetSkeletonComponent(entity);
    DVASSERT(component);
    if (event == EventSystem::SKELETON_CONFIG_CHANGED)
    {
        RebuildSkeleton(component);        
    }    
}

void SkeletonSystem::Process(float32 timeElapsed)
{
    for (int32 i=0, sz=entities.size(); i<sz; ++i)
    {
        SkeletonComponent *component = GetSkeletonComponent(entities[i]);
        if (component&&(component->startJoint!=SkeletonComponent::INVALID_JOINT_INDEX))
        {
            UpdatePose(component);
            
            RenderObject *ro = GetRenderObject(entities[i]);
            if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
            {
                SkinnedMesh *skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
                DVASSERT(skinnedMeshObject);
                UpdateSkinnedMesh(component, skinnedMeshObject);
            }
        }        
    }    
}

void SkeletonSystem::UpdatePose(SkeletonComponent *component)
{    
    uint16 count = component->GetJointsCount();
    for (uint16 currJoint = component->startJoint; currJoint<count; ++currJoint)
    {
        uint16 parentJoint = component->jointInfo[currJoint]&SkeletonComponent::INFO_PARENT_MASK;
        if ((component->jointInfo[currJoint]&SkeletonComponent::FLAG_MARKED_FOR_UPDATED)||(component->jointInfo[parentJoint]&SkeletonComponent::FLAG_UPDATED_THIS_FRAME))
        {
            //calculate local pose
            if (parentJoint == SkeletonComponent::INVALID_JOINT_INDEX) //root
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint]; //just copy
            }
            else
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint].MultiplyByParent(component->objectSpaceTransforms[parentJoint]);
            }            
            SkeletonComponent::JointTransform finalTransform = component->inverseBindTransforms[currJoint].MultiplyByParent(component->objectSpaceTransforms[currJoint]);
            component->resultPositions[currJoint].Set(finalTransform.position.x, finalTransform.position.y, finalTransform.position.z, finalTransform.scale);
            component->resultQuaternions[currJoint].Set(finalTransform.orientation.x, finalTransform.orientation.y, finalTransform.orientation.z, finalTransform.orientation.w);
            //update bbox
            const Vector3& min = component->jointSpaceBoxes[currJoint].min;
            const Vector3& max = component->jointSpaceBoxes[currJoint].max;
            AABBox3& box = component->objectSpaceBoxes[currJoint];
            box.Empty();
            /*rework analogically to box.applytransform later*/
            box.AddPoint(finalTransform.TransformVector(min));
            box.AddPoint(finalTransform.TransformVector(max));
            box.AddPoint(finalTransform.TransformVector(Vector3(min.x, min.y, max.z)));
            box.AddPoint(finalTransform.TransformVector(Vector3(min.x, max.y, min.z)));
            box.AddPoint(finalTransform.TransformVector(Vector3(min.x, max.y, max.z)));
            box.AddPoint(finalTransform.TransformVector(Vector3(max.x, min.y, min.z)));
            box.AddPoint(finalTransform.TransformVector(Vector3(max.x, min.y, max.z)));
            box.AddPoint(finalTransform.TransformVector(Vector3(max.x, max.y, min.z)));
            
            //  add [was updated]  remove [marked for update]            
            component->jointInfo[currJoint] &=~ SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
            component->jointInfo[currJoint] |= SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
        else
        {
            /*  remove was updated  - note that as bones come in descending order we do not care that was updated flag would be cared to next frame*/
            component->jointInfo[currJoint] &=~ SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
    }
    component->startJoint = SkeletonComponent::INVALID_JOINT_INDEX;    
}

void SkeletonSystem::UpdateSkinnedMesh(SkeletonComponent *component, SkinnedMesh *skinnedMeshObject)
{
    //recalculate object box
    uint16 count = component->GetJointsCount();
    AABBox3 resBox;
    for (uint16 currJoint=0; currJoint<count; ++currJoint)
        resBox.AddAABBox(component->objectSpaceBoxes[currJoint]);

    //set data to SkinnedMesh    
    skinnedMeshObject->SetJointsPtr(&component->resultPositions[0], &component->resultQuaternions[0], count);
    skinnedMeshObject->SetObjectSpaceBoundingBox(resBox);
    GetScene()->renderSystem->MarkForUpdate(skinnedMeshObject);    
}


void SkeletonSystem::RebuildSkeleton(SkeletonComponent *component)
{
    /*convert joint configs to joints*/
    component->jointInfo.clear();
    component->localSpaceTransforms.clear();
    component->jointSpaceBoxes;
    
    for (int32 i=0, sz = component->rootJoints.size(); i<sz; ++i)
        AddJointConfig(component, component->rootJoints[i], SkeletonComponent::INVALID_JOINT_INDEX);

    component->startJoint = 0;
    UpdatePose(component);
}

void SkeletonSystem::AddJointConfig(SkeletonComponent *component, SkeletonComponent::JointConfig &config, uint16 parent)
{
    /*int32 jointid = component->localSpaceTransforms.size();
    jointid*/
}


}