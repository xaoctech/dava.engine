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

    ILBPointCloudHandle GetPointCloudHandle();

    ILBTargetHandle GetCloudTarget() const;
    ILBTargetEntityHandle GetEntityTarget() const;
    void SetTargets(ILBTargetHandle target, ILBTargetEntityHandle targetEntity);

    DAVA::int32 GetEntityCount();
    DAVA::Entity* GetEntity(DAVA::int32 index);

private:
    BeastPointCloud(const DAVA::String& name, BeastManager* manager);
    ILBTargetHandle cloudTarget;
    ILBPointCloudHandle cloudHandle;
    ILBTargetEntityHandle entityTarget;

    DAVA::Vector<DAVA::Entity*> bakeEntities;

    friend class BeastResource<BeastPointCloud>;
};

#endif //__BEAST_POINT_CLOUD__

#endif //__DAVAENGINE_BEAST__
