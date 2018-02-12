#include "include/common.h"
#include "include/shading-options.h"
#include "include/math.h"

#ensuredefined USE_PREVIOUS_LANDSCAPE_LAYER 0
#ensuredefined LANDSCAPE_LAYER_BLEND_HEIGHT_MODE 0
#ensuredefined BLEND_LANDSCAPE_HEIGHT 0

#if USE_PREVIOUS_LANDSCAPE_LAYER
uniform sampler2D dynamicTextureSrc0; // prev albedo + biased height, for decoration prev mask.
uniform sampler2D dynamicTextureSrc1; // prev normal, for decoration prev height.
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
#if LANDSCAPE_VT_PAGE || USE_PREVIOUS_LANDSCAPE_LAYER
    float2 pageTexCoord : TEXCOORD5;
#endif
};

fragment_out
{
#if DECORATION
    float4 decorationmask : SV_TARGET0;
    float4 height : SV_TARGET1;
#else
    float4 albedo : SV_TARGET0;
    float4 normalmap : SV_TARGET1;
#endif
};

uniform sampler2D tilemask;
uniform sampler2D colortexture;
uniform sampler2D albedoTile0;
uniform sampler2D albedoTile1;
uniform sampler2D albedoTile2;
uniform sampler2D albedoTile3;
uniform sampler2D normalmapTile0;
uniform sampler2D normalmapTile1;
uniform sampler2D normalmapTile2;
uniform sampler2D normalmapTile3;

[material][instance] property float4 tile0DecorationChannels = float4(1, 0, 0, 0);
[material][instance] property float4 tile1DecorationChannels = float4(0, 1, 0, 0);
[material][instance] property float4 tile2DecorationChannels = float4(0, 0, 1, 0);
[material][instance] property float4 tile3DecorationChannels = float4(0, 0, 0, 1);
[material][instance] property float4 materialsHeight = float4(0.2, 0.2, 0.2, 0.2);
[material][instance] property float tilemaskHeight = 0.2;

[auto][a] property float tessellationHeight;

#if BLEND_LANDSCAPE_HEIGHT
struct BlendHeightRes
{
    float3 blendColor;
    float4 normal;
};
#endif // BLEND_LANDSCAPE_HEIGHT

