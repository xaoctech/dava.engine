/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The beast texture function definitions
 */ 
#ifndef BEASTTEXTURE_H
#define BEASTTEXTURE_H

#include "beastapitypes.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Gamma for input pixels data
 */
typedef enum {
    /**
	 * Gamma ramp, will always be combined with a gamma value
	 */
    ILB_IG_GAMMA,
    //ILB_TG_SRGB,
} ILBImageGammaType;

/**
 * Format for pixel data
 */
typedef enum {
    /**
	 * Monochrome floating point pixels
	 */
    ILB_PF_MONO_FLOAT,
    /**
	 * Color floating point pixels
	 */
    ILB_PF_RGB_FLOAT,
    /**
	 * Color with alpha floating point pixels
	 */
    ILB_PF_RGBA_FLOAT,
    /**
	 * Monochrome byte pixels
	 */
    ILB_PF_MONO_BYTE,
    /**
	 * Color byte pixels
	 */
    ILB_PF_RGB_BYTE,
    /**
	 * Color with alpha byte pixels
	 */
    ILB_PF_RGBA_BYTE,
} ILBPixelFormat;

/** 
 * References in an external texture.\n
 * Note, a referenced texture doesn't need a call to ILBEndTexture!
 * @param beastManager the beast manager this texture will be associated with
 * @param uniqueName A unique name for the texture.
 * @param filename the path to the file to use
 * @param target a pointer to a ILBTextureHandle that will receive the new
 * created texture
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBReferenceTexture(ILBManagerHandle beastManager,
                                               ILBConstString uniqueName,
                                               ILBConstString filename,
                                               ILBTextureHandle* target);

/** 
 * Begins creation of a texture.\n
 * @param beastManager the beast manager this texture will be associated with
 * @param uniqueName a unique name for the texture. 
 * @param width the width of the texture
 * @param height the height of the texture
 * @param inputFormat the pixel format you intend to use to input data. This
 * will not necessarily be the same format as Beast choose to save the image
 * in.
 * @param target a pointer to a ILBTextureHandle that will receive the 
 * created texture
 * @return The result of the operation.
 * @bug The pixelformat ILB_PF_MONO_FLOAT is currently not supported.
 */
ILB_DLL_FUNCTION ILBStatus ILBBeginTexture(ILBManagerHandle beastManager,
                                           ILBConstString uniqueName,
                                           int32 width,
                                           int32 height,
                                           ILBPixelFormat inputFormat,
                                           ILBTextureHandle* target);

/** 
 * Finds a cached texture.\n
 * @param beastManager the beast manager to check whether the texture is available 
 * in
 * @param uniqueName the unique name for texture.
 * @param target the texture handle to store the texture in
 * @return The result of the operation. 
 * ILB_ST_SUCCESS if the texture is available
 * ILB_ST_UNKNOWN_OBJECT if the texture is not in the cache
 */
ILB_DLL_FUNCTION ILBStatus ILBFindTexture(ILBManagerHandle beastManager,
                                          ILBConstString uniqueName,
                                          ILBTextureHandle* target);

/** 
 * Adds pixels to a texture. This function should only be used on textures using
 * FLOAT pixel formats.
 * Pixel data is treated as a linear array of pixels
 * line by line. It may be called multiple times avoid having to
 * replicate the entire image in Beast format, but the total number of pixels
 * must not exceed what was specified in beginTexture.\n
 * The lines are given from the bottom and up,
 * the lines are stored left to right.
 * @param texture the texture to add the pixels for
 * @param data the pixel data to add. Note this must correspond to the pixel
 * format specified in the beginTexture call
 * @param pixelCount the number of pixel (not data values) specified 
 * in this batch. 
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddPixelDataHDR(ILBTextureHandle texture,
                                              const float* data,
                                              int32 pixelCount);

/** 
 * Adds pixels to a texture. This function should only be used on textures using
 * BYTE pixel formats.
 * Pixel data is treated as a linear array of pixels
 * line by line. It may be called multiple times avoid having to
 * replicate the entire image in Beast format, but the total number of pixels
 * must not exceed what was specified in beginTexture.\n
 * The lines are given from the bottom and up,
 * the lines are stored left to right.
 * @param texture the texture to add the pixels for
 * @param data the pixel data to add. Note this must correspond to the pixel
 * format specified in the beginTexture call
 * @param pixelCount the number of pixels (not data values) specified 
 * in this batch. 
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddPixelDataLDR(ILBTextureHandle texture,
                                              const unsigned char* data,
                                              int32 pixelCount);

/** 
 * Finalizes creation of a texture.
 * Will fail unless it has got the width * height pixels added
 * @param texture the texture to finalize
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBEndTexture(ILBTextureHandle texture);

/** 
 * Sets what gamma encoding the colors in LDR texture has.\n
 * This call is only valid on textures with an LDR pixel format.
 * HDR textures are always regarded as linear.
 * By default it's set to ILB_IG_GAMMA with gamma 2.2
 * @param texture the texture to set gamma on
 * @param type the gamma ramp type
 * @param gamma the gamma value. 
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetInputGamma(ILBTextureHandle texture, ILBImageGammaType type, float gamma);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTTEXTURE_H
