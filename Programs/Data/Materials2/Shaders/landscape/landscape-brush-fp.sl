#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/landscape-mask.h"

fragment_in
{
    float2 texCoord : TEXCOORD0;
};

fragment_out
{
    float4 target0 : SV_TARGET0;
};

#ensuredefined HEIGHTMAP_LOWER_RISE 0
#ensuredefined HEIGHTMAP_PUSH_PULL 0
#ensuredefined HEIGHTMAP_ADD_SUB 0
#ensuredefined HEIGHTMAP_SET 0
#ensuredefined HEIGHTMAP_FLATTEN 0
#ensuredefined HEIGHT_AVERAGE 0
#ensuredefined HEIGHT_SMOOTH 0
#ensuredefined HEIGHT_SHARPEN 0
#ensuredefined SHARPEN_LEVEL 0
#ensuredefined HEIGHT_NOISE 0
#ensuredefined MORPH_TO_R32F 0
#ensuredefined R32F_TO_MORPH 0
#ensuredefined COPY_TEXTURE_LOD 0
#ensuredefined GENERATE_TANGENT_MAP 0
#ensuredefined RENDER_BRUSH_FORM 0
#ensuredefined HEIGHT_CLONE 0

uniform sampler2D texture0;
uniform sampler2D texture1;

float gaussianKernel3[9] =
{
  0.077847, 0.123317, 0.077847,
  0.123317, 0.195346, 0.123317,
  0.077847, 0.123317, 0.077847
};

float gaussianKernel7[49] =
{
  0.000036, 0.000363, 0.001446, 0.002291, 0.001446, 0.000363, 0.000036,
  0.000363, 0.003676, 0.014662, 0.023226, 0.014662, 0.003676, 0.000363,
  0.001446, 0.014662, 0.058488, 0.092651, 0.058488, 0.014662, 0.001446,
  0.002291, 0.023226, 0.092651, 0.146768, 0.092651, 0.023226, 0.002291,
  0.001446, 0.014662, 0.058488, 0.092651, 0.058488, 0.014662, 0.001446,
  0.000363, 0.003676, 0.014662, 0.023226, 0.014662, 0.003676, 0.000363,
  0.000036, 0.000363, 0.001446, 0.002291, 0.001446, 0.000363, 0.000036
};

float gaussianKernel11[121] =
{
  0, 0, 0, 0, 0.000001, 0.000001, 0.000001, 0, 0, 0, 0,
  0, 0, 0.000001, 0.000014, 0.000055, 0.000088, 0.000055, 0.000014, 0.000001, 0, 0,
  0, 0.000001, 0.000036, 0.000362, 0.001445, 0.002289, 0.001445, 0.000362, 0.000036, 0.000001, 0,
  0, 0.000014, 0.000362, 0.003672, 0.014648, 0.023204, 0.014648, 0.003672, 0.000362, 0.000014, 0,
  0.000001, 0.000055, 0.001445, 0.014648, 0.058433, 0.092564, 0.058433, 0.014648, 0.001445, 0.000055, 0.000001,
  0.000001, 0.000088, 0.002289, 0.023204, 0.092564, 0.146632, 0.092564, 0.023204, 0.002289, 0.000088, 0.000001,
  0.000001, 0.000055, 0.001445, 0.014648, 0.058433, 0.092564, 0.058433, 0.014648, 0.001445, 0.000055, 0.000001,
  0, 0.000014, 0.000362, 0.003672, 0.014648, 0.023204, 0.014648, 0.003672, 0.000362, 0.000014, 0,
  0, 0.000001, 0.000036, 0.000362, 0.001445, 0.002289, 0.001445, 0.000362, 0.000036, 0.000001, 0,
  0, 0, 0.000001, 0.000014, 0.000055, 0.000088, 0.000055, 0.000014, 0.000001, 0, 0,
  0, 0, 0, 0, 0.000001, 0.000001, 0.000001, 0, 0, 0, 0
};

[material][a] property float4 params = float4(0.0, 0.0, 0.0, 0.0);

