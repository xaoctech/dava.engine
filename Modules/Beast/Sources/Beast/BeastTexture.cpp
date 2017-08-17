#ifdef __DAVAENGINE_BEAST__

#include "BeastTexture.h"
#include "BeastDebug.h"
#include "BeastManager.h"

BeastTexture::BeastTexture(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
{
}

void BeastTexture::InitWithTexture(DAVA::Texture* davaTexture)
{
    DVASSERT(nullptr != davaTexture);
    InitWithFile(davaTexture->GetDescriptor()->GetSourceTexturePathname());
}

void BeastTexture::InitWithFile(const DAVA::FilePath& filePath)
{
    if (nullptr == texture)
    {
        DVASSERT(!filePath.IsEmpty());
        BEAST_VERIFY(ILBReferenceTexture(manager->GetILBManager(), STRING_TO_BEAST_STRING(resourceName),
                                         STRING_TO_BEAST_STRING(filePath.GetAbsolutePathname()), &texture));
    }
}

ILBTextureHandle BeastTexture::GetILBTexture()
{
    return texture;
}

#endif //__DAVAENGINE_BEAST__
