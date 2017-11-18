/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * A set of functions to create textures
 */ 
#ifndef TEXTURES_H
#define TEXTURES_H
#include <beastapi/beasttexture.h>
#include <beastapi/beastframebuffer.h>
#include <beastapi/beasttarget.h>
#include "utils.h"
#include <complex>
#include <cmath>
namespace bex
{
/**
	 * Creates a simple procedural texture using xor of u and v
	 * and a checker board in alpha
	 */
inline ILBTextureHandle createXorTexture(ILBManagerHandle bm, const std::basic_string<TCHAR>& name, const ColorRGB& baseColor)
{
    ILBTextureHandle tex;
    const int width = 256;
    const int height = 256;
    const int checkerSize = 31;
    apiCall(ILBBeginTexture(bm,
                            name.c_str(),
                            width,
                            height,
                            ILB_PF_RGBA_BYTE,
                            &tex));

    const int components = 4;
    apiCall(ILBSetInputGamma(tex, ILB_IG_GAMMA, 2.2f));
    std::vector<unsigned char> lineData(width * components);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            unsigned char valLdr = ((x ^ y) & 0xff);
            float val = static_cast<float>(valLdr) / 255.0f;
            bool checker = ((y & checkerSize) > checkerSize / 2) ^ ((x & checkerSize) > checkerSize / 2);
            lineData[x * components] = static_cast<unsigned char>(val * 255.0f * baseColor.r);
            lineData[x * components + 1] = static_cast<unsigned char>(val * 255.0f * baseColor.g);
            lineData[x * components + 2] = static_cast<unsigned char>(val * 255.0f * baseColor.b);
            lineData[x * components + 3] = 255 * checker;
        }
        apiCall(ILBAddPixelDataLDR(tex, &lineData[0], width));
    }
    apiCall(ILBEndTexture(tex));
    return tex;
}

float mandelbrot(std::complex<float>& j0, int maxIter)
{
    std::complex<float> j(0.0f, 0.0f);
    int i;
    for (i = 0; i < maxIter; ++i)
    {
        j = j * j + j0;
        float absJ = abs(j);
        if (absJ > 2.0f)
        {
            // Interpolation for smoother/slower mandelbrot results!
            float res = (log(log(2.0f)) - log(log(absJ))) / log(static_cast<float>(2.0f));
            return (static_cast<float>(i) + res) / static_cast<float>(maxIter);
        }
    }
    return 1.0f;
}
/**
	 * Creates a mandelbrot procedural texture 
	 */
inline ILBTextureHandle createMandelbrotTexture(ILBManagerHandle bm, const std::basic_string<TCHAR>& name, const ColorRGB& baseColor, int width, int height)
{
    ILBTextureHandle tex;
    apiCall(ILBBeginTexture(bm,
                            name.c_str(),
                            width,
                            height,
                            ILB_PF_RGBA_FLOAT,
                            &tex));

    const int components = 4;
    const int iterations = 100;
    float minU = -2.0f;
    float maxU = 2.0f;
    float minV = -1.0f;
    float maxV = 1.0f;
    std::vector<float> lineData(width * components);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float localX = static_cast<float>(x) * (maxU - minU) / static_cast<float>(width) + minU;
            float localY = static_cast<float>(y) * (maxV - minV) / static_cast<float>(height) + minV;

            std::complex<float> c(localX, localY);
            float val = mandelbrot(c, iterations);

            lineData[x * components] = val * baseColor.r;
            lineData[x * components + 1] = val * baseColor.g;
            lineData[x * components + 2] = val * baseColor.b;
            lineData[x * components + 3] = 1.0f;
        }
        apiCall(ILBAddPixelDataHDR(tex, &lineData[0], width));
    }
    apiCall(ILBEndTexture(tex));
    return tex;
}

