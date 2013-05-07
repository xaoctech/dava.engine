#ifndef __DAVAENGINE_TEXTURE_COMPRESSION_H__
#define __DAVAENGINE_TEXTURE_COMPRESSION_H__

#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

namespace DAVA
{

class TextureCompression: public StaticSingleton<TextureCompression>
{
public:

    TextureCompression();
    ~TextureCompression();
    
    const Vector<PixelFormat> & GetAvailableFormats(eGraphicsPlatfoms forPlatform) const;
    
protected:

    Vector<PixelFormat> availableFormats[PLATFORM_COUNT];
};
    

    
};


#endif // __DAVAENGINE_TEXTURE_COMPRESSION_H__