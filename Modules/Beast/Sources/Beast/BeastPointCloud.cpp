#include "Beast/BeastDebug.h"
#include "Beast/BeastManager.h"
#include "Beast/BeastPointCloud.h"

#include <Scene3D/Components/ComponentHelpers.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Entity.h>

namespace Beast
{
BeastPointCloud::BeastPointCloud(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
    , cloudHandle(0)
{
    BEAST_VERIFY(ILBCreatePointCloud(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &cloudHandle));
}

BeastPointCloud::~BeastPointCloud()
{
    bakeEntities.clear();
}

void BeastPointCloud::AddBakeEntity(DAVA::Entity* entityNode)
{
    DAVA::Vector3 position = DAVA::GetRenderObject(entityNode)->GetWorldBoundingBox().GetCenter();
    DAVA::Vector3 normal;

    ILBVec3 point(position.x, position.y, position.z);
    ILBVec3 norm(normal.x, normal.y, normal.z);

    ILBAddPointCloudData(cloudHandle, &point, &norm, 1);
    bakeEntities.push_back(entityNode);
}

void BeastPointCloud::FinishCloud()
{
    ILBEndPointCloud(cloudHandle);
}

ILBPointCloudHandle BeastPointCloud::GetPointCloudHandle()
{
    return cloudHandle;
}

ILBTargetHandle BeastPointCloud::GetCloudTarget() const
{
    return cloudTarget;
}

void BeastPointCloud::SetTargets(ILBTargetHandle _target, ILBTargetEntityHandle _targetEntity)
{
    cloudTarget = _target;
    entityTarget = _targetEntity;
}

ILBTargetEntityHandle BeastPointCloud::GetEntityTarget() const
{
    return entityTarget;
}

DAVA::int32 BeastPointCloud::GetEntityCount()
{
    return (DAVA::int32)bakeEntities.size();
}

DAVA::Entity* BeastPointCloud::GetEntity(DAVA::int32 index)
{
    return bakeEntities[index];
}
}