inline ILBTextureHandle createTestColorTexture(ILBManagerHandle bm, const std::basic_string<TCHAR>& name)
{
    ILBTextureHandle tex;
    const int width = 256;
    const int height = 256;

    apiCall(ILBBeginTexture(bm,
                            name.c_str(),
                            width,
                            height,
                            ILB_PF_RGBA_BYTE,
                            &tex));

    const int components = 4;
    apiCall(ILBSetInputGamma(tex, ILB_IG_GAMMA, 2.2f));
    std::vector<unsigned char> lineData(width * components);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            bool lr = x > (width / 2);
            bool ud = y > (height / 2);
            unsigned char r, g, b;
            r = g = b = 0;

            if (lr)
            {
                if (ud)
                {
                    r = 255;
                }
                else
                {
                    g = 255;
                }
            }
            else
            {
                if (ud)
                {
                    b = 255;
                }
                else
                {
                    r = g = 255;
                }
            }

            lineData[x * components] = r;
            lineData[x * components + 1] = g;
            lineData[x * components + 2] = b;
            lineData[x * components + 3] = 255;
        }
        apiCall(ILBAddPixelDataLDR(tex, &lineData[0], width));
    }
    apiCall(ILBEndTexture(tex));
    return tex;
}

inline ILBTextureHandle copyFrameBuffer(ILBManagerHandle bmh, ILBTargetHandle target, ILBRenderPassHandle pass, const std::basic_string<TCHAR>& textureName, bool hdr)
{
    const float gamma = 2.2f;

    // They call me THE COUNT because I love to COUNT things!
    int32 count;
    bex::apiCall(ILBGetFramebufferCount(target, &count));

    ILBFramebufferHandle frameBuffer;
    bex::apiCall(ILBGetFramebuffer(target, pass, 0, &frameBuffer));

    int32 width, height;
    bex::apiCall(ILBGetResolution(frameBuffer, &width, &height));

    ILBTextureHandle lightMap;

    if (hdr)
    {
        bex::apiCall(ILBBeginTexture(bmh, textureName.c_str(), width, height, ILB_PF_RGB_FLOAT, &lightMap));
        std::vector<float> texture(width * 3);
        for (int i = 0; i < height; i++)
        {
            bex::apiCall(ILBReadRegionHDR(frameBuffer, 0, height - i - 1, width, height - i, ILB_CS_RGB, &texture[0]));
            bex::apiCall(ILBAddPixelDataHDR(lightMap, &texture[0], width));
        }
    }
    else
    {
        bex::apiCall(ILBBeginTexture(bmh, textureName.c_str(), width, height, ILB_PF_RGB_BYTE, &lightMap));
        bex::apiCall(ILBSetInputGamma(lightMap, ILB_IG_GAMMA, gamma));
        std::vector<unsigned char> texture(width * 3);
        for (int i = 0; i < height; i++)
        {
            bex::apiCall(ILBReadRegionLDR(frameBuffer, 0, height - i - 1, width, height - i, ILB_CS_RGB, gamma, &texture[0]));
            bex::apiCall(ILBAddPixelDataLDR(lightMap, &texture[0], width));
        }
    }

    bex::apiCall(ILBEndTexture(lightMap));
    bex::apiCall(ILBDestroyFramebuffer(frameBuffer));
    return lightMap;
}

inline void copyVertexBuffer(ILBTargetHandle target, ILBRenderPassHandle pass, ILBTargetEntityHandle entity, std::vector<float>& colors)
{
    ILBFramebufferHandle vertexBuffer;
    bex::apiCall(ILBGetVertexbuffer(target, pass, entity, &vertexBuffer));
    int32 channelCount;
    bex::apiCall(ILBGetChannelCount(vertexBuffer, &channelCount));
    int32 width, height;
    bex::apiCall(ILBGetResolution(vertexBuffer, &width, &height));
    colors.resize(width * height * channelCount);
    bex::apiCall(ILBReadRegionHDR(vertexBuffer, 0, 0, width, 1, ILB_CS_ALL, &colors[0]));
    bex::apiCall(ILBDestroyFramebuffer(vertexBuffer));
}

} // namespace bex

#endif // TEXTURES_H