/*
landscapeParams.x - landscapeSize
landscapeParams.y - landscapeHeight
landscapeParams.z - heightMapSize
*/
[material][a] property float4 landscapeParams = float4(0.0, 0.0, 0.0, 0.0);

float HeightToObjectHeight(float h, float landMaxH)
{
    return (floor(h * 65535.0 + 0.5) * landMaxH) / 65535.0;
}

float4 GetNormalHeightFromGradientKernel(float2 uv, float kernel, float brushRadius, float4 landParams)
{
    float heightMapPixelSize = 1.0 / landParams.z;
    float offset = heightMapPixelSize + kernel * (saturate(brushRadius - heightMapPixelSize));

    float3 pos;
    pos.xy = uv;
    pos.z = SampleToHeight(tex2Dlod(texture1, pos.xy, 0));

    float3 right;
    right.xy = uv + float2(offset, 0.0);
    right.z = SampleToHeight(tex2Dlod(texture1, right.xy, 0));

    float3 bottom;
    bottom.xy = uv + float2(0.0, offset);
    bottom.z = SampleToHeight(tex2Dlod(texture1, bottom.xy, 0));

    float3 left;
    left.xy = uv + float2(-offset, 0.0);
    left.z = SampleToHeight(tex2Dlod(texture1, left.xy, 0));

    float3 top;
    top.xy = uv + float2(0.0, -offset);
    top.z = SampleToHeight(tex2Dlod(texture1, top.xy, 0));

    float meterOffset = offset * landParams.x;

    float3 topV = float3(0.0, -meterOffset, HeightToObjectHeight(top.z - pos.z, landParams.y));
    float3 rightV = float3(meterOffset, 0.0, HeightToObjectHeight(right.z - pos.z, landParams.y));
    float3 bottomV = float3(0.0, meterOffset, HeightToObjectHeight(bottom.z - pos.z, landParams.y));
    float3 leftV = float3(-meterOffset, 0.0, HeightToObjectHeight(left.z - pos.z, landParams.y));

    float3 normal0 = cross(topV, rightV);
    float3 normal1 = cross(rightV, bottomV);
    float3 normal2 = cross(bottomV, leftV);
    float3 normal3 = cross(leftV, topV);

    float3 normalAverage = normalize(normal0 + normal1 + normal2 + normal3);

    float height = (top.z + right.z + bottom.z + left.z + pos.z) / 5.0;

    return float4(normalAverage.x, normalAverage.y, normalAverage.z, height);
}

