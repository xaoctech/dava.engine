#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

color_mask = rgba;
color_mask1 = rgba;

fragment_in
{
    float4 uv : TEXCOORD0;
};

fragment_out
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

uniform sampler2D blendedAlbedo;
uniform sampler2D blendedNormal;
uniform sampler2D blendedHeight;

uniform sampler2D dynamicTextureSrc0;
uniform sampler2D dynamicTextureSrc1;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float4 blendedAlbedoSample = tex2D(blendedAlbedo, input.uv);
    float3 blendedNormalSample = tex2D(blendedNormal, input.uv).xyz;
    float blendedHeightSample = tex2D(blendedHeight, input.uv).x;

    float4 srcAlbedoSample = tex2D(dynamicTextureSrc0, input.uv);
    float4 srcNormalSample = tex2D(dynamicTextureSrc1, input.uv);

    float alphaValue = blendedAlbedoSample.w;
    float invAlphaValue = 1.0 - alphaValue;

    //GFX_COMPLETE - move to premultiplied alpha
    output.albedo = srcAlbedoSample * invAlphaValue + float4(blendedAlbedoSample.xyz, blendedHeightSample);
    output.normal = srcNormalSample * invAlphaValue + float4(blendedNormalSample, alphaValue);

    return output;
}
