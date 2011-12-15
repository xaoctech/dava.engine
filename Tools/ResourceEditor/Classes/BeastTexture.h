#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_TEXTURE__
#define __BEAST_TEXTURE__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"

class BeastTexture : public BeastResource<BeastTexture>
{
public:
	void InitWithFile(const DAVA::String & filePath);

private:
	BeastTexture(ILBManagerHandle manager);
	ILBTextureHandle texture;
};

#endif //__BEAST_TEXTURE__

#endif //__DAVAENGINE_BEAST__
