#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

#ensuredefined USE_FRAMEBUFFER_FETCH 0
#ensuredefined DECORATION 0
#ensuredefined BLEND_LAYER 0

#define DECAL_VT_GENERIC 0 //rgb height, nx nyn roughness blendfactor
#define DECAL_VT_NORMAL 1 //nx, ny, roughness (?), blendfactor 

#ifndef DECAL_TYPE
#define DECAL_TYPE DECAL_VT_GENERIC
#endif

color_mask = rgba;
color_mask1 = rgba;

#if DECORATION
blending
{
    src = zero dst = inv_src_alpha
}
#elif BLEND_LAYER
blending
{
    src = one dst = inv_src_alpha
}
#endif

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
    #if BLEND_LAYER
    float4 height : SV_TARGET2;
    #endif
#endif
};

uniform sampler2D albedoDecal;
uniform sampler2D normalDecal;

#if USE_FRAMEBUFFER_FETCH
//GFX_COMPLETE - FETCH FLOW NOT SUPPORTED YET
#endif

#if !BLEND_LAYER
uniform sampler2D dynamicTextureSrc0;
uniform sampler2D dynamicTextureSrc1;
#endif

[material][instance] property float2 decalHeightRange = float2(-0.2, 0.2);
[auto][a] property float tessellationHeight;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    float2 uvDecal = input.uv.xy;
    float2 uvPage = input.uv.zw;
    
#if DECORATION
    float4 decalNormalSample = tex2D(normalDecal, uvDecal);
    float alphaValue = decalNormalSample.w * input.value;
    output.decorationmask = float4(0, 0, 0, alphaValue);
#else
    float4 decalAlbedoSample = tex2D(albedoDecal, uvDecal);
    float4 decalNormalSample = tex2D(normalDecal, uvDecal);
    
        
    #if DECAL_TYPE == DECAL_VT_GENERIC
    float alphaValue = decalNormalSample.w * input.value;
    float heghtValue = (decalHeightRange.x + (decalHeightRange.y - decalHeightRange.x) * decalAlbedoSample.w) / tessellationHeight + 0.5;
    float2 decalSpaceNormal = decalNormalSample.xy * 2.0 - 1.0;
    float2 decalRotatedNormal = (input.basis.xy * decalSpaceNormal.x + input.basis.zw * decalSpaceNormal.y) * 0.5 + 0.5;
    float roughnessValue = decalNormalSample.z;        
    #endif
    
    #if BLEND_LAYER
    output.albedo = float4(decalAlbedoSample.xyz * alphaValue, alphaValue);
    output.normal = float4(decalRotatedNormal * alphaValue, roughnessValue * alphaValue, alphaValue);
    output.height = float4(heghtValue * alphaValue, 0.0, 0.0, alphaValue);
    #else
    float4 srcAlbedoSample = tex2D(dynamicTextureSrc0, uvPage);
    float4 srcNormalSample = tex2D(dynamicTextureSrc1, uvPage);
    output.albedo = lerp(srcAlbedoSample, float4(decalAlbedoSample.xyz, heghtValue), alphaValue);
    output.normal = lerp(srcNormalSample, float4(decalRotatedNormal, roughnessValue, 1.0), alphaValue);
    #endif
#endif

    return output;
}
