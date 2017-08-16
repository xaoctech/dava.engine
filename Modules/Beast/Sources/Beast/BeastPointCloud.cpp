#ifdef __DAVAENGINE_BEAST__

#include "BeastDebug.h"
#include "BeastManager.h"
#include "BeastPointCloud.h"

BeastPointCloud::BeastPointCloud(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
    ,
    cloudHandle(0)
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreatePointCloud(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &cloudHandle));
}

BeastPointCloud::~BeastPointCloud()
{
    bakeEntities.clear();
}

void BeastPointCloud::AddBakeEntity(DAVA::Entity* entityNode)
{
    DAVA::Vector3 position = GetRenderObject(entityNode)->GetWorldBoundingBox().GetCenter();
    DAVA::Vector3 normal;

    DAVA_BEAST::ILBVec3 point(position.x, position.y, position.z);
    DAVA_BEAST::ILBVec3 norm(normal.x, normal.y, normal.z);

    DAVA_BEAST::ILBAddPointCloudData(cloudHandle, &point, &norm, 1);
    bakeEntities.push_back(entityNode);
}

void BeastPointCloud::FinishCloud()
{
    DAVA_BEAST::ILBEndPointCloud(cloudHandle);
}

DAVA_BEAST::ILBPointCloudHandle BeastPointCloud::GetPointCloudHandle()
{
    return cloudHandle;
}

DAVA_BEAST::ILBTargetHandle BeastPointCloud::GetCloudTarget() const
{
    return cloudTarget;
}

void BeastPointCloud::SetTargets(DAVA_BEAST::ILBTargetHandle _target, DAVA_BEAST::ILBTargetEntityHandle _targetEntity)
{
    cloudTarget = _target;
    entityTarget = _targetEntity;
}

DAVA_BEAST::ILBTargetEntityHandle BeastPointCloud::GetEntityTarget() const
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

#endif //__DAVAENGINE_BEAST__
