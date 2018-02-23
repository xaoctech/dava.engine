#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
    float4 texCoordTop : TEXCOORD1;
    float4 texCoordBottom : TEXCOORD2;
    float2 uniformTexCoords : TEXCOORD3;
#if (TECH_COMBINE || TECH_BLOOM_THRESHOLD)
    float luminanceHistoryValue : COLOR0;
#endif
};

fragment_out
{
#if (TECH_COMBINE)
    #if (COMBINE_INPLACE == 1)
    float4 color : SV_TARGET0;
    float4 luminance : SV_TARGET2;
    #elif (COMBINE_INPLACE == 2)
    float4 color : SV_TARGET5;
    float4 luminance : SV_TARGET3;
    #else
    float4 color : SV_TARGET0;
    #endif
#else
    float4 color : SV_TARGET0;
#endif
};

#if TECH_COMBINE

#if (COMBINE_INPLACE == 0)
uniform sampler2D hdrImage;
#endif
#if (ENABLE_COLOR_GRADING)
uniform sampler2D colorGradingTable;
#endif
#if (DISPLAY_HEAT_MAP)
uniform sampler2D heatmapTable;
#endif
uniform sampler2D lightMeterTable;

#else

uniform sampler2D tex0;

    #if (TECH_INIT_LUMINANCE)
uniform sampler2D lightMeterTable;
    #endif

#endif

#if ((TECH_LUMINANCE_HISTORY || TECH_COMBINED_LUMINANCE) && (TECH_COMBINED_LUMINANCE == 0))
uniform sampler2D luminancePrevious;
#endif

#if TECH_MUL_ADD_BLUR
uniform sampler2D tex1;
#endif

[material][a] property float2 srcTexSize;

[material][b] property float frameTime;
[material][b] property float exposure;
[material][b] property float lightMeterMaskWeight = 1.0;
[material][b] property float2 adaptationRange;
[material][b] property float2 adaptationSpeed;
[material][b] property float2 texelOffset;

#define GRAYSCALE_LUM 1

#if TECH_COMBINE

fragment_out fp_main(fragment_in input)
{
#if (COMBINE_INPLACE == 1)
    float3 hdr = FramebufferFetch(1).xyz;
#elif (COMBINE_INPLACE == 2)
    float3 hdr = FramebufferFetch(4).xyz;
#else

    float3 hdr = tex2D(hdrImage, input.varTexCoord0).xyz;
    #if (DISPLAY_HEAT_MAP)
    {
        float2 ts = 1.0 / viewportSize;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(-ts.x, -ts.y)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(0.0, -ts.y)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(+ts.x, -ts.y)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(-ts.x, 0.0)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(+ts.x, 0.0)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(-ts.x, +ts.y)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(0.0, +ts.y)).xyz;
        hdr += tex2D(hdrImage, input.varTexCoord0 + float2(+ts.x, +ts.y)).xyz;
        hdr *= 1.0 / 9.0;
    } 
    #endif
#endif

    float3 originalColor = hdr;

    float gain = input.luminanceHistoryValue / cameraTargetLuminance;
    float lumMin = gain * cameraDynamicRange.x;
    float lumMax = gain * cameraDynamicRange.y;
    hdr = (hdr - lumMin) / (lumMax - lumMin);

#if (DISPLAY_HEAT_MAP)
    float hdrlum = dot(hdr, float3(0.2126, 0.7152, 0.0722));
    float4 heatSample = tex2D(heatmapTable, float2(hdrlum, 0.5));
#endif

    {
    #if (ADVANCED_TONE_MAPPING)
        hdr = LinearTosRGB(hdr);
        float3x3 ACESInputMat = float3x3(float3(0.59719, 0.07600, 0.02840), float3(0.35458, 0.90834, 0.13383), float3(0.04823, 0.01566, 0.83777));
        float3x3 ACESOutputMat = float3x3(float3(1.60475, -0.10208, -0.00327), float3(-0.53108, 1.10813, -0.07276), float3(-0.07367, -0.00605, 1.07602));
        float a = 0.0245786;
        float b = 0.000090537;
        float c = 0.983729;
        float d = 0.4329510;
        float e = 0.238081;
        hdr = mul(hdr, ACESInputMat);
        hdr = (hdr * (hdr + a) - b) / (hdr * (c * hdr + d) + e);
        hdr = mul(hdr, ACESOutputMat);
    #else
        hdr = 1.0 - exp(-hdr);
        hdr = LinearTosRGB(hdr);
    #endif

        hdr = saturate(hdr);
    }

