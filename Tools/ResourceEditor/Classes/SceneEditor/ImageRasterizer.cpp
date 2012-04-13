#include "ImageRasterizer.h"


void ImageRasterizer::DrawRelative(Heightmap *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(src && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *srcData = src->data;
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += srcOffset;
        dstData += dstOffset;

        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                float32 newValue = *dstData + (*srcData * k);
                if(newValue < 0.f)
                {
                    newValue = 0.f;
                }
                else if(Heightmap::MAX_VALUE < newValue)
                {
                    newValue = Heightmap::MAX_VALUE;
                }
                
                *dstData = newValue;
                
                srcData++;
                dstData += xAddDst;
            }
            srcData += yAddSrc;
            dstData += yAddDst;
        }
    }
}

void ImageRasterizer::DrawRelativeRGBA(Heightmap *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(src && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += (srcOffset * 4);
        dstData += dstOffset;
        
        yAddSrc *= 4;
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                float32 newValue = *dstData + (*srcData * k);
                if(newValue < 0.f)
                {
                    newValue = 0.f;
                }
                else if(Heightmap::MAX_VALUE < newValue)
                {
                    newValue = Heightmap::MAX_VALUE;
                }
                
                *dstData = newValue;
                
                srcData += 4;
                dstData += xAddDst;
            }
            srcData += yAddSrc;
            dstData += yAddDst;
        }
    }
}


void ImageRasterizer::DrawAverage(Heightmap *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(mask && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *maskData = mask->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        maskData += srcOffset;
        dstData += dstOffset;
        
        uint8 *maskDataSaved = maskData;
        uint16 *dstDataSaved = dstData;
        
        float64 average = 0.0f;
        int32 count = 0;
        
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*maskData)
                {
                    average += *dstData;
                    ++count;
                }
                
                maskData++;
                dstData += xAddDst;
            }
            maskData += yAddSrc;
            dstData += yAddDst;
        }
        
        if(count && k)
        {
            average /= count;

            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        float32 newValue = *dstDataSaved + (average - *dstDataSaved) * k;
                        if(newValue < 0.f)
                        {
                            newValue = 0.f;
                        }
                        else if(Heightmap::MAX_VALUE < newValue)
                        {
                            newValue = Heightmap::MAX_VALUE;
                        }
                        
                        *dstDataSaved = newValue;
                    }
                    
                    maskDataSaved++;
                    dstDataSaved += xAddDst;
                }
                maskDataSaved += yAddSrc;
                dstDataSaved += yAddDst;
            }
        }
    }
}


void ImageRasterizer::DrawAverageRGBA(Heightmap *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(mask && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *maskData = mask->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        maskData += (srcOffset * 4);
        dstData += dstOffset;
        
        uint16 *dstDataSaved = dstData;
        uint8 *maskDataSaved = maskData;
        
        float64 average = 0;
        int32 count = 0;
        
        yAddSrc *= 4;
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*maskData)
                {
                    average += *dstData;
                    ++count;
                }
                
                maskData += 4;
                dstData += xAddDst;
            }
            maskData += yAddSrc;
            dstData += yAddDst;
        }
        
        if(count && k)
        {
            average /= count;
            
            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        float64 newValue = (float64)*dstDataSaved + (float64)(average - *dstDataSaved) * k;
                        if(newValue < 0.f)
                        {
                            newValue = 0.f;
                        }
                        else if(Heightmap::MAX_VALUE < newValue)
                        {
                            newValue = Heightmap::MAX_VALUE;
                        }
                      
                        *dstDataSaved = newValue;
                    }
                    
                    maskDataSaved += 4;
                    dstDataSaved += xAddDst;
                }
                maskDataSaved += yAddSrc;
                dstDataSaved += yAddDst;
            }
        }
    }
}


void ImageRasterizer::DrawAbsolute(Heightmap *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k, float32 dstHeight)
{
    if(src && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += srcOffset;
        dstData += dstOffset;
        
        
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*srcData)
                {
                    float32 newValue = dstHeight + (*srcData * k);
                    if(newValue < 0.f)
                    {
                        newValue = 0.f;
                    }
                    else if(Heightmap::MAX_VALUE < newValue)
                    {
                        newValue = Heightmap::MAX_VALUE;
                    }
                    
                    *dstData = newValue;
                }
                
                srcData++;
                dstData += xAddDst;
            }
            srcData += yAddSrc;
            dstData += yAddDst;
        }
    }
}

void ImageRasterizer::DrawAbsoluteRGBA(Heightmap *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k, float32 dstHeight)
{
    if(src && dst)
    {
        uint16 *dstData = dst->Data();
        uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->Size(), dst->Size(), width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += (srcOffset * 4);
        dstData += dstOffset;
        
        
        yAddSrc *= 4;
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*srcData)
                {
                    float32 newValue = dstHeight + (*srcData * k);
                    if(newValue < 0.f)
                    {
                        newValue = 0.f;
                    }
                    else if(Heightmap::MAX_VALUE < newValue)
                    {
                        newValue = Heightmap::MAX_VALUE;
                    }
                    
                    *dstData = newValue;
                }
                
                srcData += 4;
                dstData += xAddDst;
            }
            srcData += yAddSrc;
            dstData += yAddDst;
        }
    }
}



bool ImageRasterizer::Clipping(int32 & srcOffset, 
                               int32 & dstOffset, 
                               int32 & dstX, 
                               int32 & dstY, 
                               int32 dstWidth,
                               int32 dstHeight,
                               int32 & width, 
                               int32 & height, 
                               int32 & yAddSrc, 
                               int32 & xAddDst, 
                               int32 & yAddDst)
{
    
	int32 xDecLeft = 0;
	int32 xDecRight = 0;
	int32 yDecUp = 0;
	int32 yDecDown = 0;
	int32 nWidth = width;
	int32 nHeight = height;
    
    
    int32 clipStartX = 0;
	int32 clipStartY = 0;
	int32 clipEndX = dstWidth - 1;
	int32 clipEndY = dstHeight - 1;


    if(dstX > clipEndX || dstY > clipEndY || dstX + nWidth < clipStartX || dstY + nHeight < clipStartY)
    {
        return false;    
    }
    
    if(dstX < clipStartX)
    {
        xDecLeft = clipStartX - dstX;
    }
    if(dstY < clipStartY)
    {
        yDecUp = clipStartY - dstY;
    }
    
    if(dstX + nWidth > clipEndX)
    {
        xDecRight = dstX + nWidth - clipEndX - 1;
    }
    if(dstY + nHeight > clipEndY)
    {
        yDecDown = dstY + nHeight - clipEndY - 1;
    }
    
    width -= (xDecRight + xDecLeft);
    height -= (yDecDown + yDecUp);
    dstOffset = (dstX + dstY * dstWidth) + dstWidth * yDecUp + xDecLeft;
    srcOffset = nWidth * yDecUp + xDecLeft;
    yAddSrc = nWidth - width;
    xAddDst = 1;
    yAddDst = dstWidth - width;
    
	return true;
}