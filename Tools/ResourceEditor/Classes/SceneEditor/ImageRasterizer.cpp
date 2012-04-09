#include "ImageRasterizer.h"


void ImageRasterizer::DrawRelative(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(src && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
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
                else if(255.f < newValue)
                {
                    newValue = 255.f;
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

void ImageRasterizer::DrawRelativeRGBA(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(src && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += (srcOffset * 4);
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
                else if(255.f < newValue)
                {
                    newValue = 255.f;
                }
                
                *dstData = newValue;
                
//                srcData++;
                srcData += 4;

                dstData += xAddDst;
            }
            srcData += (yAddSrc * 4);
            dstData += yAddDst;
        }
    }
}


void ImageRasterizer::DrawAverage(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(mask && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *maskData = mask->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        maskData += srcOffset;
        dstData += dstOffset;
        
        const uint8 *maskDataSaved = maskData;
        uint8 *dstDataSaved = dstData;
        
        float32 average = 0.0f;
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
        
        if(count && time)
        {
            average /= count;

            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        *dstDataSaved = *dstDataSaved + (average - *dstDataSaved) * k;
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


void ImageRasterizer::DrawAverageRGBA(Image *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k)
{
    if(mask && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *maskData = mask->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        maskData += (srcOffset * 4);
        dstData += dstOffset;
        
        const uint8 *maskDataSaved = maskData;
        uint8 *dstDataSaved = dstData;
        
        float32 average = 0.0f;
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
                
//                maskData++;
                maskData += 4;

                dstData += xAddDst;
            }
            maskData += yAddSrc;
            dstData += yAddDst;
        }
        
        if(count && time)
        {
            average /= count;
            
            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        *dstDataSaved = *dstDataSaved + (average - *dstDataSaved) * k;
                    }
                    
//                    maskDataSaved++;
                    maskDataSaved += 4;

                    dstDataSaved += xAddDst;
                }
                maskDataSaved += yAddSrc;
                dstDataSaved += yAddDst;
            }
        }
    }
    
}


void ImageRasterizer::DrawAbsolute(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k, float32 dstHeight)
{
    if(src && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
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
                    else if(255.f < newValue)
                    {
                        newValue = 255.f;
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

void ImageRasterizer::DrawAbsoluteRGBA(Image *dst, Image *src, int32 x, int32 y, int32 width, int32 height, float32 k, float32 dstHeight)
{
    if(src && dst)
    {
        uint8 *dstData = dst->data;
        const uint8 *srcData = src->data;
        
        
        uint32 cntX,cntY;
        
        int32 yAddSrc;
        int32 xAddDst;
        int32 yAddDst;
        int32 srcOffset;
        int32 dstOffset;
        
        if (!Clipping(srcOffset,dstOffset,x,y, dst->width, dst->height, width,height, yAddSrc, xAddDst, yAddDst))
            return;
        
        srcData += (srcOffset * 4);
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
                    else if(255.f < newValue)
                    {
                        newValue = 255.f;
                    }
                    
                    *dstData = newValue;
                }
                
                //                srcData++;
                srcData += 4;
                
                dstData += xAddDst;
            }
            srcData += (yAddSrc * 4);
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