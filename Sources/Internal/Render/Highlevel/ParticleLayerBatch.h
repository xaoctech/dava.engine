#ifndef __DAVAENGINE_PARTICLELAYER_BATCH_H__
#define __DAVAENGINE_PARTICLELAYER_BATCH_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderBatch.h"

namespace DAVA
{

class ParticleLayerBatch : public RenderBatch
{
public:
	ParticleLayerBatch();
	virtual ~ParticleLayerBatch();

	virtual void Draw(Camera * camera);
	void SetTotalCount(int32 totalCount);

	virtual RenderBatch * Clone();

protected:
	int32 totalCount;
    
public:
    INTROSPECTION_EXTEND(ParticleLayerBatch, RenderBatch,
        MEMBER(totalCount, "Total Count", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR | INTROSPECTION_EDITOR_READONLY)
    );
};

}

#endif //__DAVAENGINE_PARTICLELAYER_BATCH_H__