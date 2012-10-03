/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/LibPVRHelper.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"
#include "Render/OGLHelpers.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"

#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
#include "libpvr/PVRTError.h"
#include "libpvr/PVRTDecompress.h"
#include "libpvr/PVRTMap.h"
#include "libpvr/PVRTextureHeader.h"
#include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)


namespace DAVA 
{
    
#if defined (__DAVAENGINE_IPHONE__)
    #define GL_HALF_FLOAT                                               0x140B
    
    /* GL_OES_compressed_ETC1_RGB8_texture */
    #ifndef GL_OES_compressed_ETC1_RGB8_texture
        #define GL_ETC1_RGB8_OES                                        0x8D64
    #endif

#else //#if defined (__DAVAENGINE_IPHONE__)
    /* GL_IMG_texture_compression_pvrtc */
    #ifndef GL_IMG_texture_compression_pvrtc
        #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
        #define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
        #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
        #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
    #endif
    
    /* GL_OES_compressed_ETC1_RGB8_texture */
    #ifndef GL_OES_compressed_ETC1_RGB8_texture
        #define GL_ETC1_RGB8_OES                                        0x8D64
    #endif
#endif //#if defined (__DAVAENGINE_IPHONE__)
    

    
uint32 LibPVRHelper::GetBitsPerPixel(uint64 pixelFormat)
{
#if defined (__DAVAENGINE_IPHONE__)
    if((pixelFormat & PVRTEX_PFHIGHMASK) != 0)
    {
        uint8 *pixelFormatChar = (uint8 *)&pixelFormat;
        return (pixelFormatChar[4] + pixelFormatChar[5] + pixelFormatChar[6] + pixelFormatChar[7]);
    }
    else
    {
        switch (pixelFormat)
        {
            case ePVRTPF_BW1bpp:
                return 1;
            case ePVRTPF_PVRTCI_2bpp_RGB:
            case ePVRTPF_PVRTCI_2bpp_RGBA:
            case ePVRTPF_PVRTCII_2bpp:
                return 2;
            case ePVRTPF_PVRTCI_4bpp_RGB:
            case ePVRTPF_PVRTCI_4bpp_RGBA:
            case ePVRTPF_PVRTCII_4bpp:
            case ePVRTPF_ETC1:
            case ePVRTPF_EAC_R11:
            case ePVRTPF_ETC2_RGB:
            case ePVRTPF_ETC2_RGB_A1:
            case ePVRTPF_DXT1:
            case ePVRTPF_BC4:
                return 4;
            case ePVRTPF_DXT2:
            case ePVRTPF_DXT3:
            case ePVRTPF_DXT4:
            case ePVRTPF_DXT5:
            case ePVRTPF_BC5:
            case ePVRTPF_EAC_RG11:
            case ePVRTPF_ETC2_RGBA:
                return 8;
            case ePVRTPF_YUY2:
            case ePVRTPF_UYVY:
            case ePVRTPF_RGBG8888:
            case ePVRTPF_GRGB8888:
                return 16;
            case ePVRTPF_SharedExponentR9G9B9E5:
                return 32;
            case ePVRTPF_NumCompressedPFs:
                return 0;
        }
    }
    return 0;
#else //__DAVAENGINE_IPHONE__
    return PVRTGetBitsPerPixel(pixelFormat);
#endif //__DAVAENGINE_IPHONE__
}

    
void LibPVRHelper::GetFormatMinDims(uint64 pixelFormat, uint32 &minX, uint32 &minY, uint32 &minZ)
{
#if defined (__DAVAENGINE_IPHONE__)
    switch(pixelFormat)
    {
        case ePVRTPF_DXT1:
        case ePVRTPF_DXT2:
        case ePVRTPF_DXT3:
        case ePVRTPF_DXT4:
        case ePVRTPF_DXT5:
        case ePVRTPF_BC4:
        case ePVRTPF_BC5:
        case ePVRTPF_ETC1:
        case ePVRTPF_ETC2_RGB:
        case ePVRTPF_ETC2_RGBA:
        case ePVRTPF_ETC2_RGB_A1:
        case ePVRTPF_EAC_R11:
        case ePVRTPF_EAC_RG11:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_4bpp_RGB:
        case ePVRTPF_PVRTCI_4bpp_RGBA:
            minX = 8;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_2bpp_RGB:
        case ePVRTPF_PVRTCI_2bpp_RGBA:
            minX = 16;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_4bpp:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_2bpp:
            minX = 8;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_UYVY:
        case ePVRTPF_YUY2:
        case ePVRTPF_RGBG8888:
        case ePVRTPF_GRGB8888:
            minX = 2;
            minY = 1;
            minZ = 1;
            break;
        case ePVRTPF_BW1bpp:
            minX = 8;
            minY = 1;
            minZ = 1;
            break;
        default: //Non-compressed formats all return 1.
            minX = 1;
            minY = 1;
            minZ = 1;
            break;
    }
#else //__DAVAENGINE_IPHONE__
    PVRTGetFormatMinDims(pixelFormat, minX, minY, minZ);
#endif //__DAVAENGINE_IPHONE__
}

    
uint32 LibPVRHelper::GetTextureDataSize(PVRHeaderV3 textureHeader, int32 mipLevel, bool allSurfaces, bool allFaces)
{
#if defined (__DAVAENGINE_IPHONE__)
    //The smallest divisible sizes for a pixel format
    uint32 uiSmallestWidth = 1;
    uint32 uiSmallestHeight = 1;
    uint32 uiSmallestDepth = 1;
    
    uint64 PixelFormatPartHigh = textureHeader.u64PixelFormat & PVRTEX_PFHIGHMASK;
    
    //If the pixel format is compressed, get the pixel format's minimum dimensions.
    if (PixelFormatPartHigh==0)
    {
        GetFormatMinDims(textureHeader.u64PixelFormat, uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);
    }
    
    //Needs to be 64-bit integer to support 16kx16k and higher sizes.
    uint64 uiDataSize = 0;
    if (mipLevel == -1)
    {
        for (uint8 uiCurrentMIP = 0; uiCurrentMIP<textureHeader.u32MIPMapCount; ++uiCurrentMIP)
        {
            //Get the dimensions of the current MIP Map level.
            uint32 uiWidth = PVRT_MAX(1,textureHeader.u32Width>>uiCurrentMIP);
            uint32 uiHeight = PVRT_MAX(1,textureHeader.u32Height>>uiCurrentMIP);
            uint32 uiDepth = PVRT_MAX(1,textureHeader.u32Depth>>uiCurrentMIP);
            
            //If pixel format is compressed, the dimensions need to be padded.
            if (PixelFormatPartHigh==0)
            {
                uiWidth=uiWidth+( (-1*uiWidth)%uiSmallestWidth);
                uiHeight=uiHeight+( (-1*uiHeight)%uiSmallestHeight);
                uiDepth=uiDepth+( (-1*uiDepth)%uiSmallestDepth);
            }
            
            //Add the current MIP Map's data size to the total.
            uiDataSize+=(uint64)GetBitsPerPixel(textureHeader.u64PixelFormat)*(uint64)uiWidth*(uint64)uiHeight*(uint64)uiDepth;
        }
    }
    else
    {
        //Get the dimensions of the specified MIP Map level.
        uint32 uiWidth = PVRT_MAX(1,textureHeader.u32Width>>mipLevel);
        uint32 uiHeight = PVRT_MAX(1,textureHeader.u32Height>>mipLevel);
        uint32 uiDepth = PVRT_MAX(1,textureHeader.u32Depth>>mipLevel);
        
        //If pixel format is compressed, the dimensions need to be padded.
        if (PixelFormatPartHigh==0)
        {
            uiWidth=uiWidth+( (-1*uiWidth)%uiSmallestWidth);
            uiHeight=uiHeight+( (-1*uiHeight)%uiSmallestHeight);
            uiDepth=uiDepth+( (-1*uiDepth)%uiSmallestDepth);
        }
        
        //Work out the specified MIP Map's data size
        uiDataSize=GetBitsPerPixel(textureHeader.u64PixelFormat)*uiWidth*uiHeight*uiDepth;
    }
    
    //The number of faces/surfaces to register the size of.
    uint32 numfaces = ((allFaces)?(textureHeader.u32NumFaces):(1));
    uint32 numsurfs = ((allSurfaces)?(textureHeader.u32NumSurfaces):(1));
    
    //Multiply the data size by number of faces and surfaces specified, and return.
    return (uint32)(uiDataSize/8)*numsurfs*numfaces;

#else //#if defined (__DAVAENGINE_IPHONE__)
    PVRTextureHeaderV3 *header = (PVRTextureHeaderV3 *)&textureHeader;
    return PVRTGetTextureDataSize(*header, mipLevel, allSurfaces, allFaces);
#endif //#if defined (__DAVAENGINE_IPHONE__)
}
 
void LibPVRHelper::MapLegacyTextureEnumToNewFormat(PVRTPixelType OldFormat, uint64& newType, EPVRTColourSpace& newCSpace, EPVRTVariableType& newChanType, bool& isPreMult)
{
#if defined (__DAVAENGINE_IPHONE__)
    //Default value.
    isPreMult=false;
    
    switch (OldFormat)
    {
        case MGLPT_ARGB_4444:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_ARGB_1555:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_555:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_888:
        {
            newType=PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case MGLPT_ARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case MGLPT_ARGB_8332:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,3,3,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_I_8:
        {
            newType=PVRTGENPIXELID1('i',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_AI_88:
        {
            newType=PVRTGENPIXELID2('a','i',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_1_BPP:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_VY1UY0:
        {
            newType=ePVRTPF_YUY2;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_Y1VY0U:
        {
            newType=ePVRTPF_UYVY;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_PVRTC2:
        {
            newType=ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_PVRTC4:
        {
            newType=ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_RGBA_4444:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGBA_5551:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_RGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGB_555:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',5,5,5,1);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGB_888:
        {
            newType=PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_I_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_AI_88:
        {
            newType=PVRTGENPIXELID2('l','a',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTC2:
        {
            newType=ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTC4:
        {
            newType=ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_BGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_A_8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTCII4:
        {
            newType=ePVRTPF_PVRTCII_4bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTCII2:
        {
            newType=ePVRTPF_PVRTCII_2bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
//#ifdef _WIN32
//        case D3D_DXT1:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//        case D3D_DXT2:
//        {
//            newType=ePVRTPF_DXT2;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            isPreMult=true;
//            break;
//        }
//            
//        case D3D_DXT3:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//        case D3D_DXT4:
//        {
//            newType=ePVRTPF_DXT4;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            isPreMult=true;
//            break;
//        }
//            
//        case D3D_DXT5:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//#endif
        case D3D_RGB_332:
        {
            newType=PVRTGENPIXELID3('r','g','b',3,3,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AL_44:
        {
            newType=PVRTGENPIXELID2('a','l',4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_LVU_655:
        {
            newType=PVRTGENPIXELID3('l','g','r',6,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_XLVU_8888:
        {
            newType=PVRTGENPIXELID4('x','l','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_QWVU_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_ABGR_2101010:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_ARGB_2101010:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AWVU_2101010:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_GR_1616:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_VU_1616:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_ABGR_16161616:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_R16F:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_GR_1616F:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_ABGR_16161616F:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_R32F:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_GR_3232F:
        {
            newType=PVRTGENPIXELID2('g','r',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_ABGR_32323232F:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case ETC_RGB_4BPP:
        {
            newType=ePVRTPF_ETC1;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case D3D_A8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_V8U8:
        {
            newType=PVRTGENPIXELID2('g','r',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_L16:
        {
            newType=PVRTGENPIXELID1('l',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_L8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AL_88:
        {
            newType=PVRTGENPIXELID2('a','l',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_UYVY:
        {
            newType=ePVRTPF_UYVY;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case D3D_YUY2:
        {
            newType=ePVRTPF_YUY2;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R32G32B32A32_FLOAT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32B32A32_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32B32A32_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R32G32B32_FLOAT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32B32_UINT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32B32_SINT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R16G16B16A16_FLOAT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16G16B16A16_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16G16B16A16_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16G16B16A16_SNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16G16B16A16_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R32G32_FLOAT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R10G10B10A2_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case DX10_R10G10B10A2_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R11G11B10_FLOAT:
        {
            newType=PVRTGENPIXELID3('r','g','b',11,11,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R8G8B8A8_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_UNORM_SRGB:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8G8B8A8_SNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_R16G16_FLOAT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16G16_UNORM:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16G16_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16G16_SNORM:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16G16_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R32_FLOAT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32_UINT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32_SINT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R8G8_UNORM:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8G8_SNORM:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8G8_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_R16_FLOAT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16_UNORM:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16_UINT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16_SNORM:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16_SINT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R8_UNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8_UINT:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8_SNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8_SINT:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_A8_UNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R1_UNORM:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R9G9B9E5_SHAREDEXP:
        {
            newType=ePVRTPF_SharedExponentR9G9B9E5;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R8G8_B8G8_UNORM:
        {
            newType=ePVRTPF_RGBG8888;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_G8R8_G8B8_UNORM:
        {
            newType=ePVRTPF_GRGB8888;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
//#ifdef _WIN32
//        case DX10_BC1_UNORM:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC1_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC2_UNORM:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC2_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC3_UNORM:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC3_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC4_UNORM:
//        {
//            newType=ePVRTPF_BC4;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC4_SNORM:
//        {
//            newType=ePVRTPF_BC4;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeSignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC5_UNORM:
//        {
//            newType=ePVRTPF_BC5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC5_SNORM:
//        {
//            newType=ePVRTPF_BC5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeSignedIntegerNorm;
//            break;
//        }
//            
//#endif
        case ePT_VG_sRGBX_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sRGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sRGBA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sRGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sRGBA_5551:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sRGBA_4444:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sL_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBX_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_lL_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_A_8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_BW_1:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sXRGB_8888:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sARGB_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sARGB_1555:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sARGB_4444:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lXRGB_8888:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lARGB_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sBGRX_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sBGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sBGRA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sBGR_565:
        {
            newType=PVRTGENPIXELID3('b','g','r',5,6,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sBGRA_5551:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',5,5,5,1);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sBGRA_4444:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lBGRX_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lBGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lBGRA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sXBGR_8888:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sABGR_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sABGR_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sABGR_1555:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',1,5,5,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sABGR_4444:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lXBGR_8888:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lABGR_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lABGR_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
        default:
        {
            newType=ePVRTPF_NumCompressedPFs;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeNumVarTypes;
            break;
        }
    }
    
#else //#if defined (__DAVAENGINE_IPHONE__)
    PVRTMapLegacyTextureEnumToNewFormat(OldFormat, newType, newCSpace, newChanType, isPreMult);
#endif //#if defined (__DAVAENGINE_IPHONE__)
}

void LibPVRHelper::ConvertOldTextureHeaderToV3(const PVRHeaderV2* LegacyHeader, PVRHeaderV3& NewHeader)
{
#if defined (__DAVAENGINE_IPHONE__)
    //Setup variables
    bool isPreMult;
    uint64 ptNew;
    EPVRTColourSpace cSpaceNew;
    EPVRTVariableType chanTypeNew;
    
    //Map the old enum to the new format.
    MapLegacyTextureEnumToNewFormat((PVRTPixelType)(LegacyHeader->dwpfFlags&0xff),ptNew,cSpaceNew,chanTypeNew,isPreMult);
    
    //Check if this is a cube map.
    bool isCubeMap = (LegacyHeader->dwpfFlags&PVRTEX_CUBEMAP)!=0;
    
    //Setup the new header.
    NewHeader.u64PixelFormat=ptNew;
    NewHeader.u32ChannelType=chanTypeNew;
    NewHeader.u32ColourSpace=cSpaceNew;
    NewHeader.u32Depth=1;
    NewHeader.u32Flags=isPreMult?PVRTEX3_PREMULTIPLIED:0;
    NewHeader.u32Height=LegacyHeader->dwHeight;
    NewHeader.u32MetaDataSize=0;
    NewHeader.u32MIPMapCount=(LegacyHeader->dwpfFlags&PVRTEX_MIPMAP?LegacyHeader->dwMipMapCount+1:1); //Legacy headers have a MIP Map count of 0 if there is only the top level. New Headers have a count of 1.
    NewHeader.u32NumFaces=(isCubeMap?6:1);
    
    //Only compute the number of surfaces if it's a V2 header, else default to 1 surface.
    if (LegacyHeader->dwHeaderSize==sizeof(PVRHeaderV2))
        NewHeader.u32NumSurfaces=(LegacyHeader->dwNumSurfs/(isCubeMap?6:1));
    else
        NewHeader.u32NumSurfaces=1;
    
    NewHeader.u32Version=PVRTEX3_IDENT;
    NewHeader.u32Width=LegacyHeader->dwWidth;
    
//    //Clear any currently stored MetaData, or it will be inaccurate.
//    if (pMetaData)
//    {
//        pMetaData->Clear();
//    }
//    
//    //Check if this is a normal map.
//    if (LegacyHeader->dwpfFlags&PVRTEX_BUMPMAP && pMetaData)
//    {
//        //Get a reference to the correct block.
//        MetaDataBlock& mbBumpData=(*pMetaData)[PVRTEX_CURR_IDENT][ePVRTMetaDataBumpData];
//        
//        //Set up the block.
//        mbBumpData.DevFOURCC=PVRTEX_CURR_IDENT;
//        mbBumpData.u32Key=ePVRTMetaDataBumpData;
//        mbBumpData.u32DataSize=8;
//        mbBumpData.Data=new PVRTuint8[8];
//        
//        //Setup the data for the block.
//        float bumpScale = 1.0f;
//        const char* bumpOrder = "xyz";
//        
//        //Copy the bumpScale into the data.
//        memcpy(mbBumpData.Data,&bumpScale,4);
//        
//        //Clear the string
//        memset(mbBumpData.Data+4,0,4);
//        
//        //Copy the bumpOrder into the data.
//        memcpy(mbBumpData.Data+4, bumpOrder,3);
//        
//        //Increment the meta data size.
//        NewHeader.u32MetaDataSize+=(12+mbBumpData.u32DataSize);
//    }
//    
//    //Check if for vertical flip orientation.
//    if (LegacyHeader->dwpfFlags&PVRTEX_VERTICAL_FLIP && pMetaData)
//    {
//        //Get the correct meta data block
//        MetaDataBlock& mbTexOrientation=(*pMetaData)[PVRTEX_CURR_IDENT][ePVRTMetaDataTextureOrientation];
//        
//        //Set the block up.
//        mbTexOrientation.u32DataSize=3;
//        mbTexOrientation.Data=new PVRTuint8[3];
//        mbTexOrientation.DevFOURCC=PVRTEX_CURR_IDENT;
//        mbTexOrientation.u32Key=ePVRTMetaDataTextureOrientation;
//        
//        //Initialise the block to default orientation.
//        memset(mbTexOrientation.Data,0,3);
//        
//        //Set the block oriented upwards.
//        mbTexOrientation.Data[ePVRTAxisY]=ePVRTOrientUp;
//        
//        //Increment the meta data size.
//        NewHeader.u32MetaDataSize+=(12+mbTexOrientation.u32DataSize);
//    }

#else //#if defined (__DAVAENGINE_IPHONE__)
    PVRTConvertOldTextureHeaderToV3((const PVR_Texture_Header *)LegacyHeader, (PVRTextureHeaderV3&)NewHeader, NULL);
#endif //#if defined (__DAVAENGINE_IPHONE__)
}

    
bool LibPVRHelper::IsGLExtensionSupported(const char * const extension)
{
    // The recommended technique for querying OpenGL extensions;
    // from http://opengl.org/resources/features/OGLextensions/
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;
    
    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0')
        return 0;
    
    extensions = glGetString(GL_EXTENSIONS);
    
    /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings, etc. */
    start = extensions;
    for (;;) {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;
        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return true;
        start = terminator;
    }
    
    return false;
}


const PixelFormatDescriptor LibPVRHelper::GetCompressedFormat(const uint64 PixelFormat)
{
    PixelFormatDescriptor formatDescriptor;
    switch (PixelFormat)
    {
        case ePVRTPF_PVRTCI_2bpp_RGB:
        {
            formatDescriptor.internalformat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            formatDescriptor.formatID = FORMAT_PVR2;
            break;
        }
        case ePVRTPF_PVRTCI_2bpp_RGBA:
        {
            formatDescriptor.internalformat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            formatDescriptor.formatID = FORMAT_PVR2;
            break;
        }
        case ePVRTPF_PVRTCI_4bpp_RGB:
        {
            formatDescriptor.internalformat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            formatDescriptor.formatID = FORMAT_PVR4;
            break;
        }
        case ePVRTPF_PVRTCI_4bpp_RGBA:
        {
            formatDescriptor.internalformat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            formatDescriptor.formatID = FORMAT_PVR4;
            break;
        }
        case ePVRTPF_ETC1:
        {
            formatDescriptor.internalformat = GL_ETC1_RGB8_OES;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
            
        default:
            break;
    }
    
    return formatDescriptor;
}
    
const PixelFormatDescriptor LibPVRHelper::GetFloatTypeFormat(const uint64 PixelFormat)
{
    PixelFormatDescriptor formatDescriptor;
    switch (PixelFormat)
    {
            //HALF_FLOAT_OES
        case PVRTGENPIXELID4('r','g','b','a',16,16,16,16):
        {
            formatDescriptor.type=GL_HALF_FLOAT;
            formatDescriptor.format = GL_RGBA;
            formatDescriptor.internalformat=GL_RGBA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID3('r','g','b',16,16,16):
        {
            formatDescriptor.type=GL_HALF_FLOAT;
            formatDescriptor.format = GL_RGB;
            formatDescriptor.internalformat=GL_RGB;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID2('l','a',16,16):
        {
            formatDescriptor.type=GL_HALF_FLOAT;
            formatDescriptor.format = GL_LUMINANCE_ALPHA;
            formatDescriptor.internalformat=GL_LUMINANCE_ALPHA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('l',16):
        {
            formatDescriptor.type=GL_HALF_FLOAT;
            formatDescriptor.format = GL_LUMINANCE;
            formatDescriptor.internalformat=GL_LUMINANCE;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('a',16):
        {
            formatDescriptor.type=GL_HALF_FLOAT;
            formatDescriptor.format = GL_ALPHA;
            formatDescriptor.internalformat=GL_ALPHA;
            formatDescriptor.formatID = FORMAT_A16;
            break;
        }
            //FLOAT (OES)
        case PVRTGENPIXELID4('r','g','b','a',32,32,32,32):
        {
            formatDescriptor.type=GL_FLOAT;
            formatDescriptor.format = GL_RGBA;
            formatDescriptor.internalformat=GL_RGBA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID3('r','g','b',32,32,32):
        {
            formatDescriptor.type=GL_FLOAT;
            formatDescriptor.format = GL_RGB;
            formatDescriptor.internalformat=GL_RGB;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID2('l','a',32,32):
        {
            formatDescriptor.type=GL_FLOAT;
            formatDescriptor.format = GL_LUMINANCE_ALPHA;
            formatDescriptor.internalformat=GL_LUMINANCE_ALPHA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('l',32):
        {
            formatDescriptor.type=GL_FLOAT;
            formatDescriptor.format = GL_LUMINANCE;
            formatDescriptor.internalformat=GL_LUMINANCE;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('a',32):
        {
            formatDescriptor.type=GL_FLOAT;
            formatDescriptor.format = GL_ALPHA;
            formatDescriptor.internalformat=GL_ALPHA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
    }
    
    return formatDescriptor;
}

const PixelFormatDescriptor LibPVRHelper::GetUnsignedByteFormat(const uint64 PixelFormat)
{
    PixelFormatDescriptor formatDescriptor;
    formatDescriptor.type = GL_UNSIGNED_BYTE;
    
    switch (PixelFormat)
    {
        case PVRTGENPIXELID4('r','g','b','a',8,8,8,8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_RGBA;
            formatDescriptor.formatID = FORMAT_RGBA8888;
            break;
        }
        case PVRTGENPIXELID3('r','g','b',8,8,8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_RGB;
            formatDescriptor.formatID = FORMAT_RGB888;
            break;
        }
        case PVRTGENPIXELID2('l','a',8,8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_LUMINANCE_ALPHA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('l',8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_LUMINANCE;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
        case PVRTGENPIXELID1('a',8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_ALPHA;
            formatDescriptor.formatID = FORMAT_A8;
            break;
        }
        case PVRTGENPIXELID4('b','g','r','a',8,8,8,8):
        {
            formatDescriptor.format = formatDescriptor.internalformat = GL_BGRA;
            formatDescriptor.formatID = FORMAT_INVALID;
            break;
        }
    }

    return formatDescriptor;
}
    
const PixelFormatDescriptor LibPVRHelper::GetUnsignedShortFormat(const uint64 PixelFormat)
{
    PixelFormatDescriptor formatDescriptor;

    switch (PixelFormat)
    {
        case PVRTGENPIXELID4('r','g','b','a',4,4,4,4):
        {
            formatDescriptor.type = GL_UNSIGNED_SHORT_4_4_4_4;
            formatDescriptor.format = formatDescriptor.internalformat = GL_RGBA;
            formatDescriptor.formatID = FORMAT_RGBA4444;
            break;
        }
        case PVRTGENPIXELID4('r','g','b','a',5,5,5,1):
        {
            formatDescriptor.type = GL_UNSIGNED_SHORT_5_5_5_1;
            formatDescriptor.format = formatDescriptor.internalformat = GL_RGBA;
            formatDescriptor.formatID = FORMAT_RGBA5551;
            break;
        }
        case PVRTGENPIXELID3('r','g','b',5,6,5):
        {
            formatDescriptor.type = GL_UNSIGNED_SHORT_5_6_5;
            formatDescriptor.format = formatDescriptor.internalformat = GL_RGB;
            formatDescriptor.formatID = FORMAT_RGB565;
            break;
        }
    }
    
    return formatDescriptor;
}
    
const PixelFormatDescriptor LibPVRHelper::GetTextureFormat(const PVRHeaderV3& textureHeader)
{
    uint64 PixelFormat = textureHeader.u64PixelFormat;
    EPVRTVariableType ChannelType = (EPVRTVariableType)textureHeader.u32ChannelType;
    
    //Get the last 32 bits of the pixel format.
    uint64 PixelFormatPartHigh = PixelFormat&PVRTEX_PFHIGHMASK;
    
    //Check for a compressed format (The first 8 bytes will be 0, so the whole thing will be equal to the last 32 bits).
    PixelFormatDescriptor formatDescriptor;
    if (PixelFormatPartHigh==0)
    {
        //Format and type == 0 for compressed textures.
        formatDescriptor = GetCompressedFormat(PixelFormat);
    }
    else
    {
        switch (ChannelType)
        {
            case ePVRTVarTypeFloat:
            {
                formatDescriptor = GetFloatTypeFormat(PixelFormat);
                break;
            }
            case ePVRTVarTypeUnsignedByteNorm:
            {
                formatDescriptor = GetUnsignedByteFormat(PixelFormat);
                break;
            }
            case ePVRTVarTypeUnsignedShortNorm:
            {
                formatDescriptor = GetUnsignedShortFormat(PixelFormat);
            }
            default:
                break;
        }
    }
    
    return formatDescriptor;
}

bool LibPVRHelper::FillTextureWithPVRData(const char* pvrData, const int32 pvrDataSize, Texture *texture)
{
    //Compression bools
    bool bIsCompressedFormatSupported=false;
    bool bIsCompressedFormat=false;
    bool bIsLegacyPVR=false;
    
    //Texture setup
    PVRHeaderV3 sTextureHeader;
    uint8* pTextureData=NULL;
    
    //Just in case header and pointer for decompression.
    PVRHeaderV3 sTextureHeaderDecomp;
    void* pDecompressedData=NULL;
    
    //Check if it's an old header format
    if((*(uint32*)pvrData)!=PVRTEX3_IDENT)
    {
        ConvertOldTextureHeaderToV3((PVRHeaderV2 *)pvrData,sTextureHeader);
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData + *(uint32*)pvrData;
        
        bIsLegacyPVR=true;
    }
    else
    {
        //Get the header from the main pointer.
        sTextureHeader=*(PVRHeaderV3*)pvrData;
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData+PVRTEX3_HEADERSIZE+sTextureHeader.u32MetaDataSize;
        
//        CPVRTMap<unsigned int, CPVRTMap<unsigned int, struct MetaDataBlock> > *pMetaData=NULL;
//        if (pMetaData)
//        {
//            //Read in all the meta data.
//            PVRTuint32 metaDataSize=0;
//            while (metaDataSize<sTextureHeader.u32MetaDataSize)
//            {
//                //Read the DevFourCC and advance the pointer offset.
//                PVRTuint32 DevFourCC=*(PVRTuint32*)((PVRTuint8*)pvrFileData+PVRTEX3_HEADERSIZE+metaDataSize);
//                metaDataSize+=sizeof(DevFourCC);
//                
//                //Read the Key and advance the pointer offset.
//                PVRTuint32 u32Key=*(PVRTuint32*)((PVRTuint8*)pvrFileData+PVRTEX3_HEADERSIZE+metaDataSize);
//                metaDataSize+=sizeof(u32Key);
//                
//                //Read the DataSize and advance the pointer offset.
//                PVRTuint32 u32DataSize = *(PVRTuint32*)((PVRTuint8*)pvrFileData+PVRTEX3_HEADERSIZE+metaDataSize);
//                metaDataSize+=sizeof(u32DataSize);
//                
//                //Get the current meta data.
//                MetaDataBlock& currentMetaData = (*pMetaData)[DevFourCC][u32Key];
//                
//                //Assign the values to the meta data.
//                currentMetaData.DevFOURCC=DevFourCC;
//                currentMetaData.u32Key=u32Key;
//                currentMetaData.u32DataSize=u32DataSize;
//                
//                //Check for data, if there is any, read it into the meta data.
//                if(u32DataSize > 0)
//                {
//                    //Allocate memory.
//                    currentMetaData.Data = new PVRTuint8[u32DataSize];
//                    
//                    //Copy the data.
//                    memcpy(currentMetaData.Data, ((PVRTuint8*)pvrFileData+PVRTEX3_HEADERSIZE+metaDataSize), u32DataSize);
//                    
//                    //Advance the meta data size.
//                    metaDataSize+=u32DataSize;
//                }
//            }
//        }
    }
    
    //Get the OGLES format values.
    PixelFormatDescriptor formatDescriptor = GetTextureFormat(sTextureHeader);
    
    //Check supported texture formats.
	bool bIsPVRTCSupported = IsGLExtensionSupported("GL_IMG_texture_compression_pvrtc");
#if !defined (__DAVAENGINE_IPHONE__)
	bool bIsBGRA8888Supported  = IsGLExtensionSupported("GL_IMG_texture_format_BGRA8888");
	bool bIsETCSupported = IsGLExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture");
#else //#if !defined (__DAVAENGINE_IPHONE__)
	bool bIsBGRA8888Supported  = IsGLExtensionSupported("GL_APPLE_texture_format_BGRA8888");
#endif //#if !defined (__DAVAENGINE_IPHONE__)
	bool bIsFloat16Supported = IsGLExtensionSupported("GL_OES_texture_half_float");
	bool bIsFloat32Supported = IsGLExtensionSupported("GL_OES_texture_float");
    
    //Check for compressed formats
    if (0 == formatDescriptor.format && 0 == formatDescriptor.type && 0 != formatDescriptor.internalformat)
    {
        if (formatDescriptor.internalformat>=GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG && formatDescriptor.internalformat<=GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
        {
            //Check for PVRTCI support.
            if(bIsPVRTCSupported)
            {
                bIsCompressedFormatSupported = bIsCompressedFormat = true;
            }
            else
            {
                //Output a warning.
                Logger::Warning("PVRTTextureLoadFromPointer warning: PVRTC not supported. Converting to RGBA8888 instead.\n");
                
                //Modify boolean values.
                bIsCompressedFormatSupported = false;
                bIsCompressedFormat = true;
                
                //Check if it's 2bpp.
                bool bIs2bppPVRTC = (formatDescriptor.internalformat==GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG || formatDescriptor.internalformat==GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);
                
                //Change texture format.
                formatDescriptor.format = formatDescriptor.internalformat = GL_RGBA;
                formatDescriptor.type = GL_UNSIGNED_BYTE;
                
                //Create a near-identical texture header for the decompressed header.
                sTextureHeaderDecomp = sTextureHeader;
                sTextureHeaderDecomp.u32ChannelType=ePVRTVarTypeUnsignedByteNorm;
                sTextureHeaderDecomp.u32ColourSpace=ePVRTCSpacelRGB;
                sTextureHeaderDecomp.u64PixelFormat=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
                
                //Allocate enough memory for the decompressed data. OGLES2, so only decompress one surface/face.
                pDecompressedData = malloc(GetTextureDataSize(sTextureHeaderDecomp, PVRTEX_ALLMIPLEVELS, false, true) );
                
                //Check the malloc.
                if (!pDecompressedData)
                {
                    Logger::Error("PVRTTextureLoadFromPointer error: Unable to allocate memory to decompress texture.\n");
                    return false;
                }
                
                //Get the dimensions for the current MIP level.
                uint32 uiMIPWidth = sTextureHeaderDecomp.u32Width;
                uint32 uiMIPHeight = sTextureHeaderDecomp.u32Height;
                
                //Setup temporary variables.
                uint8* pTempDecompData = (uint8*)pDecompressedData;
                uint8* pTempCompData = (uint8*)pTextureData;
                
                if (bIsLegacyPVR)
                {
                    //Decompress all the MIP levels.
                    for (uint32 uiFace=0;uiFace<sTextureHeader.u32NumFaces;++uiFace)
                    {
                        for (uint32 uiMIPMap=0;uiMIPMap<sTextureHeader.u32MIPMapCount;++uiMIPMap)
                        {
                            //Get the face offset. Varies per MIP level.
                            uint32 decompressedFaceOffset = GetTextureDataSize(sTextureHeaderDecomp, uiMIPMap, false, false);
                            uint32 compressedFaceOffset = GetTextureDataSize(sTextureHeader, uiMIPMap, false, false);

#if defined (__DAVAENGINE_IPHONE__)
                            DVASSERT(false && "Can be hardware supported");
#else //#if defined (__DAVAENGINE_IPHONE__)
                            //Decompress the texture data.
                            PVRTDecompressPVRTC(pTempCompData,bIs2bppPVRTC?1:0,uiMIPWidth,uiMIPHeight,pTempDecompData);
#endif //#if defined (__DAVAENGINE_IPHONE__)
                            
                            //Move forward through the pointers.
                            pTempDecompData+=decompressedFaceOffset;
                            pTempCompData+=compressedFaceOffset;
                            
                            //Work out the current MIP dimensions.
                            uiMIPWidth=PVRT_MAX(1,uiMIPWidth>>1);
                            uiMIPHeight=PVRT_MAX(1,uiMIPHeight>>1);
                        }
                        
                        //Reset the dims.
                        uiMIPWidth=sTextureHeader.u32Width;
                        uiMIPHeight=sTextureHeader.u32Height;
                    }
                }
                else
                {
                    //Decompress all the MIP levels.
                    for (uint32 uiMIPMap=0;uiMIPMap<sTextureHeader.u32MIPMapCount;++uiMIPMap)
                    {
                        //Get the face offset. Varies per MIP level.
                        uint32 decompressedFaceOffset = GetTextureDataSize(sTextureHeaderDecomp, uiMIPMap, false, false);
                        uint32 compressedFaceOffset = GetTextureDataSize(sTextureHeader, uiMIPMap, false, false);
                        
                        for (uint32 uiFace=0;uiFace<sTextureHeader.u32NumFaces;++uiFace)
                        {
#if defined (__DAVAENGINE_IPHONE__)
                            DVASSERT(false && "Can be hardware supported");
#else //#if defined (__DAVAENGINE_IPHONE__)
                            //Decompress the texture data.
                            PVRTDecompressPVRTC(pTempCompData,bIs2bppPVRTC?1:0,uiMIPWidth,uiMIPHeight,pTempDecompData);
#endif //#if defined (__DAVAENGINE_IPHONE__)
                            
                            //Move forward through the pointers.
                            pTempDecompData+=decompressedFaceOffset;
                            pTempCompData+=compressedFaceOffset;
                        }
                        
                        //Work out the current MIP dimensions.
                        uiMIPWidth=PVRT_MAX(1,uiMIPWidth>>1);
                        uiMIPHeight=PVRT_MAX(1,uiMIPHeight>>1);
                    }
                }
            }
        }
#if !defined(__DAVAENGINE_IPHONE__)
        else if (formatDescriptor.internalformat==GL_ETC1_RGB8_OES)
        {
            if(bIsETCSupported)
            {
                bIsCompressedFormatSupported = bIsCompressedFormat = true;
            }
            else
            {
                //Output a warning.
                Logger::Warning("PVRTTextureLoadFromPointer warning: ETC not supported. Converting to RGBA8888 instead.\n");
                
                //Modify boolean values.
                bIsCompressedFormatSupported = false;
                bIsCompressedFormat = true;
                
                //Change texture format.
                formatDescriptor.format = formatDescriptor.internalformat = GL_RGBA;
                formatDescriptor.type = GL_UNSIGNED_BYTE;
                
                //Create a near-identical texture header for the decompressed header.
                sTextureHeaderDecomp = sTextureHeader;
                sTextureHeaderDecomp.u32ChannelType=ePVRTVarTypeUnsignedByteNorm;
                sTextureHeaderDecomp.u32ColourSpace=ePVRTCSpacelRGB;
                sTextureHeaderDecomp.u64PixelFormat=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);

                //Allocate enough memory for the decompressed data. OGLES1, so only decompress one surface/face.
                pDecompressedData = malloc(GetTextureDataSize(sTextureHeaderDecomp, PVRTEX_ALLMIPLEVELS, false, true) );
                
                //Check the malloc.
                if (!pDecompressedData)
                {
                    Logger::Error("PVRTTextureLoadFromPointer error: Unable to allocate memory to decompress texture.\n");
                    return false;
                }
                
                //Get the dimensions for the current MIP level.
                uint32 uiMIPWidth = sTextureHeaderDecomp.u32Width;
                uint32 uiMIPHeight = sTextureHeaderDecomp.u32Height;
                
                //Setup temporary variables.
                uint8* pTempDecompData = (uint8*)pDecompressedData;
                uint8* pTempCompData = (uint8*)pTextureData;
                
                if (bIsLegacyPVR)
                {
                    //Decompress all the MIP levels.
                    for (uint32 uiFace=0;uiFace<sTextureHeader.u32NumFaces;++uiFace)
                    {
                        for (uint32 uiMIPMap=0;uiMIPMap<sTextureHeader.u32MIPMapCount;++uiMIPMap)
                        {
                            //Get the face offset. Varies per MIP level.
                            uint32 decompressedFaceOffset = GetTextureDataSize(sTextureHeaderDecomp, uiMIPMap, false, false);
                            uint32 compressedFaceOffset = GetTextureDataSize(sTextureHeader, uiMIPMap, false, false);

                            //Decompress the texture data.
                            PVRTDecompressETC(pTempCompData,uiMIPWidth,uiMIPHeight,pTempDecompData,0);
                            
                            //Move forward through the pointers.
                            pTempDecompData+=decompressedFaceOffset;
                            pTempCompData+=compressedFaceOffset;
                            
                            //Work out the current MIP dimensions.
                            uiMIPWidth=PVRT_MAX(1,uiMIPWidth>>1);
                            uiMIPHeight=PVRT_MAX(1,uiMIPHeight>>1);
                        }
                        
                        //Reset the dims.
                        uiMIPWidth=sTextureHeader.u32Width;
                        uiMIPHeight=sTextureHeader.u32Height;
                    }
                }
                else
                {
                    //Decompress all the MIP levels.
                    for (uint32 uiMIPMap=0;uiMIPMap<sTextureHeader.u32MIPMapCount;++uiMIPMap)
                    {
                        //Get the face offset. Varies per MIP level.
                        uint32 decompressedFaceOffset = GetTextureDataSize(sTextureHeaderDecomp, uiMIPMap, false, false);
                        uint32 compressedFaceOffset = GetTextureDataSize(sTextureHeader, uiMIPMap, false, false);

                        for (uint32 uiFace=0;uiFace<sTextureHeader.u32NumFaces;++uiFace)
                        {
                            //Decompress the texture data.
                            PVRTDecompressETC(pTempCompData,uiMIPWidth,uiMIPHeight,pTempDecompData,0);
                            
                            //Move forward through the pointers.
                            pTempDecompData+=decompressedFaceOffset;
                            pTempCompData+=compressedFaceOffset;
                        }
                        
                        //Work out the current MIP dimensions.
                        uiMIPWidth=PVRT_MAX(1,uiMIPWidth>>1);
                        uiMIPHeight=PVRT_MAX(1,uiMIPHeight>>1);
                    }
                }
            }
        }
#endif //#if !defined(__DAVAENGINE_IPHONE__)
    }
    
    //Check for BGRA support.
    if(formatDescriptor.format==GL_BGRA)
    {
#if defined (__DAVAENGINE_IPHONE__)
        formatDescriptor.internalformat = GL_RGBA;
#endif //#if defined (__DAVAENGINE_IPHONE__)
        
		if(!bIsBGRA8888Supported)
		{
#if defined (__DAVAENGINE_IPHONE__)
			Logger::Error("PVRTTextureLoadFromPointer failed: Unable to load GL_BGRA texture as extension GL_APPLE_texture_format_BGRA8888 is unsupported.\n");
#else //#if defined (__DAVAENGINE_IPHONE__)
			Logger::Error("PVRTTextureLoadFromPointer failed: Unable to load GL_BGRA texture as extension GL_IMG_texture_format_BGRA8888 is unsupported.\n");
#endif //#if defined (__DAVAENGINE_IPHONE__)
            return false;
        }
    }
    
    //Check for floating point textures
    if (formatDescriptor.type==GL_HALF_FLOAT)
    {
        if(!bIsFloat16Supported)
        {
            Logger::Error("PVRTTextureLoadFromPointer failed: Unable to load GL_HALF_FLOAT_OES texture as extension GL_OES_texture_half_float is unsupported.\n");
        }
    }
    if (formatDescriptor.type==GL_FLOAT)
    {
        if(!bIsFloat32Supported)
        {
            Logger::Error("PVRTTextureLoadFromPointer failed: Unable to load GL_FLOAT texture as extension GL_OES_texture_float is unsupported.\n");
        }
    }
    
    //Deal with unsupported texture formats
    if (formatDescriptor.internalformat==0)
    {
        Logger::Error("PVRTTextureLoadFromPointer failed: pixel type not supported.\n");
        return false;
    }


    texture->width = sTextureHeader.u32Width;
    texture->height = sTextureHeader.u32Height;
    texture->format = formatDescriptor.formatID;

    
#if defined (__DAVAENGINE_OPENGL__)
    
    //PVR files are never row aligned.
    RENDER_VERIFY(glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ));

    //Generate a texture
    RENDER_VERIFY(glGenTextures(1, &texture->id));
    
    //Initialise a texture target.
    GLint eTarget=GL_TEXTURE_2D;
    
    if(sTextureHeader.u32NumFaces>1)
    {
        eTarget=GL_TEXTURE_CUBE_MAP;
    }
    
    //Check if this is a texture array.
    if(sTextureHeader.u32NumSurfaces>1)
    {
        //Not supported in OpenGLES 2.0
        Logger::Error("PVRTTextureLoadFromPointer failed: Texture arrays are not available in OGLES2.0.\n");
        return false;
    }
    
    //Bind the 2D texture
    int32 savedTexture = GetSavedTextureID();
    BindTexture(texture->id);
    
    //Initialise the current MIP size.
    uint32 uiCurrentMIPSize=0;
    
    //Loop through the faces
    //Check if this is a cube map.
    if(sTextureHeader.u32NumFaces>1)
    {
        eTarget=GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        Logger::Warning("PVRTTextureLoadFromPointer warning: GL_TEXTURE_CUBE_MAP_POSITIVE_X");
    }
    
    //Initialise the width/height
    uint32 u32MIPWidth = sTextureHeader.u32Width;
    uint32 u32MIPHeight = sTextureHeader.u32Height;
    
    //Temporary data to save on if statements within the load loops.
    uint8* pTempData=NULL;
    PVRHeaderV3 *psTempHeader=NULL;
    if (bIsCompressedFormat && !bIsCompressedFormatSupported)
    {
        pTempData=(uint8*)pDecompressedData;
        psTempHeader=&sTextureHeaderDecomp;
    }
    else
    {
        pTempData=pTextureData;
        psTempHeader=&sTextureHeader;
    }
    
    //Loop through all MIP levels.
    if (bIsLegacyPVR)
    {
        //Temporary texture target.
        GLint eTextureTarget=eTarget;
        
        //Loop through all the faces.
        for (uint32 uiFace=0; uiFace<psTempHeader->u32NumFaces; ++uiFace)
        {
            //Loop through all the mip levels.
            for (uint32 uiMIPLevel=0; uiMIPLevel<psTempHeader->u32MIPMapCount; ++uiMIPLevel)
            {
                //Get the current MIP size.
                uiCurrentMIPSize=GetTextureDataSize(*psTempHeader,uiMIPLevel,false,false);
                
                //Upload the texture
                if (bIsCompressedFormat && bIsCompressedFormatSupported)
                {
                    RENDER_VERIFY(glCompressedTexImage2D(eTextureTarget,uiMIPLevel,formatDescriptor.internalformat,u32MIPWidth, u32MIPHeight, 0, uiCurrentMIPSize, pTempData));
                }
                else
                {
                    texture->TexImage(uiMIPLevel, u32MIPWidth, u32MIPHeight, pTempData);
//                    glTexImage2D(eTextureTarget,uiMIPLevel,eTextureInternalFormat, u32MIPWidth, u32MIPHeight, 0, eTextureFormat, eTextureType, pTempData);
                }
                pTempData+=uiCurrentMIPSize;
                
                //Reduce the MIP Size.
                u32MIPWidth=PVRT_MAX(1,u32MIPWidth>>1);
                u32MIPHeight=PVRT_MAX(1,u32MIPHeight>>1);
            }
            
            //Increase the texture target.
            eTextureTarget++;
            
            //Reset the current MIP dimensions.
            u32MIPWidth=psTempHeader->u32Width;
            u32MIPHeight=psTempHeader->u32Height;
        }
    }
    else
    {
        for (uint32 uiMIPLevel=0; uiMIPLevel<psTempHeader->u32MIPMapCount; ++uiMIPLevel)
        {
            //Get the current MIP size.
            uiCurrentMIPSize=GetTextureDataSize(*psTempHeader,uiMIPLevel,false,false);

            GLint eTextureTarget=eTarget;
            for (uint32 uiFace=0; uiFace<psTempHeader->u32NumFaces; ++uiFace)
            {
                //Upload the texture
                if (bIsCompressedFormat && bIsCompressedFormatSupported)
                {
                    RENDER_VERIFY(glCompressedTexImage2D(eTextureTarget,uiMIPLevel,formatDescriptor.internalformat,u32MIPWidth, u32MIPHeight, 0, uiCurrentMIPSize, pTempData));
                }
                else
                {
                    texture->TexImage(uiMIPLevel, u32MIPWidth, u32MIPHeight, pTempData);
//                    glTexImage2D(eTextureTarget,uiMIPLevel,eTextureInternalFormat, u32MIPWidth, u32MIPHeight, 0, eTextureFormat, eTextureType, pTempData);
                }
                pTempData+=uiCurrentMIPSize;
                eTextureTarget++;
            }
            
            //Reduce the MIP Size.
            u32MIPWidth=PVRT_MAX(1,u32MIPWidth>>1);
            u32MIPHeight=PVRT_MAX(1,u32MIPHeight>>1);
        }
    }
    
    FREE(pDecompressedData);
    
//    if (eTarget!=GL_TEXTURE_2D)
//    {
//        eTarget=GL_TEXTURE_CUBE_MAP;
//    }
    
//    //Set Minification and Magnification filters according to whether MIP maps are present.
//    if(formatDescriptor.type==GL_FLOAT || formatDescriptor.type==GL_HALF_FLOAT)
//    {
//        if(sTextureHeader.u32MIPMapCount==1)
//        {	// Texture filter modes are limited to these for float textures
//            glTexParameteri(eTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//            glTexParameteri(eTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        }
//        else
//        {
//            glTexParameteri(eTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
//            glTexParameteri(eTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        }
//    }
//    else
//    {
//        if(sTextureHeader.u32MIPMapCount==1)
//        {
//            glTexParameteri(eTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            glTexParameteri(eTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        }
//        else
//        {
//            glTexParameteri(eTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//            glTexParameteri(eTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        }
//    }
    
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    GLint wrapMode = GL_CLAMP_TO_EDGE;
#else //Non ES platforms
    GLint wrapMode = GL_CLAMP;
#endif //PLATFORMS

    RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
    RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
	
    BindTexture(savedTexture);
#endif //#if defined (__DAVAENGINE_OPENGL__)
    
    return true;
}


bool LibPVRHelper::PreparePVRData(const char* pvrData, const int32 pvrDataSize)
{
    PVRHeaderV3 textureHeader;
    //Header size.
    uint32 u32HeaderSize=0;
    
    //Boolean whether to byte swap the texture data or not.
    bool bSwapDataEndianness=false;
    
    //The channel type for endian swapping.
    EPVRTVariableType u32CurrentChannelType=ePVRTVarTypeUnsignedByte;
    
    //Check the first word of the file and see if it's equal to the current identifier (or reverse identifier)
    if(*(uint32*)pvrData!=PVRTEX_CURR_IDENT && *(uint32*)pvrData!=PVRTEX_CURR_IDENT_REV)
    {
        //Swap the header bytes if necessary.
        if(!PVRTIsLittleEndian())
        {
            bSwapDataEndianness=true;
            uint32 u32HeaderSize=PVRTByteSwap32(*(uint32*)pvrData);
            
            for (uint32 i=0; i<u32HeaderSize; ++i)
            {
                PVRTByteSwap( (uint8*)( ( (uint32*)pvrData )+i),sizeof(uint32) );
            }
        }
        
        //Get a pointer to the header.
        PVRHeaderV2* sLegacyTextureHeader=(PVRHeaderV2*)pvrData;
        
        //Set the header size.
        u32HeaderSize=sLegacyTextureHeader->dwHeaderSize;
        
        //We only really need the channel type.
        uint64 tempFormat;
        EPVRTColourSpace tempColourSpace;
        bool tempIsPreMult;
        
        //Map the enum to get the channel type.
        MapLegacyTextureEnumToNewFormat( (PVRTPixelType)( sLegacyTextureHeader->dwpfFlags&0xff),tempFormat,tempColourSpace, u32CurrentChannelType, tempIsPreMult);
    }
    else
    {
        // If the header file has a reverse identifier, then we need to swap endianness
        if(*(uint32*)pvrData==PVRTEX_CURR_IDENT_REV)
        {
            bSwapDataEndianness=true;
            PVRHeaderV3* pTextureHeader=(PVRHeaderV3*)pvrData;
            
            pTextureHeader->u32ChannelType=PVRTByteSwap32(pTextureHeader->u32ChannelType);
            pTextureHeader->u32ColourSpace=PVRTByteSwap32(pTextureHeader->u32ColourSpace);
            pTextureHeader->u32Depth=PVRTByteSwap32(pTextureHeader->u32Depth);
            pTextureHeader->u32Flags=PVRTByteSwap32(pTextureHeader->u32Flags);
            pTextureHeader->u32Height=PVRTByteSwap32(pTextureHeader->u32Height);
            pTextureHeader->u32MetaDataSize=PVRTByteSwap32(pTextureHeader->u32MetaDataSize);
            pTextureHeader->u32MIPMapCount=PVRTByteSwap32(pTextureHeader->u32MIPMapCount);
            pTextureHeader->u32NumFaces=PVRTByteSwap32(pTextureHeader->u32NumFaces);
            pTextureHeader->u32NumSurfaces=PVRTByteSwap32(pTextureHeader->u32NumSurfaces);
            pTextureHeader->u32Version=PVRTByteSwap32(pTextureHeader->u32Version);
            pTextureHeader->u32Width=PVRTByteSwap32(pTextureHeader->u32Width);
            PVRTByteSwap((uint8*)&pTextureHeader->u64PixelFormat,sizeof(uint64));
            
            //Channel type.
            u32CurrentChannelType=(EPVRTVariableType)pTextureHeader->u32ChannelType;
        }
        
        //Setup the texture header
        textureHeader = *(PVRHeaderV3*)pvrData;
        //Header size.
        u32HeaderSize=PVRTEX3_HEADERSIZE+textureHeader.u32MetaDataSize;
    }

    // Convert the data if needed
    if(bSwapDataEndianness)
    {
        //Get the size of the variables types.
        uint32 ui32VariableSize=0;
        switch(u32CurrentChannelType)
        {
            case ePVRTVarTypeFloat:
            case ePVRTVarTypeUnsignedInteger:
            case ePVRTVarTypeUnsignedIntegerNorm:
            case ePVRTVarTypeSignedInteger:
            case ePVRTVarTypeSignedIntegerNorm:
            {
                ui32VariableSize=4;
                break;
            }
            case ePVRTVarTypeUnsignedShort:
            case ePVRTVarTypeUnsignedShortNorm:
            case ePVRTVarTypeSignedShort:
            case ePVRTVarTypeSignedShortNorm:
            {
                ui32VariableSize=2;
                break;
            }
            case ePVRTVarTypeUnsignedByte:
            case ePVRTVarTypeUnsignedByteNorm:
            case ePVRTVarTypeSignedByte:
            case ePVRTVarTypeSignedByteNorm:
            {
                ui32VariableSize=1;
                break;
            }
            default:
                break;
        }
        
        //If the size of the variable type is greater than 1, then we need to byte swap.
        if (ui32VariableSize>1)
        {
            //Get the texture data.
            uint8* pu8OrigData = ( (uint8*)pvrData + u32HeaderSize);

            //Get the size of the texture data.
            uint32 ui32TextureDataSize = GetTextureDataSize(textureHeader);

            //Loop through and byte swap all the data. It's swapped in place so no need to do anything special.
            for(uint32 i = 0; i < ui32TextureDataSize; i+=ui32VariableSize)
            {
                PVRTByteSwap(pu8OrigData+i,ui32VariableSize);
            }
        }
    }
    
    return true;
}

    
	
};

