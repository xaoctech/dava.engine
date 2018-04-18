#include "Scene3D/SkeletonAnimation/SkeletonUtils.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
void SkeletonUtils::UpdateJointTransforms(SkeletonComponent* skeletonComponent)
{
    DVASSERT(skeletonComponent != nullptr);

    DVASSERT(!skeletonComponent->configUpdated);

    uint32 count = skeletonComponent->GetJointsCount();
    for (uint32 currJoint = skeletonComponent->startJoint; currJoint < count; ++currJoint)
    {
        uint32 parentJoint = skeletonComponent->jointInfo[currJoint] & SkeletonComponent::INFO_PARENT_MASK;
        if ((skeletonComponent->jointInfo[currJoint] & SkeletonComponent::FLAG_MARKED_FOR_UPDATED) || ((parentJoint != SkeletonComponent::INVALID_JOINT_INDEX) && (skeletonComponent->jointInfo[parentJoint] & SkeletonComponent::FLAG_UPDATED_THIS_FRAME)))
        {
            //calculate object space transforms
            if (parentJoint == SkeletonComponent::INVALID_JOINT_INDEX) //root
            {
                skeletonComponent->objectSpaceTransforms[currJoint] = skeletonComponent->localSpaceTransforms[currJoint]; //just copy
            }
            else
            {
                skeletonComponent->objectSpaceTransforms[currJoint] = skeletonComponent->objectSpaceTransforms[parentJoint].AppendTransform(skeletonComponent->localSpaceTransforms[currJoint]);
            }

            //calculate final transform including bindTransform
            skeletonComponent->finalTransforms[currJoint] = skeletonComponent->objectSpaceTransforms[currJoint].AppendTransform(skeletonComponent->inverseBindTransforms[currJoint]);

            if (!skeletonComponent->jointsArray[currJoint].bbox.IsEmpty())
            {
                skeletonComponent->objectSpaceBoxes[currJoint] = skeletonComponent->objectSpaceTransforms[currJoint].ApplyToAABBox(skeletonComponent->jointsArray[currJoint].bbox);
            }
            else
            {
                skeletonComponent->objectSpaceBoxes[currJoint].Empty();
            }

            //  add [was updated]  remove [marked for update]
            skeletonComponent->jointInfo[currJoint] &= ~SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
            skeletonComponent->jointInfo[currJoint] |= SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
        else
        {
            /*  remove was updated  - note that as bones come in descending order we do not care that was updated flag would be cared to next frame*/
            skeletonComponent->jointInfo[currJoint] &= ~SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
    }
    skeletonComponent->startJoint = SkeletonComponent::INVALID_JOINT_INDEX;
}
}