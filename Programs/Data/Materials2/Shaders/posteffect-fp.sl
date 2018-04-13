#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"
#include "include/hdr.h"

#ensuredefined TXAA_YCOCG_SPACE 1
#ensuredefined TXAA_DEPTH_DILATION 0
#ensuredefined TXAA_USE_CLIP_MODE 2 // 0 - clamp, 1 - intersect fast (maybe), 2 - intersect as a king, but slower.

int CmpZ(float a, float b)
{
    return a < b; // GFX_COMPLETE wrong for non inverse z.
}

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

#if TXAA_DEPTH_DILATION
uniform sampler2D depth; // Do not forget to pass this texture from c++ side.
#endif

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

#if (ENABLE_COLOR_GRADING)
float3 ApplyColorGrading(in float3 color)
{
    float2 uvOffset = float2(0.5, 0.5);
    float u = color.x / 16.0 + uvOffset.x / 256.0;
    float v = color.y + uvOffset.y / 16.0;
    float z0 = floor(color.z * 16.0) / 16.0;
    float z1 = z0 + 1.0 / 16.0;
    float3 sample0 = tex2D(colorGradingTable, saturate(float2(u + z0, v))).xyz;
    float3 sample1 = tex2D(colorGradingTable, saturate(float2(u + z1, v))).xyz;
    return lerp(sample0, sample1, (color.z - z0) * 16.0);
}
#endif

#if TECH_COMBINE

#if (ENABLE_TXAA)

#if TXAA_DEPTH_DILATION
float3 DilateDepth(float2 texcoord_depth)
{
    float du = texelOffset.x;
    float dv = texelOffset.y;

    float3 depth_tl = float3(-du, -dv, tex2D(depth, texcoord_depth - dv - du).x);
    float3 depth_tc = float3(0, -dv, tex2D(depth, texcoord_depth - dv).x);
    float3 depth_tr = float3(du, -dv, tex2D(depth, texcoord_depth - dv + du).x);

    float3 depth_ml = float3(-du, 0, tex2D(depth, texcoord_depth - du).x);
    float3 depth_mc = float3(0, 0, tex2D(depth, texcoord_depth).x);
    float3 depth_mr = float3(du, 0, tex2D(depth, texcoord_depth + du).x);

    float3 depth_bl = float3(-du, dv, tex2D(depth, texcoord_depth + dv - du).x);
    float3 depth_bc = float3(0, dv, tex2D(depth, texcoord_depth + dv).x);
    float3 depth_br = float3(du, dv, tex2D(depth, texcoord_depth + dv + du).x);

    float3 depthMin = depth_tl;

    if (CmpZ(depth_tl.z, depth_tc.z))
        depthMin = depth_tc;
    if (CmpZ(depthMin.z, depth_tr.z))
        depthMin = depth_tr;

    if (CmpZ(depthMin.z, depth_ml.z))
        depthMin = depth_ml;
    if (CmpZ(depthMin.z, depth_mc.z))
        depthMin = depth_mc;
    if (CmpZ(depthMin.z, depth_mr.z))
        depthMin = depth_mr;

    if (CmpZ(depthMin.z, depth_bl.z))
        depthMin = depth_bl;
    if (CmpZ(depthMin.z, depth_bc.z))
        depthMin = depth_bc;
    if (CmpZ(depthMin.z, depth_br.z))
        depthMin = depth_br;

    return float3(texcoord_depth + depthMin.xy, depthMin.z);
}
#endif

struct TAAData
{
    float2 texelOffset;
    float2 cameraDynamicRange;
    float cameraTargetLuminance;
};

