#include "include/common.h"

color_mask = rg;

[auto][global] property float4x4 projMatrix;
[auto][global] property float4x4 viewMatrix;
[auto][global] property float4x4 worldMatrix;
[auto][global] property float4x4 prevWorldMatrix;
[auto][global] property float4x4 invViewMatrix;
[auto][global] property float4x4 invProjMatrix;
[auto][global] property float4x4 viewProjMatrix;
[auto][global] property float4x4 prevViewProjMatrix;
[auto][global] property float4x4 worldViewMatrix;
[auto][global] property float4x4 invViewProjMatrix;
[auto][global] property float4x4 invWorldViewMatrix;
[auto][global] property float4x4 worldInvTransposeMatrix;

[auto][global] property float4 cameraProjJitterPrevCurr;

uniform sampler2D depth;

fragment_in
{
    float2 uv : TEXCOORD0;
    float2 inPos : TEXCOORD1;
    float2 uvScale : TEXCOORD2;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float depthSample = tex2D(depth, input.uv).x;
    float3 ndcPos = float3(input.inPos, (depthSample - ndcToZMapping.y) / ndcToZMapping.x);
    float4 worldPos = mul(float4(ndcPos, 1.0), invViewProjMatrix);
    worldPos /= worldPos.w;

    float4 prevCoordNDCunjitter = mul(worldPos, prevViewProjMatrix);
    prevCoordNDCunjitter /= prevCoordNDCunjitter.w;
    prevCoordNDCunjitter.xy += cameraProjJitterPrevCurr.xy;

    float2 thisCoordNDCunjitter = input.inPos.xy + cameraProjJitterPrevCurr.zw;
    float2 vel = (prevCoordNDCunjitter.xy - thisCoordNDCunjitter.xy) * 0.5f;

    if (depthSample < 0.000001f)
        vel.xy = float2(0.0f, 0.0f);
    output.color = float4(vel * input.uvScale, 0.0f, 0.0f);
    return output;
}