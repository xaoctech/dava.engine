#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"
#include "include/hdr.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
    float4 texCoordTop : TEXCOORD1;
    float4 texCoordBottom : TEXCOORD2;
    float2 uniformTexCoords : TEXCOORD3;

#if (TECH_COMBINE || TECH_BLOOM_THRESHOLD)
    float luminanceHistoryValue : COLOR0;
#endif

#if (ENABLE_TXAA)
    float4 texClmp : TEXCOORD4;
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

#if (ENABLE_TXAA)
uniform sampler2D history;
uniform sampler2D velocity;
[material][a] property float2 destRectOffset;
[material][a] property float2 destRectSize;
[material][a] property float2 destTexSize;
#endif

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

#if (ENABLE_TXAA)
float3 ApplyTemporalAA(float2 texcoord, float luminanceHistoryValue, float4 texClmp)
{
    float2 velocitySample = tex2D(velocity, texcoord).xy;

    float2 historyUv = texcoord + velocitySample;
    float3 historySample = tex2D(history, historyUv).xyz;
    historySample = SRGBToLinear(historySample);

    float3 s00 = tex2D(hdrImage, texcoord + float2(-texelOffset.x, -texelOffset.y)).xyz;
    float3 s01 = tex2D(hdrImage, texcoord + float2(0, -texelOffset.y)).xyz;
    float3 s02 = tex2D(hdrImage, texcoord + float2(+texelOffset.x, -texelOffset.y)).xyz;
    float3 s10 = tex2D(hdrImage, texcoord + float2(-texelOffset.x, 0)).xyz;
    float3 s11 = tex2D(hdrImage, texcoord + float2(0, 0)).xyz;
    float3 s12 = tex2D(hdrImage, texcoord + float2(+texelOffset.x, 0)).xyz;
    float3 s20 = tex2D(hdrImage, texcoord + float2(-texelOffset.x, +texelOffset.y)).xyz;
    float3 s21 = tex2D(hdrImage, texcoord + float2(0, +texelOffset.y)).xyz;
    float3 s22 = tex2D(hdrImage, texcoord + float2(+texelOffset.x, +texelOffset.y)).xyz;

    s00 = HDRtoLDR(s00, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s01 = HDRtoLDR(s01, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s02 = HDRtoLDR(s02, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s10 = HDRtoLDR(s10, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s11 = HDRtoLDR(s11, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s12 = HDRtoLDR(s12, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s20 = HDRtoLDR(s20, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s21 = HDRtoLDR(s21, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    s22 = HDRtoLDR(s22, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);

    float3 minSample = min(s00, min(min(min(s01, s02), min(s10, s11)), min(min(s12, s20), min(s21, s22))));
    float3 maxSample = max(s00, max(max(max(s01, s02), max(s10, s11)), max(max(s12, s20), max(s21, s22))));
    float3 average = 1.0 / 9.0 * (s00 + s01 + s02 + s10 + s11 + s12 + s20 + s21 + s22);
    minSample = RGBToYCoCg(minSample);
    maxSample = RGBToYCoCg(maxSample);
    historySample = RGBToYCoCg(historySample);
    historySample = clamp(historySample, minSample, maxSample);
    float3 currentSample = RGBToYCoCg(s11);

    float weightMin = 0.15;
    float weightMax = 0.1;
    float lumDifference = abs(currentSample.y - historySample.y) / max(currentSample.y, max(historySample.y, 0.2));
    float weight = 1.0 - lumDifference;
    float weightSquared = weight * weight;
    weight = lerp(weightMin, weightMax, weightSquared);

    if (!all(equal(clamp(historyUv, texClmp.xy, texClmp.zw), historyUv)))
        weight = 1.0f;

    float3 temporal = lerp(historySample, currentSample, weight);

    float3 temporalToRGB = YCoCgToRGB(temporal);

    temporalToRGB = LDRtoHDR(temporalToRGB, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, luminanceHistoryValue);
    return temporalToRGB;
}
#endif // ENABLE_TXAA

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
    float3 ldr = HDRtoLDR(hdr, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, input.luminanceHistoryValue);
    ldr = LinearTosRGB(ldr);
    float ldrLuminance = dot(ldr, float3(0.2126, 0.7152, 0.0722));
#endif

#endif

    float3 originalColor = hdr;

#if ENABLE_TXAA
    hdr = ApplyTemporalAA(input.varTexCoord0, input.luminanceHistoryValue, input.texClmp);
#endif

    hdr = HDRtoLDR(hdr, cameraDynamicRange.x, cameraDynamicRange.y, cameraTargetLuminance, input.luminanceHistoryValue);
    hdr = LinearTosRGB(hdr);

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
    output.color = tex2D(heatmapTable, float2(ldrLuminance, 0.5));
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
