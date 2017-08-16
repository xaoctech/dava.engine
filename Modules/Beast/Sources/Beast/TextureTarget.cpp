#ifdef __DAVAENGINE_BEAST__

#include "TextureTarget.h"
#include "BeastDebug.h"

TextureTarget::TextureTarget(DAVA_BEAST::ILBJobHandle job, DAVA::int32 size)
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreateTextureTarget(job, GENERATE_BEAST_NAME(TextureTarget), size, size, &handle));
}

TextureTarget::~TextureTarget()
{
}

DAVA_BEAST::ILBTargetHandle TextureTarget::GetHandle()
{
    return handle;
}

#endif //__DAVAENGINE_BEAST__
