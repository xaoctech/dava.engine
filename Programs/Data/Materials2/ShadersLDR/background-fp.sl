#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/atmosphere.h"

#ensuredefined CUBEMAP_ENVIRONMENT_TEXTURE 0
#ensuredefined EQUIRECTANGULAR_ENVIRONMENT_TEXTURE 0
#ensuredefined USE_FRAMEBUFFER_FETCH 0
#ensuredefined DIRECTIONAL_LIGHT 0
#ensuredefined ATMOSPHERE 0
#ensuredefined RGBM_INPUT 0

#if (DIRECTIONAL_LIGHT)
blending
{
    src = one
    dst = one
}
#endif

fragment_in
{
    float3 direction : TEXCOORD0;
    float2 normalizedCoordinates : TEXCOORD1;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

#if (CUBEMAP_ENVIRONMENT_TEXTURE)
uniform samplerCUBE environmentMap;
#endif

#if (EQUIRECTANGULAR_ENVIRONMENT_TEXTURE)
uniform sampler2D environmentMap;
#endif

[material][a] property float sunSmoothness = 0.0001;

fragment_out fp_main(fragment_in input)
{
    float3 normalizedDir = normalize(input.direction);

#if (ATMOSPHERE)

    float3 sunLuminance = dot(lightColor0.xyz, float3(0.2126, 0.7152, 0.0722)) / GLOBAL_LUMINANCE_SCALE;
    float3 value = SampleAtmosphere(cameraPosition, normalizedDir, lightPosition0.xyz, sunLuminance, fogParameters.y, fogParameters.z);
    value = LinearTosRGB(value);

#elif (CUBEMAP_ENVIRONMENT_TEXTURE)

    float4 sampledColor = texCUBElod(environmentMap, normalizedDir, 0.0);
    #if (RGBM_INPUT)
    sampledColor.xyz = DecodeRGBM(sampledColor);
    #endif

    float3 value = sampledColor.xyz * environmentColor.xyz / GLOBAL_LUMINANCE_SCALE;

#elif (EQUIRECTANGULAR_ENVIRONMENT_TEXTURE)

    float4 sampledColor = tex2Dlod(environmentMap, UnwrapDirectionToEquirectangular(normalizedDir), 0.0);
    #if (RGBM_INPUT)
    sampledColor.xyz = DecodeRGBM(sampledColor);
    #endif

    float3 value = sampledColor.xyz * environmentColor.xyz / GLOBAL_LUMINANCE_SCALE;

#elif (DIRECTIONAL_LIGHT)

    float r = smoothstep(1.0, 0.85, length(input.normalizedCoordinates));
    float3 value = environmentColor.xyz / GLOBAL_LUMINANCE_SCALE * r;

#else

    float3 value = environmentColor.xyz / GLOBAL_LUMINANCE_SCALE;

#endif

    fragment_out output;
    output.color = float4(value, 1.0);
    return output;
}
