#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/landscape-mask.h"

vertex_in
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

vertex_out
{
    float4 pos : SV_POSITION;
    float4 color : TEXCOORD0;
};

[material][a] property float4 landscapeUV = float4(0.0, 0.0, 0.0, 1.0);
[material][a] property float4 calcNormalUV = float4(0.0, 0.0, 0.0, 1.0);

/*
landscapeParams.x - landscapeSize
landscapeParams.y - landscapeHeight
landscapeParams.z - heightMapSize
*/
[material][a] property float4 landscapeParams = float4(0.0, 0.0, 0.0, 0.0);
[material][a] property float2 kernel = float2(0.01, 1.0);

/*
params.x - strength
params.y - brushRadius
*/
[material][a] property float4 params = float4(0.0, 0.0, 0.0, 0.0);

[auto][a] property float4x4 worldViewProjMatrix;

uniform sampler2D landscapeSourceHeightMap;

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
    pos.z = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, pos.xy, 0));

    float3 right;
    right.xy = uv + float2(offset, 0.0);
    right.z = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, right.xy, 0));

    float3 bottom;
    bottom.xy = uv + float2(0.0, offset);
    bottom.z = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, bottom.xy, 0));

    float3 left;
    left.xy = uv + float2(-offset, 0.0);
    left.z = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, left.xy, 0));

    float3 top;
    top.xy = uv + float2(0.0, -offset);
    top.z = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, top.xy, 0));

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

float3x3 rotationAxis(float3 normalAxis)
{
    float3 normal = normalAxis;
    float3 tangent = cross(float3(0.0, 1.0, 0.0), normal);
    float3 binormal = cross(normal, tangent);

    normal = normalize(normal);
    tangent = normalize(tangent);
    binormal = normalize(binormal);

    return float3x3(tangent, binormal, normal);
}

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    if (landscapeUV.w == 1.0 || calcNormalUV.w == 1.0)
    {
        output.pos = float4(0.0, 0.0, 0.0, 0.0);
    }
    else
    {
        float heightStart = SampleToHeight(tex2Dlod(landscapeSourceHeightMap, calcNormalUV.xy, 0.0));
        float4 normalAndH = float4(0.0, 0.0, 0.0, 0.0);
        {
            normalAndH = GetNormalHeightFromGradientKernel(calcNormalUV.xy, kernel.x, params.y, landscapeParams);
        }

        float avgHeight = normalAndH.w;
        float finalHeight = lerp(heightStart, avgHeight, kernel.y);

        float3x3 arrowTranform = rotationAxis(normalAndH.xyz);
        float3x3 scaleTranform = float3x3(landscapeParams.x * params.y);

        float3 rotatedPos = mul(mul(input.pos.xyz, scaleTranform), arrowTranform);

        float2 uvPos = landscapeUV.xy;
        rotatedPos.xy += 0.5 * landscapeParams.x * (2.0 * uvPos - 1.0);
        rotatedPos.z += landscapeParams.y * finalHeight;

        output.pos = mul(float4(rotatedPos, 1.0), worldViewProjMatrix);

        // add simple lighting
        float3 baseColor = float3(0.0, 1.0, 0.0);
        float3 lightDir = float3(1.0, 1.0, 0.0);

        float3 rotatedNormal = mul(input.normal, arrowTranform);
        float NdotL = saturate(dot(rotatedNormal, lightDir));
        output.color = float4(baseColor * NdotL * 0.5, 0.7);
    }
    return output;
}
