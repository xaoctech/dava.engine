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


namespace DAVA
{

SkeletonSystem::SkeletonSystem(Scene * scene):SceneSystem(scene)
{

}

void SkeletonSystem::Process(float32 timeElapsed)
{
    /*for each active component*/
    {
       // SkeletonComponent *component;
        /*for each bone*/        
        /*if bone [marked for update]  or parent [was updated]*/
        /*  recalculate object space transform*/
        /*  add [was updated]*/
        /*  remove [marked for update]*/
        /*else*/
        /*  remove was updated  - note that as bones come in descending order we do not care that was updated flag would be cared to next frame*/

    }
}

void SkeletonSystem::RebuildPose(Entity *entity)
{
    SkeletonComponent *component = GetSkeletonComponent(entity);
    uint16 count = component->GetJointsCount();
    for (uint16 currJoint = component->startJoint; currJoint<count; ++currJoint)
    {
        uint16 parentJoint = component->jointInfo[currJoint]&SkeletonComponent::INFO_PARENT_MASK;
        if ((component->jointInfo[currJoint]&SkeletonComponent::FLAG_MARKED_FOR_UPDATED)||(component->jointInfo[parentJoint]&SkeletonComponent::FLAG_UPDATED_THIS_FRAME))
        {
            //calculate local pose
            if (parentJoint == SkeletonComponent::INVALID_BONE_INDEX) //root
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint]; //just copy
            }
            else
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint].MultiplyByParent(component->objectSpaceTransforms[parentJoint]);
            }            
            //update bbox
            
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

    //recalculate object box
    AABBox3 resBox;
    for (uint16 currJoint=0; currJoint<count; ++currJoint)
        resBox.AddAABBox(component->objectSpaceBoxes[currJoint]);

    //set data to SkinnedMesh
    RenderObject *ro = GetRenderObject(entity);
    if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
    {
        SkinnedMesh *skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
        DVASSERT(skinnedMeshObject);

        skinnedMeshObject->SetJointsPtr(&component->resultPositions[0], &component->resultQuaternions[0], count);
        skinnedMeshObject->SetObjectSpaceBoundingBox(resBox);
        GetScene()->renderSystem->MarkForUpdate(skinnedMeshObject);
    }
}

}