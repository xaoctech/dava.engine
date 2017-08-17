#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_TEXTURE_TARGET__
#define __BEAST_TEXTURE_TARGET__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastNames.h"

class TextureTarget
{
public:
    DECLARE_BEAST_NAME(BeastManager);

    TextureTarget(ILBJobHandle job, DAVA::int32 size);
    ~TextureTarget();

    ILBTargetHandle GetHandle();

private:
    ILBTargetHandle handle;
};

#endif //__BEAST_TEXTURE_TARGET__

#endif //__DAVAENGINE_BEAST__
