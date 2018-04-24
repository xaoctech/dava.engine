#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

uniform sampler2D depth;

fragment_in
{
    float2 uv : TEXCOORD0;
    float2 inPos : TEXCOORD1;
};

fragment_out
{
    float4 color : SV_TARGET0;
};


struct ProjectionParameters
{
    float4x4 matrix;
    float4x4 inverseMatrix;
    float2 vpSize;
    float2 vpOffset;
    float2 rtSize;
    float2 zMapping;
};

float ReadDepth(float2 uv, float2 mapping)
{
    return (tex2D(depth, uv).x - mapping.y) / mapping.x;
}

float GetLinearDepth(in float d, ProjectionParameters params)
{
    float4 vp = mul(float4(0.0, 0.0, d, 1.0), params.inverseMatrix);
    return abs(vp.z / vp.w);
}

float3 GetViewSpacePosition(float2 xy, float z, float4x4 m)
{
    float4 vp = mul(float4(xy, z, 1.0), m);
    return vp.xyz / vp.w;
}

float3 GetScreenSpacePosition(float3 vsp, ProjectionParameters params)
{
    float4 pp = mul(float4(vsp, 1.0), params.matrix);
    float2 texPos = pp.xy / pp.w * ndcToUvMapping.xy + ndcToUvMapping.zw;
    float2 uv = (texPos * params.vpSize + centerPixelMapping + params.vpOffset) / params.rtSize;
    return float3(uv, pp.z / pp.w);
}

int MAX_SS_SHADOW_SAMPLES = 16;

float CastShadowRay(float3 origin, float3 direction, float depthComparisonTolerance, ProjectionParameters proj)
{
    float result = 1.0;
    
    float3 samplePosition = origin + direction + float3(0.0, 0.0, 0.01);
    for (int i = 0; i < MAX_SS_SHADOW_SAMPLES; ++i)
    {
        float4 projectedSample = mul(float4(samplePosition, 1.0), proj.matrix);
        projectedSample /= projectedSample.w;
        
        float2 texturePosition = projectedSample.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
        texturePosition = (texturePosition * proj.vpSize + centerPixelMapping + proj.vpOffset) / proj.rtSize;
        
        float sampledDepth = tex2D(depth, texturePosition);
        float ndcDepth = (sampledDepth - proj.zMapping.y) / proj.zMapping.x;
        
        float4 unprojectedPosition = mul(float4(projectedSample.xy, ndcDepth, 1.0), proj.inverseMatrix);
        unprojectedPosition /= unprojectedPosition.w;
        
        if ((unprojectedPosition.z > samplePosition.z) && (abs(unprojectedPosition.z - samplePosition.z) <= depthComparisonTolerance))
        {
            result = float(i) / float(MAX_SS_SHADOW_SAMPLES);
            break;
        }
        
        samplePosition += direction;
    }
    
    
    return result;
    
    /*
    float3 samplePosition = origin; // + direction;
    float4 projPos = mul(float4(samplePosition, 1.0), proj.matrix);
    projPos /= projPos.w;
    float4 viewPos = mul(projPos, proj.inverseMatrix);
    viewPos /= viewPos.w;
    return abs(viewPos.z - samplePosition.z) * 1000.0f;
    
    /*
    float3 projectedSample = GetScreenSpacePosition(samplePosition, proj);
    float sampledDepth = ReadDepth(projectedSample.xy, proj.zMapping);
    float linearDepth = GetLinearDepth(sampledDepth, proj);
    
    return abs(linearDepth - samplePosition.z);
    
    /*
    for (int i = 0; i < MAX_SS_SHADOW_SAMPLES; ++i)
    {
        float t;
        float3 projectedPosition = GetScreenSpacePosition(samplePosition, proj);
        float sampledDepth = ReadDepth(projectedPosition.xy, proj.zMapping);
        
        float linearDepth = GetLinearDepth(sampledDepth, proj);
        float depthDifference = linearDepth - samplePosition.z;
        result = 0.01 * abs(depthDifference);
        break;
        
        if ((depthDifference < 0.0) && (abs(depthDifference) < depthComparisonTolerance))
        {
            result = 0.0;
            break;
        }
        
        samplePosition += direction;
    }
    
    return result;
    */
}

fragment_out fp_main(fragment_in input)
{
    ProjectionParameters projParams;
    projParams.matrix = projMatrix;
    projParams.inverseMatrix = invProjMatrix;
    projParams.vpSize = viewportSize;
    projParams.vpOffset = viewportOffset;
    projParams.rtSize = renderTargetSize;
    projParams.zMapping = ndcToZMapping;
    
    float depthSample = ReadDepth(input.uv, ndcToZMapping);
    float3 viewSpacePosition = GetViewSpacePosition(input.inPos.xy, depthSample, invProjMatrix);
    float3 lightInViewSpace = mul(float4(lightPosition0.xyz, 0.0), viewMatrix);
    
    float distanceToFragment = -viewSpacePosition.z;
    float screenSpaceRadius = 0.1;
    float baseTolerance = 0.02;
    
    float sampleSize = screenSpaceRadius / projMatrix[0][0] * abs(viewSpacePosition.z);
    float3 viewSpaceStep = lightInViewSpace * (sampleSize / float(MAX_SS_SHADOW_SAMPLES));

    float tolerance = baseTolerance * distanceToFragment;
    float result = CastShadowRay(viewSpacePosition, viewSpaceStep, tolerance, projParams);
    
    fragment_out output;
    output.color = result;
    return output;
}
