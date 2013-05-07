#include "TextureCompression.h"


namespace DAVA
{
    
TextureCompression::TextureCompression()
{
    //ios
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_RGBA8888);
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_RGBA5551);
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_RGBA4444);
//    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_RGB888); //?
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_RGB565);
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_A8);
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_PVR4);
    availableFormats[PLATFORM_PowerVR_IOS].push_back(FORMAT_PVR2);
    
    //android
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_RGBA8888);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_RGBA5551);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_RGBA4444);
//    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_RGB888); //?
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_RGB565);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_A8);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_PVR4);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_PVR2);
    availableFormats[PLATFORM_PowerVR_ANDROID].push_back(FORMAT_ETC1);

    
    
    
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_RGBA8888);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT1);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT1A);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT1NM);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT3);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT5);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_DXT5NM);
    availableFormats[PLATFORM_TEGRA].push_back(FORMAT_ETC1);

    
    availableFormats[PLATFORM_MALI].push_back(FORMAT_RGBA8888);
    availableFormats[PLATFORM_MALI].push_back(FORMAT_ETC1);

    
    availableFormats[PLATFORM_ADRENO].push_back(FORMAT_RGBA8888);
    availableFormats[PLATFORM_ADRENO].push_back(FORMAT_ATC_RGB);
    availableFormats[PLATFORM_ADRENO].push_back(FORMAT_ATC_RGBA_EXPLICIT_ALPHA);
    availableFormats[PLATFORM_ADRENO].push_back(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA);
    availableFormats[PLATFORM_ADRENO].push_back(FORMAT_ETC1);
}

TextureCompression::~TextureCompression()
{
    for(int32 i = 0; i < PLATFORM_COUNT; ++i)
    {
        availableFormats[i].clear();
    }
}
    
const Vector<PixelFormat> & TextureCompression::GetAvailableFormats(eGraphicsPlatfoms forPlatform) const
{
    DVASSERT(PLATFORM_UNKNOWN < forPlatform && forPlatform < PLATFORM_COUNT);

    return availableFormats[forPlatform];
}

    
    
};
