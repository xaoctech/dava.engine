#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_TEXTURE__
#define __BEAST_TEXTURE__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"

class BeastTexture : public BeastResource<BeastTexture>
{
public:
    void InitWithTexture(DAVA::Texture* davaTexture);
    void InitWithFile(const DAVA::FilePath& filePath);
    DAVA_BEAST::ILBTextureHandle GetILBTexture();

private:
    BeastTexture(const DAVA::String& name, BeastManager* manager);
    DAVA_BEAST::ILBTextureHandle texture = nullptr;

    friend class BeastResource<BeastTexture>;
};

#endif //__BEAST_TEXTURE__

#endif //__DAVAENGINE_BEAST__
