#ifdef __DAVAENGINE_BEAST__

#include "TextureTarget.h"
#include "BeastDebug.h"

TextureTarget::TextureTarget(ILBJobHandle job, DAVA::int32 size)
{
    BEAST_VERIFY(ILBCreateTextureTarget(job, GENERATE_BEAST_NAME(TextureTarget), size, size, &handle));
}

TextureTarget::~TextureTarget()
{
}

ILBTargetHandle TextureTarget::GetHandle()
{
    return handle;
}

#endif //__DAVAENGINE_BEAST__