#if HEIGHTMAP_LOWER_RISE
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0));
    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    height = height + sign(params.x) * brushFactor * params.y;

    return float4(clamp(height, 0.0, 1.0), 0.0, 0.0, 0.0);
}
#elif HEIGHTMAP_PUSH_PULL
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0));
    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float newHeight = saturate(height + params.x);

    return lerp(height, newHeight, brushFactor);
}
#elif HEIGHTMAP_ADD_SUB
float4 ApplyBrush(float2 inputTex)
{
    float height0 = SampleToHeight(tex2Dlod(texture0, inputTex, 0));
    float height1 = SampleToHeight(tex2Dlod(texture1, inputTex, 0));
    float height = height1;
    float brushFactor = sign(params.x) * params.y * GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    if (brushFactor > 0.0)
    {
        height = max(height0 + brushFactor, height1);
    }
    else if (brushFactor < 0.0)
    {
        height = min(height0 + brushFactor, height1);
    }

    return float4(clamp(height, 0.0, 1.0), 0.0, 0.0, 0.0);
}
#elif HEIGHTMAP_SET
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    height = lerp(height, params.x, brushFactor);

    return float4(clamp(height, 0.0, 1.0), 0.0, 0.0, 0.0);
}
#elif HEIGHTMAP_FLATTEN
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float operationMask = brushFactor * params.z;

    if (brushFactor > 0.0)
    {
        float weigth = 0.0;
        float avgHeight = 0.0;
        for (int y = -5; y < 6; ++y)
        {
            for (int x = -5; x < 6; ++x)
            {
                float2 texCoord = params.xy + 0.5 * float2(x / 5.0, y / 5.0) * landCursorPosition.zw;
                float opStartHeight = brushFactor * SampleToHeight(tex2Dlod(texture1, texCoord, 0));
                weigth += brushFactor;
                avgHeight += opStartHeight;
            }
        }
        avgHeight /= weigth;
        height = lerp(height, avgHeight, operationMask);
    }

    return float4(clamp(height, 0.0, 1.0), 0.0, 0.0, 0.0);
}
#elif HEIGHT_AVERAGE
[material][a] property float2 uvPos = float2(0.0, 0.0);
[material][a] property float2 kernel = float2(0.01, 1.0);
float4 ApplyBrush(float2 inputTex)
{
    float src0 = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));
    float heightStart = SampleToHeight(tex2Dlod(texture1, uvPos, 0.0));

    float4 dirAndH = GetNormalHeightFromGradientKernel(uvPos, kernel.x, params.y, landscapeParams);

    float3 direction = dirAndH.xyz;
    float avgHeight = dirAndH.w;

    float finalHeight = lerp(heightStart, avgHeight, kernel.y);

    float3 pointStart = float3(uvPos.x, uvPos.y, finalHeight);

    // Plane Equation
    // A * (x - x0) + B * (y - y0) + C * (z - z0) == 0 ->
    // z = (A * (x0 - x) + B * (y0 - y) + C * z0) / C;
    float meterXOffset = landscapeParams.x * direction.x * (pointStart.x - inputTex.x);
    float meterYOffset = landscapeParams.x * direction.y * (pointStart.y - inputTex.y);
    float res = (landscapeParams.y * pointStart.z + ((meterXOffset + meterYOffset) / direction.z)) / landscapeParams.y;

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float operationMask = brushFactor * params.x;

    return saturate(lerp(src0, res, operationMask));
}
#elif HEIGHT_SMOOTH
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float operationMask = brushFactor * params.x;

    if (brushFactor > 0.0)
    {
        float filtered = 0.0;
        float weight = 0.0;
        float kernel = params.y * 10.0;

        for (int y = 0; y < 11; ++y)
        {
            for (int x = 0; x < 11; ++x)
            {
                float2 sampleTexcoord = inputTex + float2(x - 5.0, y - 5.0) * kernel;
                float sample = gaussianKernel11[x + 11 * y] * brushFactor;
                weight += sample;

                float sampleHeight = SampleToHeight(tex2Dlod(texture0, sampleTexcoord, 0.0));
                filtered += sampleHeight * sample;
            }
        }

        filtered /= weight;
        height = lerp(height, filtered, operationMask);
    }

    return height;
}
#elif HEIGHT_SHARPEN
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float operationMask = brushFactor * params.x;

    float filtered = 0;
    float weight = 0.00001;

#if SHARPEN_LEVEL 0
    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            float2 sampleTexcoord = inputTex + params.y * float2(x - 1.0, y - 1.0);
            float sampleBrushFactor = GetBrushMaskFactor(sampleTexcoord, landCursorPosition, cursorRotation, invertFactor);
            float sample = gaussianKernel3[x + 3 * y] * sampleBrushFactor;
            weight += sample;

            float sampleHeight = SampleToHeight(tex2Dlod(texture0, sampleTexcoord, 0.0));
            filtered += sampleHeight * sample;
        }
    }
#elif SHARPEN_LEVEL 1
    for (int y = 0; y < 7; ++y)
    {
        for (int x = 0; x < 7; ++x)
        {
            float2 sampleTexcoord = inputTex + params.y * float2(x - 3.0, y - 3.0);
            float sampleBrushFactor = GetBrushMaskFactor(sampleTexcoord, landCursorPosition, cursorRotation, invertFactor);
            float sample = gaussianKernel7[x + 7 * y] * sampleBrushFactor;
            weight += sample;

            float sampleHeight = SampleToHeight(tex2Dlod(texture0, sampleTexcoord, 0.0));
            filtered += sampleHeight * sample;
        }
    }
