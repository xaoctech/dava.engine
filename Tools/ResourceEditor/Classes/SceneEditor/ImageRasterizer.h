#ifndef __IMAGE_RASTERIZER_H__
#define __IMAGE_RASTERIZER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ImageRasterizer
{
    
public:
    
    static void DrawRelative(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k);
    static void DrawRelativeRGBA(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k);
    
    static void DrawAverage(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k);
    static void DrawAverageRGBA(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k);

    static void DrawAbsolute(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 time, float32 dstHeight);
    static void DrawAbsoluteRGBA(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 time, float32 dstHeight);

    
    static bool Clipping(int32 & srcOffset, int32 & dstOffset, int32 & dstX, int32 & dstY, 
                         int32 dstWidth, int32 dstHeight, int32 & width, int32 & height, 
                         int32 & yAddSrc, int32 & xAddDst, int32 & yAddDst);
    
};



#endif // __IMAGE_RASTERIZER_H__