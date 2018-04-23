#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"
#include "include/math.h"

#ensuredefined USE_PREVIOUS_LANDSCAPE_LAYER 0
#ensuredefined LANDSCAPE_LAYER_BLEND_HEIGHT_MODE 0
#ensuredefined BLEND_LANDSCAPE_HEIGHT 0

#if USE_PREVIOUS_LANDSCAPE_LAYER
uniform sampler2D dynamicTextureSrc0; // prev albedo + biased height, for decoration prev mask.
#endif

#if BLEND_LANDSCAPE_HEIGHT
[material][instance] property float heightDelta = 0.05f;
#endif

fragment_in
{
    float2 texCoord : TEXCOORD0;
    float2 texCoordTiled0 : TEXCOORD1;
    float2 texCoordTiled1 : TEXCOORD2;
    float2 texCoordTiled2 : TEXCOORD3;
    float2 texCoordTiled3 : TEXCOORD4;
#if USE_PREVIOUS_LANDSCAPE_LAYER
    float2 pageTexCoord : TEXCOORD5;
#endif
};

fragment_out
{
#if DECORATION
    float4 decorationmask : SV_TARGET0;
#else
    float4 albedo : SV_TARGET0;
#endif
};

uniform sampler2D tilemask;
uniform sampler2D colortexture;
uniform sampler2D albedoTile0;
uniform sampler2D albedoTile1;
uniform sampler2D albedoTile2;
uniform sampler2D albedoTile3;

[material][instance] property float4 tileDecorationIDs = float4(1.0, 2.0, 3.0, 4.0);
[material][instance] property float4 materialsHeight = float4(0.2, 0.2, 0.2, 0.2);
[material][instance] property float tilemaskHeight = 0.2;

[auto][a] property float tessellationHeight;

#if BLEND_LANDSCAPE_HEIGHT
struct BlendHeightRes
{
    float3 blendColor;
};
#endif // BLEND_LANDSCAPE_HEIGHT

#if BLEND_LANDSCAPE_HEIGHT == 1
BlendHeightRes BlendLandscape(float3 color0, float height0, float3 color1, float height1, float delta)
{
    BlendHeightRes res;
    float med = max(height0, height1) - delta;

    float k0 = max(height0 - med, 0.0f);
    float k1 = max(height1 - med, 0.0f);
    float lrp = k1 / (k0 + k1);

    res.blendColor = lerp(color0, color1, lrp);
    return res;
}
#elif BLEND_LANDSCAPE_HEIGHT == 2
BlendHeightRes BlendLandscape(float3 color0, float height0, float3 color1, float height1, float delta)
{
    BlendHeightRes res;
    float diff = height0 - height1;
    float lrp = 1.0f - saturate(abs(diff) / delta);
    float halfLrp = lrp * 0.5f;
    halfLrp = lerp(1.0f - halfLrp, halfLrp, step(0.0f, diff));

    res.blendColor = lerp(color0, color1, halfLrp);
    return res;
}
#endif // BLEND_LANDSCAPE_HEIGHT == 2

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float4 baseColorSample0 = tex2D(albedoTile0, input.texCoordTiled0);
    float4 baseColorSample1 = tex2D(albedoTile1, input.texCoordTiled1);
    float4 baseColorSample2 = tex2D(albedoTile2, input.texCoordTiled2);
    float4 baseColorSample3 = tex2D(albedoTile3, input.texCoordTiled3);

    float4 tilemaskSample = tex2D(tilemask, input.texCoord);
    float4 maskHeight = (tilemaskSample - 0.5) * tilemaskHeight;
    maskHeight += materialsHeight * (float4(baseColorSample0.w, baseColorSample1.w, baseColorSample2.w, baseColorSample3.w) - 0.5);

    float maxHeight = max(max(maskHeight.x, maskHeight.y), max(maskHeight.z, maskHeight.w));
    float4 indexMask = step(maxHeight, maskHeight);
    
#if DECORATION
    {
        float4 mixMask = normalize(indexMask) * tilemaskSample;
        float decorationIndex = dot(indexMask, tileDecorationIDs / 255.0);
        float4 decorationMask = float4(decorationIndex, dot(mixMask, indexMask), maxHeight, 0.0);
        
    #if USE_PREVIOUS_LANDSCAPE_LAYER

        float4 prevMask = tex2D(dynamicTextureSrc0, input.pageTexCoord);
        decorationMask.xy = lerp(prevMask.xy, decorationMask.xy, step(prevMask.z, decorationMask.z));
        decorationMask.z = max(prevMask.z, decorationMask.z);

    #endif // USE_PREVIOUS_LANDSCAPE_LAYER

        output.decorationmask = decorationMask;
    }
#else
    {
        float4 colorSample = tex2D(colortexture, input.texCoord);
        float3 resultBaseColor = 0;
        
    #if BLEND_LANDSCAPE_HEIGHT == 0
        float4 mixMask = normalize(indexMask);
        float3 baseColorMixed = baseColorSample0.xyz * mixMask.x + baseColorSample1.xyz * mixMask.y + baseColorSample2.xyz * mixMask.z + baseColorSample3.xyz * mixMask.w;
        resultBaseColor = SoftLightBlend(baseColorMixed, colorSample.xyz);
    #else // BLEND_LANDSCAPE_HEIGHT != 0
        float2 heightTmp = float2(maskHeight.x, maskHeight.y);
        BlendHeightRes bRes = BlendLandscape(baseColorSample0.xyz, heightTmp.x, baseColorSample1.xyz, heightTmp.y, heightDelta);
        heightTmp = float2(max(heightTmp.x, heightTmp.y), maskHeight.z);
        bRes = BlendLandscape(bRes.blendColor, heightTmp.x, baseColorSample2.xyz, heightTmp.y, heightDelta);
        heightTmp = float2(max(heightTmp.x, heightTmp.y), maskHeight.w);
        bRes = BlendLandscape(bRes.blendColor, heightTmp.x, baseColorSample3.xyz, heightTmp.y, heightDelta);
        resultBaseColor.xyz = bRes.blendColor.xyz;
        resultBaseColor.xyz = SoftLightBlend(resultBaseColor.xyz, colorSample.xyz);
    #endif // BLEND_LANDSCAPE_HEIGHT finish.
        
    #if USE_PREVIOUS_LANDSCAPE_LAYER
        float4 prevLayerSample = tex2D(dynamicTextureSrc0, input.pageTexCoord);
        float outputHeight = maxHeight / tessellationHeight + 0.5;
        #if BLEND_LANDSCAPE_HEIGHT == 0
        float stepVal = step(outputHeight, prevLayerSample.w);
        output.albedo = lerp(float4(resultBaseColor, outputHeight), prevLayerSample, stepVal);
        #else // BLEND_LANDSCAPE_HEIGHT == 0
        float prevLayerRestoredHeight = tessellationHeight * (prevLayerSample.w - 0.5f); // To make in one space with blending delta.
        BlendHeightRes layersBlend = BlendLandscape(prevLayerSample.xyz, prevLayerRestoredHeight, resultBaseColor, maxHeight, heightDelta);
        output.albedo = float4(layersBlend.blendColor.xyz, max(outputHeight, prevLayerSample.w));
        #endif //BLEND_LANDSCAPE_HEIGHT == 0
    #else //USE_PREVIOUS_LANDSCAPE_LAYER
        output.albedo = float4(resultBaseColor, maxHeight / tessellationHeight + 0.5);
    #endif // USE_PREVIOUS_LANDSCAPE_LAYER
    }
#endif // DECORATION

    return output;
}
