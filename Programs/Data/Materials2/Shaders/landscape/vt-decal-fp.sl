#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

#ensuredefined USE_FRAMEBUFFER_FETCH 0
#ensuredefined DECORATION 0

#define DECAL_VT_GENERIC 0 //rgb height, nx nyn roughness blendfactor
#define DECAL_VT_NORMAL 1 //nx, ny, roughness (?), blendfactor 

#ifndef DECAL_TYPE
#define DECAL_TYPE DECAL_VT_GENERIC
#endif

color_mask = rgba;
color_mask1 = rgba;

fragment_in
{
    float4 uv : TEXCOORD0;
    float4 basis : TEXCOORD1; //(t, b);
    float value : TEXCOORD2;
};

fragment_out
{
#if DECORATION
    float4 decorationmask : SV_TARGET0;
#else
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;    
#endif
};

uniform sampler2D albedoDecal;
uniform sampler2D normalDecal;

#if USE_FRAMEBUFFER_FETCH
//GFX_COMPLETE - FETCH FLOW NOT SUPPORTED YET
#endif

uniform sampler2D dynamicTextureSrc0;
uniform sampler2D dynamicTextureSrc1;

[material][instance] property float2 decalHeightRange = float2(-0.2, 0.2);
[auto][a] property float tessellationHeight;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    float2 uvDecal = input.uv.xy;
    float2 uvPage = input.uv.zw;
    
#if DECORATION
    float4 decalNormalSample = tex2D(normalDecal, uvDecal);
    float4 srcDecorationSample = tex2D(dynamicTextureSrc0, uvPage);
    output.decorationmask = srcDecorationSample * (1.0 - decalNormalSample.w * input.value);
#else
    float4 decalAlbedoSample = tex2D(albedoDecal, uvDecal);
    float4 decalNormalSample = tex2D(normalDecal, uvDecal);

    float4 srcAlbedoSample = tex2D(dynamicTextureSrc0, uvPage);
    float4 srcNormalSample = tex2D(dynamicTextureSrc1, uvPage);    
        
    #if DECAL_TYPE == DECAL_VT_GENERIC
    //generic decals use regular blend (alpha instead of metalicity), as we cant compose height otherwise
    float alphaValue = decalNormalSample.w * input.value; //or here we should use only mask from texture?
    //apply hegiht ranges
    //decalAlbedoSample.w = 0.0;
    decalAlbedoSample.w = (decalHeightRange.x + (decalHeightRange.y - decalHeightRange.x) * decalAlbedoSample.w) / tessellationHeight + 0.5;
    //decalAlbedoSample.w = (decalHeightRange.x) / tessellationHeight + 0.5;
    output.albedo = lerp(srcAlbedoSample, decalAlbedoSample, alphaValue);
    //output.albedo = float4(1.0, 0.0, 0.0, alphaValue);
    //output.albedo.r = frac(uvDecal.x*10.0);
    float2 decalSpaceNormal = decalNormalSample.xy * 2.0 - 1.0;
    float2 decalRotatedNormal = (input.basis.xy * decalSpaceNormal.x + input.basis.zw * decalSpaceNormal.y) * 0.5 + 0.5;
    output.normal.xyz = lerp(srcNormalSample.xyz, float3(decalRotatedNormal, decalNormalSample.z), alphaValue);
    output.normal.w = 1.0; //srcNormalSample.w; //no microshadows in decals for now
    #endif
#endif

    return output;
}
