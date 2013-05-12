#ifndef __DAVAENGINE_SPRITE_RENDER_BATCH_H__
#define __DAVAENGINE_SPRITE_RENDER_BATCH_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderBatch.h"

namespace DAVA
{

class SpriteRenderBatch : public RenderBatch
{
public:
	SpriteRenderBatch();
	virtual ~SpriteRenderBatch();

	virtual void Draw(const FastName & ownerRenderPass, Camera * camera);

protected:
    

public:

	INTROSPECTION_EXTEND(SpriteRenderBatch, RenderBatch, 
		NULL
	);

};

}

#endif //__DAVAENGINE_SPRITE_RENDER_BATCH_H__