#if ((VIEW_MODE & VIEW_BY_COMPONENT_BIT) || (VIEW_MODE & VIEW_TEXTURE_MIP_LEVEL_BIT))
    {
#if (((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_BASE_COLOR) || (VIEW_MODE & VIEW_TEXTURE_MIP_LEVEL_BIT))
        hdr = lerp(hdr, LinearTosRGB(originalColor), 0.99999);
#else
        hdr = lerp(hdr, originalColor, 0.99999);
#endif
    }
#endif

#if (ENABLE_COLOR_GRADING)
    {
        float2 uvOffset = float2(0.5, 0.5);
        float u = hdr.x / 16.0 + uvOffset.x / 256.0;
        float v = hdr.y + uvOffset.y / 16.0;
        float z0 = floor(hdr.z * 16.0) / 16.0;
        float z1 = z0 + 1.0 / 16.0;
        float3 sample0 = tex2D(colorGradingTable, saturate(float2(u + z0, v))).xyz;
        float3 sample1 = tex2D(colorGradingTable, saturate(float2(u + z1, v))).xyz;
        hdr = lerp(sample0, sample1, (hdr.z - z0) * 16.0);
    }
#endif

    fragment_out output;

#if (DISPLAY_HEAT_MAP)
    output.color = heatSample;
#else
    output.color = float4(hdr, 1.0);
#endif

#if (DISPLAY_LIGHT_METER_MASK)
    float lmMask = FP_A8(tex2D(lightMeterTable, input.uniformTexCoords));
    output.color.xyz *= lmMask;
#endif

#if (COMBINE_INPLACE)
    float lmMask = FP_A8(tex2D(lightMeterTable, input.uniformTexCoords));
    float lum = dot(float3(0.2126, 0.7152, 0.0722), originalColor);
    output.luminance = log(max(lum, EPSILON)) * lmMask;
#endif

    return output;
}

#elif TECH_INIT_LUMINANCE

fragment_out fp_main(fragment_in input)
{
    float3 a = tex2D(tex0, input.texCoordTop.xy).xyz;
    float3 b = tex2D(tex0, input.texCoordTop.zw).xyz;
    float3 c = tex2D(tex0, input.texCoordBottom.xy).xyz;
    float3 d = tex2D(tex0, input.texCoordBottom.zw).xyz;
    float3 e = 0.25 * (a + b + c + d);

    float lmMask = FP_A8(tex2D(lightMeterTable, input.uniformTexCoords));

    fragment_out output;
    float luminance = dot(float3(0.2126, 0.7152, 0.0722), e);
    output.color = log(max(luminance, EPSILON)) * lmMask;
    return output;
}

#elif TECH_LUMINANCE

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

#if (GRAYSCALE_LUM)
    float a = tex2D(tex0, input.texCoordTop.xy).x;
    float b = tex2D(tex0, input.texCoordTop.zw).x;
    float c = tex2D(tex0, input.texCoordBottom.xy).x;
    float d = tex2D(tex0, input.texCoordBottom.zw).x;
    output.color = 0.25 * (a + b + c + d);
#else
    float3 a = tex2D(tex0, input.texCoordTop.xy).xyz;
    float3 b = tex2D(tex0, input.texCoordTop.zw).xyz;
    float3 c = tex2D(tex0, input.texCoordBottom.xy).xyz;
    float3 d = tex2D(tex0, input.texCoordBottom.zw).xyz;
    output.color = float4(0.25 * (a + b + c + d), 1.0);
#endif

    return output;
}

#elif TECH_FINISH_LUMINANCE

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

#if (GRAYSCALE_LUM)
    float a = tex2D(tex0, input.texCoordTop.xy).x;
    float b = tex2D(tex0, input.texCoordTop.zw).x;
    float c = tex2D(tex0, input.texCoordBottom.xy).x;
    float d = tex2D(tex0, input.texCoordBottom.zw).x;
    output.color = exp(0.25 * (a + b + c + d) * lightMeterMaskWeight);
#else
    float3 a = tex2D(tex0, input.texCoordTop.xy).xyz;
    float3 b = tex2D(tex0, input.texCoordTop.zw).xyz;
    float3 c = tex2D(tex0, input.texCoordBottom.xy).xyz;
    float3 d = tex2D(tex0, input.texCoordBottom.zw).xyz;
    float3 e = exp(0.25 * (a + b + c + d) * lightMeterMaskWeight);
    output.color = float4(e, 1.0);
#endif

    return output;
}

#elif TECH_DEBUG

fragment_out fp_main(fragment_in input)
{
    float3 hdr = tex2D(tex0, input.varTexCoord0).rgb;

    fragment_out output;
    output.color = float4(hdr, 1.0);
    return output;
}

