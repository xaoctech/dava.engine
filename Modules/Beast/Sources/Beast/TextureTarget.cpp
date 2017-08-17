#include "Beast/TextureTarget.h"
#include "Beast/BeastDebug.h"

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
