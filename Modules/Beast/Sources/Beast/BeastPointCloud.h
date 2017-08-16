#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_POINT_CLOUD__
#define __BEAST_POINT_CLOUD__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"

class BeastPointCloud : public BeastResource<BeastPointCloud>
{
public:
    virtual ~BeastPointCloud();

    void AddBakeEntity(DAVA::Entity* entityNode);
    void FinishCloud();

    DAVA_BEAST::ILBPointCloudHandle GetPointCloudHandle();

    DAVA_BEAST::ILBTargetHandle GetCloudTarget() const;
    DAVA_BEAST::ILBTargetEntityHandle GetEntityTarget() const;
    void SetTargets(DAVA_BEAST::ILBTargetHandle target, DAVA_BEAST::ILBTargetEntityHandle targetEntity);

    DAVA::int32 GetEntityCount();
    DAVA::Entity* GetEntity(DAVA::int32 index);

private:
    BeastPointCloud(const DAVA::String& name, BeastManager* manager);
    DAVA_BEAST::ILBTargetHandle cloudTarget;
    DAVA_BEAST::ILBPointCloudHandle cloudHandle;
    DAVA_BEAST::ILBTargetEntityHandle entityTarget;

    DAVA::Vector<DAVA::Entity*> bakeEntities;

    friend class BeastResource<BeastPointCloud>;
};

#endif //__BEAST_POINT_CLOUD__

#endif //__DAVAENGINE_BEAST__