#elif TECH_DEBUG_R16F

fragment_out fp_main(fragment_in input)
{
    float hdr = tex2D(tex0, input.varTexCoord0).r;

    fragment_out output;
    output.color = float4(hdr, hdr, hdr, 1.0);
    return output;
}

#elif TECH_LUMINANCE_HISTORY

fragment_out fp_main(fragment_in input)
{
    float curLum = tex2D(tex0, float2(0.5, 0.5)).r;
    curLum = clamp(curLum, adaptationRange.x, adaptationRange.y);

    float prevLum = tex2D(luminancePrevious, float2(0.5, 0.5)).r;
    float diff = curLum - prevLum;
    float dir = saturate(sign(diff));

    fragment_out output;
    output.color = prevLum + diff * (1.0 - exp(-frameTime * lerp(adaptationSpeed.x, adaptationSpeed.y, dir)));
    return output;
}

#elif TECH_COMBINED_LUMINANCE

fragment_out fp_main(fragment_in input)
{
#if (GRAYSCALE_LUM)
    float a = tex2D(tex0, input.texCoordTop.xy).x;
    float b = tex2D(tex0, input.texCoordTop.zw).x;
    float c = tex2D(tex0, input.texCoordBottom.xy).x;
    float d = tex2D(tex0, input.texCoordBottom.zw).x;
    float curLum = exp(0.25 * (a + b + c + d));
#else
    float3 a = tex2D(tex0, input.texCoordTop.xy).xyz;
    float3 b = tex2D(tex0, input.texCoordTop.zw).xyz;
    float3 c = tex2D(tex0, input.texCoordBottom.xy).xyz;
    float3 d = tex2D(tex0, input.texCoordBottom.zw).xyz;
    float curLum = dot(float3(0.2126, 0.7152, 0.0722), exp(0.25 * (a + b + c + d)));
#endif

    curLum = clamp(curLum, adaptationRange.x, adaptationRange.y);

    float prevLum = FramebufferFetch(0).x;
    float diff = curLum - prevLum;
    float dir = saturate(sign(diff));

    fragment_out output;
    output.color = prevLum + diff * (1.0 - exp(-frameTime * lerp(adaptationSpeed.x, adaptationSpeed.y, dir)));
    return output;
}

#elif TECH_RESET_LUMINANCE_HISTORY

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.color = tex2D(tex0, float2(0.5, 0.5));
    return output;
}

#elif TECH_COPY

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.color = tex2D(tex0, input.varTexCoord0);
    return output;
}

#elif TECH_BLOOM_THRESHOLD

fragment_out fp_main(fragment_in input)
{
    float maxLuminance = 0.0; // TODO

    float3 color = tex2D(tex0, input.varTexCoord0).xyz;
    float lum = dot(float3(0.2126, 0.7152, 0.0722), color);
    float val = step(maxLuminance, lum);

    fragment_out output;
    output.color = float4(val, val, val, 1.0);
    return output;
}

#elif TECH_GAUSS_BLUR

fragment_out fp_main(fragment_in input)
{
    float colorAcc = 0.0;
    /*
    float2 texCoord = input.varTexCoord0;
    colorAcc  = gaussWeight0 * tex2D(tex0, texCoord - texelOffset * gaussOffset0).r;
    colorAcc += gaussWeight1 * tex2D(tex0, texCoord - texelOffset * gaussOffset1).r;
    colorAcc += gaussWeight2 * tex2D(tex0, texCoord - texelOffset * gaussOffset2).r;
    colorAcc += gaussWeight3 * tex2D(tex0, texCoord).r;
    colorAcc += gaussWeight2 * tex2D(tex0, texCoord + texelOffset * gaussOffset2).r;
    colorAcc += gaussWeight1 * tex2D(tex0, texCoord + texelOffset * gaussOffset1).r;
    colorAcc += gaussWeight0 * tex2D(tex0, texCoord + texelOffset * gaussOffset0).r;
*/
    fragment_out output;
    output.color = colorAcc;
    return output;
}

#elif TECH_MUL_ADD_BLUR

fragment_out fp_main(fragment_in input)
{
    float3 color1 = 0.0;
    float3 color2 = 0.0;
    /*
    color1 = tex2D(tex0, input.varTexCoord0).rgb * mulAdd.x;
    color2 = tex2D(tex1, input.varTexCoord0).rgb * mulAdd.y;
    */
    fragment_out output;
    output.color = float4(color1 + color2, 1.0);
    return output;
}

#else

    #error Invalid technique

#endif
