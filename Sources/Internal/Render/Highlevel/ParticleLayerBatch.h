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

	virtual void Draw(Camera * camera);
	void SetTotalCount(int32 totalCount);

protected:
	int32 totalCount;
};

}

#endif //__DAVAENGINE_PARTICLELAYER_BATCH_H__