#elif SHARPEN_LEVEL 2
    for (int y = 0; y < 11; ++y)
    {
        for (int x = 0; x < 11; ++x)
        {
            float2 sampleTexcoord = inputTex + params.y * float2(x - 5.0, y - 5.0);
            float sampleBrushFactor = GetBrushMaskFactor(sampleTexcoord, landCursorPosition, cursorRotation, invertFactor);
            float sample = gaussianKernel11[x + 11 * y] * sampleBrushFactor;
            weight += sample;

            float sampleHeight = SampleToHeight(tex2Dlod(texture0, sampleTexcoord, 0.0));
            filtered += sampleHeight * sample;
        }
    }
#endif

    if (weight == 0.0)
    {
        filtered = height;
    }
    else
    {
        filtered /= weight;
    }

    height = height + (height - filtered) * operationMask * 0.1;

    return height;
}
#elif HEIGHT_NOISE
[material][a] property float4 random = float4(0.0, 0.0, 0.0, 0.0);

/*
params.x - strength
params.y - noiseKernelSize
params.z - renderTargetSize
params.w - noiseMode
*/
float4 ApplyBrush(float2 inputTex)
{
    float height = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    float operationMask = brushFactor * params.x * 0.01;
    float2 noiseTexCoord = (inputTex * float2(params.z, params.z) / float2(1024, 1024) + random.xy) * params.y;

    noiseTexCoord = float2(noiseTexCoord.x * random.w - noiseTexCoord.y * random.z,
                           noiseTexCoord.x * random.z + noiseTexCoord.y * random.w);

    float noise = tex2Dlod(texture1, noiseTexCoord, 0).r;
    noise = lerp(noise, noise * 2.0 - 1.0, params.w);
    height = height + operationMask * noise;

    return saturate(height);
}
#elif HEIGHT_CLONE
float4 ApplyBrush(float2 inputTex)
{
    float2 offseted = inputTex + params.xy;
    float from = SampleToHeight(tex2Dlod(texture1, offseted, 0.0));
    float to = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));

    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    //if (offseted.x > 1.0 || offseted.x < 0.0 || offseted.y > 1.0 || offseted.y < 0.0)
    // {
    //    brushFactor = 0.0;
    //}

    return lerp(to, from, brushFactor);
}
#elif MORPH_TO_R32F
float4 ApplyBrush(float2 inputTex)
{
    float h = SampleToHeight(tex2Dlod(texture0, inputTex, 0.0));
    return h;
}
#elif R32F_TO_MORPH
[material][a] property float mipTextureSize = 0;

float FloatToHeight(float intV)
{
    return floor((intV * 65535.0) + 0.5);
}

float2 DecomposeHeightToRG(float v)
{
    float intValue = v;
    float highIntValue = floor(intValue / 256.0);
    float lowIntValue = floor(fmod(intValue, 256.0));

    return float2(lowIntValue / 255.0, highIntValue / 255.0);
}

float4 ApplyBrush(float2 inputTex)
{
    float mappingStep = 1.0 / mipTextureSize;
    float halfMappingStep = 0.5 * mappingStep;

    float2 mappedTex = inputTex - float2(halfMappingStep, halfMappingStep);
    float2 mappedTexAvg1 = mappedTex;
    float2 mappedTexAvg2 = mappedTex;

    float cx = fmod(floor(inputTex.x * (mipTextureSize - 1.0) + 0.5), 2.0);
    float cy = fmod(floor(inputTex.y * (mipTextureSize - 1.0) + 0.5), 2.0);

    cx = step(0.5, cx);
    cy = step(0.5, cy);

    if ((inputTex.x * (mipTextureSize - 1.0) + 0.5) < (mipTextureSize - 1.0))
    {
        float offset = cx * mappingStep;
        mappedTexAvg1.x = mappedTexAvg1.x - offset;
        mappedTexAvg2.x = mappedTexAvg2.x + offset;
    }

    if ((inputTex.y * (mipTextureSize - 1.0) + 0.5) < (mipTextureSize - 1.0))
    {
        float offset = cy * mappingStep;
        mappedTexAvg1.y = mappedTexAvg1.y + offset;
        mappedTexAvg2.y = mappedTexAvg2.y - offset;
    }

    float accFetched = tex2D(texture0, mappedTex).r;
    float fetched1 = tex2D(texture0, mappedTexAvg1).r;
    float h1 = FloatToHeight(fetched1);
    float fetched2 = tex2D(texture0, mappedTexAvg2).r;
    float h2 = FloatToHeight(fetched2);
    float avgHeight = (h1 + h2) / 2.0;

    float2 accDecomposed = DecomposeHeightToRG(FloatToHeight(accFetched));
    float2 avgDecomposed = DecomposeHeightToRG(avgHeight);

    float4 result;
    result.x = accDecomposed.x;
    result.y = accDecomposed.y;
    result.z = avgDecomposed.x;
    result.w = avgDecomposed.y;

    return result;
}
#elif GENERATE_TANGENT_MAP
[material][a] property float floatTextureSize = 0;