#if BLEND_LANDSCAPE_HEIGHT == 1
BlendHeightRes BlendLandscape(float3 color0, float4 normal0, float height0, float3 color1, float4 normal1, float height1, float delta)
{
    BlendHeightRes res;
    float med = max(height0, height1) - delta;

    float k0 = max(height0 - med, 0.0f);
    float k1 = max(height1 - med, 0.0f);
    float lrp = k1 / (k0 + k1);

    res.blendColor = lerp(color0, color1, lrp);
    res.normal = lerp(normal0, normal1, lrp);
    return res;
}
#elif BLEND_LANDSCAPE_HEIGHT == 2
BlendHeightRes BlendLandscape(float3 color0, float4 normal0, float height0, float3 color1, float4 normal1, float height1, float delta)
{
    BlendHeightRes res;
    float diff = height0 - height1;
    float lrp = 1.0f - saturate(abs(diff) / delta);
    float halfLrp = lrp * 0.5f;
    halfLrp = lerp(1.0f - halfLrp, halfLrp, step(0.0f, diff));

    res.blendColor = lerp(color0, color1, halfLrp);
    res.normal = lerp(normal0, normal1, halfLrp);
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
#if DECORATION

    float4 mixMask = normalize(step(maxHeight, maskHeight)) * tilemaskSample;

    float4 decorationMask =
    tile0DecorationChannels * mixMask.x +
    tile1DecorationChannels * mixMask.y +
    tile2DecorationChannels * mixMask.z +
    tile3DecorationChannels * mixMask.w;

#if USE_PREVIOUS_LANDSCAPE_LAYER
    float4 prevMask = tex2D(dynamicTextureSrc0, input.pageTexCoord);
    float prevHeight = FP_A8(tex2D(dynamicTextureSrc1, input.pageTexCoord));
    decorationMask = lerp(prevMask, decorationMask, step(prevHeight, maxHeight));
    maxHeight = max(maxHeight, prevHeight);
#endif // USE_PREVIOUS_LANDSCAPE_LAYER

    output.decorationmask = decorationMask;
    FP_A8(output.height) = maxHeight;
#else

    float4 colorSample = tex2D(colortexture, input.texCoord);

    float4 normalSample0 = tex2D(normalmapTile0, input.texCoordTiled0);
    float4 normalSample1 = tex2D(normalmapTile1, input.texCoordTiled1);
    float4 normalSample2 = tex2D(normalmapTile2, input.texCoordTiled2);
    float4 normalSample3 = tex2D(normalmapTile3, input.texCoordTiled3);

    float3 resultBaseColor = 0;
    float4 normalMixes = 0;

#if BLEND_LANDSCAPE_HEIGHT == 0
    float4 mixMask = normalize(step(maxHeight, maskHeight));

    float3 baseColorMixed =
    baseColorSample0.xyz * mixMask.x +
    baseColorSample1.xyz * mixMask.y +
    baseColorSample2.xyz * mixMask.z +
    baseColorSample3.xyz * mixMask.w;

    resultBaseColor = SoftLightBlend(baseColorMixed, colorSample.xyz);

    normalMixes =
    normalSample0 * mixMask.x +
    normalSample1 * mixMask.y +
    normalSample2 * mixMask.z +
    normalSample3 * mixMask.w;

#else // BLEND_LANDSCAPE_HEIGHT != 0
    float2 heightTmp = float2(maskHeight.x, maskHeight.y);

    BlendHeightRes bRes = BlendLandscape(baseColorSample0.xyz, normalSample0, heightTmp.x, baseColorSample1.xyz, normalSample1, heightTmp.y, heightDelta);
    heightTmp = float2(max(heightTmp.x, heightTmp.y), maskHeight.z);
    bRes = BlendLandscape(bRes.blendColor, bRes.normal, heightTmp.x, baseColorSample2.xyz, normalSample2, heightTmp.y, heightDelta);
    heightTmp = float2(max(heightTmp.x, heightTmp.y), maskHeight.w);
    bRes = BlendLandscape(bRes.blendColor, bRes.normal, heightTmp.x, baseColorSample3.xyz, normalSample3, heightTmp.y, heightDelta);
    resultBaseColor.xyz = bRes.blendColor.xyz;
    resultBaseColor.xyz = SoftLightBlend(resultBaseColor.xyz, colorSample.xyz);

    normalMixes = bRes.normal;
#endif // BLEND_LANDSCAPE_HEIGHT finish.

#if USE_PREVIOUS_LANDSCAPE_LAYER
    float4 prevLayerSample = tex2D(dynamicTextureSrc0, input.pageTexCoord);
    float outputHeight = maxHeight / tessellationHeight + 0.5;
    float4 prevLayerNormal = tex2D(dynamicTextureSrc1, input.pageTexCoord);
    #if BLEND_LANDSCAPE_HEIGHT == 0
    float stepVal = step(outputHeight, prevLayerSample.w);
    output.albedo = lerp(float4(resultBaseColor, outputHeight), prevLayerSample, stepVal);
    output.normalmap = lerp(normalMixes, prevLayerNormal, stepVal);
    #else // BLEND_LANDSCAPE_HEIGHT == 0
    float prevLayerRestoredHeight = tessellationHeight * (prevLayerSample.w - 0.5f); // To make in one space with blending delta.
    BlendHeightRes layersBlend = BlendLandscape(prevLayerSample.xyz, prevLayerNormal, prevLayerRestoredHeight, resultBaseColor, normalMixes, maxHeight, heightDelta);
    output.albedo = float4(layersBlend.blendColor.xyz, max(outputHeight, prevLayerSample.w));
    output.normalmap = layersBlend.normal;
    #endif //BLEND_LANDSCAPE_HEIGHT == 0

#else //USE_PREVIOUS_LANDSCAPE_LAYER
    output.albedo = float4(resultBaseColor, maxHeight / tessellationHeight + 0.5);
    output.normalmap = normalMixes;
#endif // USE_PREVIOUS_LANDSCAPE_LAYER

    #if LANDSCAPE_VT_PAGE
    {
        #define VT_PAGE_BORDER (1.4 / 50.0)
        float4 edge = float4(VT_PAGE_BORDER, VT_PAGE_BORDER, input.pageTexCoord.x, input.pageTexCoord.y);
        float4 value = float4(input.pageTexCoord.x, input.pageTexCoord.y, 1.0 - VT_PAGE_BORDER, 1.0 - VT_PAGE_BORDER);
        float4 stepresult = step(edge, value);
        float border = 1.0 - stepresult.x * stepresult.y * stepresult.z * stepresult.w;
        output.albedo = lerp(output.albedo, float4(1.0, 1.0, 1.0, 1.0), border);
    }
    #endif

#endif // DECORATION

    return output;
}
