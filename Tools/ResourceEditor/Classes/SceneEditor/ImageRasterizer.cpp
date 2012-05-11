#include "ImageRasterizer.h"

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
        
        float64 average = 0.f;
        float64 maskMax = 0.f;
        int32 count = 0;
        
        yAddSrc *= 4;
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*maskData)
                {
                    maskMax = Max(maskMax, (float64)*maskData);
                    
                    average += *dstData;
                    ++count;
                }
                
                maskData += 4;
                dstData += xAddDst;
            }
            maskData += yAddSrc;
            dstData += yAddDst;
        }
        
        if(count && k && maskMax)
        {
            average /= count;
            
            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        float64 koef = Min(k * ((float64)*maskDataSaved / maskMax), (float64)1.0f);
                        
                        float64 newValue = (float64)*dstDataSaved + (float64)(average - *dstDataSaved) * koef;
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


void ImageRasterizer::DrawAbsoluteRGBA(Heightmap *dst, Image *mask, int32 x, int32 y, int32 width, int32 height, float32 k, float32 dstHeight)
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
        
        float64 maskMax = 0.f;
        
        yAddSrc *= 4;
        for(cntY = height; cntY > 0; cntY--)
        {
            for(cntX = width; cntX > 0; cntX--)
            {
                if(*maskData)
                {
                    maskMax = Max(maskMax, (float64)*maskData);
                }
                
                maskData += 4;
                dstData += xAddDst;
            }
            maskData += yAddSrc;
            dstData += yAddDst;
        }
        
        if(k && maskMax)
        {
            for(cntY = height; cntY > 0; cntY--)
            {
                for(cntX = width; cntX > 0; cntX--)
                {
                    if(*maskDataSaved)
                    {
                        float64 newValue = (float64)*dstDataSaved + 
                        (float64)(dstHeight - *dstDataSaved) * k * ((float64)*maskDataSaved / maskMax);
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


void ImageRasterizer::DrawCopypasteRGBA(Heightmap *dst, Image *mask, const Vector2 &posFrom, const Vector2 &posTo, int32 width, int32 height, float32 koef)
{
    //Find max mask value
    float64 maskMax = 0;
    uint8 *maskData = mask->GetData();
    for(int32 iRow = 0; iRow < height; ++iRow)
    {
        for(int32 iCol = 0; iCol < width; ++iCol)
        {
            maskMax = Max(maskMax, (float64)*maskData);
            maskData += 4;
        }
    }

    if(!maskMax) return;
    
    //copy-paste
    uint16 *dstData = dst->Data();
    for(int32 iRow = 0; iRow < height; ++iRow)
    {
        int32 ySrc = posFrom.y + iRow;
        int32 yDst = posTo.y + iRow;
        
        if(     (0 <= ySrc && ySrc < dst->Size())
           &&   (0 <= yDst && yDst < dst->Size()))
        {
            int32 srcIndex = ySrc * dst->Size();
            int32 dstIndex = yDst * dst->Size();
            
            for(int32 iCol = 0; iCol < width; ++iCol)
            {
                int32 xSrc = posFrom.x + iCol;
                int32 xDst = posTo.x + iCol;

                if(     (0 <= xSrc && xSrc < dst->Size())
                   &&   (0 <= xDst && xDst < dst->Size()))
                {
                    uint8 maskData = mask->data[(iRow * width + iCol) * 4];
                    if(maskData)
                    {
                        int32 dstOffset = (dstIndex + xDst);
                        int32 srcOffset = (srcIndex + xSrc);
                        
                        dstData[dstOffset] = dstData[dstOffset] + 
                                                (dstData[srcOffset] - dstData[dstOffset]) * koef * maskData / maskMax;
                    }
                }
            }
        }
    }
}

void ImageRasterizer::DrawCopypasteRGBA(Image *src, Image *dst, Image *mask, const Vector2 &posFrom, const Vector2 &posTo, int32 width, int32 height)
{
    DVASSERT(src->width == dst->width);
    DVASSERT(src->height == dst->height);
    DVASSERT(src->format == dst->format);
    
    int32 formatSize = Image::GetFormatSize(dst->format);
    
    //copy-paste
    uint8 *srcData = src->data;
    uint8 *dstData = dst->data;
    for(int32 iRow = 0; iRow < height; ++iRow)
    {
        int32 ySrc = posFrom.y + iRow;
        int32 yDst = posTo.y + iRow;
        
        if(     (0 <= ySrc && ySrc < src->height)
           &&   (0 <= yDst && yDst < dst->height))
        {
            int32 srcIndex = ySrc * src->width;
            int32 dstIndex = yDst * dst->width;
            
            for(int32 iCol = 0; iCol < width; ++iCol)
            {
                int32 xSrc = posFrom.x + iCol;
                int32 xDst = posTo.x + iCol;
                
                if(     (0 <= xSrc && xSrc < src->width)
                   &&   (0 <= xDst && xDst < dst->width))
                {
                    uint8 maskData = mask->data[(iRow * width + iCol) * 4];
                    if(maskData)
                    {
                        int64 dstOffset = (dstIndex + xDst) * formatSize;
                        int64 srcOffset = (srcIndex + xSrc) * formatSize;
                        
                        for(int32 i = 0; i < formatSize; ++i)
                        {
                            dstData[dstOffset + i] = srcData[srcOffset + i];
                        }
                    }
                }
            }
        }
    }
}