float4 ApplyBrush(float2 inputTex)
{
    float mappingStep = 1.0 / landscapeParams.z;
    float halfMappingStep = 0.5 * mappingStep;

    float2 mappedTex = inputTex - float2(halfMappingStep, halfMappingStep);
    float2 mappedTexAvg1 = mappedTex;
    float2 mappedTexAvg2 = mappedTex;

    float cx = fmod(floor(inputTex.x * (landscapeParams.z - 1.0) + 0.5), 2.0);
    float cy = fmod(floor(inputTex.y * (landscapeParams.z - 1.0) + 0.5), 2.0);

    cx = step(0.5, cx);
    cy = step(0.5, cy);

    if ((inputTex.x * (landscapeParams.z - 1.0) + 0.5) < (landscapeParams.z - 1.0))
    {
        float offset = cx * mappingStep;
        mappedTexAvg1.x = mappedTexAvg1.x - offset;
        mappedTexAvg2.x = mappedTexAvg2.x + offset;
    }

    if ((inputTex.y * (landscapeParams.z - 1.0) + 0.5) < (landscapeParams.z - 1.0))
    {
        float offset = cy * mappingStep;
        mappedTexAvg1.y = mappedTexAvg1.y + offset;
        mappedTexAvg2.y = mappedTexAvg2.y - offset;
    }

    float texelSize = 1.0 / floatTextureSize;
    float3 normal0 = GetNormalHeightFromGradientKernel(mappedTex, 0.0, 0.0, landscapeParams).xyz;
    float3 normal1 = GetNormalHeightFromGradientKernel(mappedTexAvg1, 0.0, 0.0, landscapeParams).xyz;
    float3 normal2 = GetNormalHeightFromGradientKernel(mappedTexAvg2, 0.0, 0.0, landscapeParams).xyz;

    normal0 = normal0 * 0.5f + 0.5f;
    float nxAcc = floor(normal0.x * 255.0) / 255.0;
    float nyAcc = floor(normal0.y * 255.0) / 255.0;

    float3 normal = normalize(normal1 + normal2) * 0.5f + 0.5f;
    float nxAvg = floor(normal.x * 255.0) / 255.0;
    float nyAvg = floor(normal.y * 255.0) / 255.0;

    float4 result;
    result.x = nxAcc;
    result.y = nyAcc;
    result.z = nxAvg;
    result.w = nyAvg;

    return result;
}
#elif COPY_TEXTURE_LOD
[material][a] property float lodLevel = 0;
float4 ApplyBrush(float2 inputTex)
{
    return tex2Dlod(texture0, inputTex, lodLevel);
}
#elif RENDER_BRUSH_FORM
float4 ApplyBrush(float2 inputTex)
{
    float brushFactor = GetBrushMaskFactor(inputTex, landCursorPosition, cursorRotation, invertFactor);
    return float4(params.xyz, brushFactor);
}
#else
float4 ApplyBrush(float2 inputTex)
{
    return float4(0.0, 0.0, 0.0, 0.0);
}
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.target0 = ApplyBrush(input.texCoord);

    return output;
}