float3 ApplyTemporalAA(float2 texcoord, TAAData data, float luminanceHistoryValue, float4 texClmp)
{
    float2 velocityUv = texcoord;
#if TXAA_DEPTH_DILATION
    float3 dilated = DilateDepth(velocityUv);
    velocityUv = dilated.xy;
#endif
    float2 velocitySample = tex2D(velocity, velocityUv).xy;

    float2 historyUv = texcoord + velocitySample;
    float3 historySample = tex2D(history, historyUv).xyz;
    historySample = SRGBToLinear(historySample);

    float3 s00 = tex2D(hdrImage, texcoord + float2(-data.texelOffset.x, -data.texelOffset.y)).xyz;
    float3 s01 = tex2D(hdrImage, texcoord + float2(0, -data.texelOffset.y)).xyz;
    float3 s02 = tex2D(hdrImage, texcoord + float2(+data.texelOffset.x, -data.texelOffset.y)).xyz;
    float3 s10 = tex2D(hdrImage, texcoord + float2(-data.texelOffset.x, 0)).xyz;
    float3 s11 = tex2D(hdrImage, texcoord + float2(0, 0)).xyz;
    float3 s12 = tex2D(hdrImage, texcoord + float2(+data.texelOffset.x, 0)).xyz;
    float3 s20 = tex2D(hdrImage, texcoord + float2(-data.texelOffset.x, +data.texelOffset.y)).xyz;
    float3 s21 = tex2D(hdrImage, texcoord + float2(0, +data.texelOffset.y)).xyz;
    float3 s22 = tex2D(hdrImage, texcoord + float2(+data.texelOffset.x, +data.texelOffset.y)).xyz;

    s00 = HDRtoLDR(s00, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s01 = HDRtoLDR(s01, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s02 = HDRtoLDR(s02, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s10 = HDRtoLDR(s10, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s11 = HDRtoLDR(s11, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s12 = HDRtoLDR(s12, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s20 = HDRtoLDR(s20, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s21 = HDRtoLDR(s21, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    s22 = HDRtoLDR(s22, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);

#if (ENABLE_COLOR_GRADING)
    s00 = ApplyColorGrading(s00);
    s01 = ApplyColorGrading(s01);
    s02 = ApplyColorGrading(s02);
    s10 = ApplyColorGrading(s10);
    s11 = ApplyColorGrading(s11);
    s12 = ApplyColorGrading(s12);
    s20 = ApplyColorGrading(s20);
    s21 = ApplyColorGrading(s21);
    s22 = ApplyColorGrading(s22);
#endif

#if TXAA_YCOCG_SPACE
    historySample = RGBToYCoCg(historySample);
    s00 = RGBToYCoCg(s00);
    s01 = RGBToYCoCg(s01);
    s02 = RGBToYCoCg(s02);
    s10 = RGBToYCoCg(s10);
    s11 = RGBToYCoCg(s11);
    s12 = RGBToYCoCg(s12);
    s20 = RGBToYCoCg(s20);
    s21 = RGBToYCoCg(s21);
    s22 = RGBToYCoCg(s22);
#endif
    
    float3 crossMin = min(s01, s10);
    crossMin = min(crossMin, s11);
    crossMin = min(crossMin, s12);
    crossMin = min(crossMin, s21);
    float3 minSample = min(crossMin, s00);
    minSample = min(minSample, s02);
    minSample = min(minSample, s20);
    minSample = min(minSample, s22);

    float3 crossMax = max(s01, s10);
    crossMax = max(crossMax, s11);
    crossMax = max(crossMax, s12);
    crossMax = max(crossMax, s21);
    float3 maxSample = max(crossMax, s00);
    maxSample = max(maxSample, s02);
    maxSample = max(maxSample, s20);
    maxSample = max(maxSample, s22);
    minSample = lerp(minSample, crossMin, 0.5);
    maxSample = lerp(maxSample, crossMax, 0.5);

    float3 average = 1.0 / 9.0 * (s00 + s01 + s02 + s10 + s11 + s12 + s20 + s21 + s22);
    float3 currentSample = s11;

#if TXAA_USE_CLIP_MODE == 1
    historySample = IntersectionAabbCenter(historySample, minSample, maxSample); // col origin - histSample, colDirection - (histSample - (minSample + maxSample)/2)
#elif TXAA_USE_CLIP_MODE == 2
    float3 colDir = average - historySample;
    float t = IntersectionAabbNear(historySample, colDir + float3(0.000001, 0.000001, 0.000001), minSample, maxSample);
    historySample = lerp(historySample, average, saturate(t));
#else
    historySample = clamp(historySample, minSample, maxSample);
#endif

    float weightMin = 0.15;
    float weightMax = 0.1;
    float lumDifference = abs(currentSample.y - historySample.y) / max(currentSample.y, max(historySample.y, 0.2));
    float weight = 1.0 - lumDifference;
    float weightSquared = weight * weight;
    weight = lerp(weightMin, weightMax, weightSquared);

    if (!all(equal(clamp(historyUv, texClmp.xy, texClmp.zw), historyUv)))
        weight = 1.0f;

    float3 temporal = lerp(historySample, currentSample, weight);

#if TXAA_YCOCG_SPACE
    float3 temporalToRGB = YCoCgToRGB(temporal);
#else
    float3 temporalToRGB = temporal;
#endif
    
    temporalToRGB = LDRtoHDR(temporalToRGB, data.cameraDynamicRange.x, data.cameraDynamicRange.y, data.cameraTargetLuminance, luminanceHistoryValue);
    
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
    TAAData taaData;
    {
        taaData.texelOffset = texelOffset;
        taaData.cameraDynamicRange = cameraDynamicRange;
        taaData.cameraTargetLuminance = cameraTargetLuminance;
    }
    hdr = ApplyTemporalAA(input.varTexCoord0, taaData, input.luminanceHistoryValue, input.texClmp);
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

#if (ENABLE_COLOR_GRADING) && (ENABLE_TXAA == 0) // if TAA enabled - color grading applied in TAA part
    hdr = ApplyColorGrading(hdr);
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

#endif
