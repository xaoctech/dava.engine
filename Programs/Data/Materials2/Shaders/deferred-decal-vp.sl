#include "include/common.h"

vertex_in
{
    [vertex] float3 position : POSITION;
    [instance] float4 inst0 : TEXCOORD0; //(ex, maskMode);
    [instance] float4 inst1 : TEXCOORD1; //(ey, maskId);
    [instance] float4 inst2 : TEXCOORD2; //(ez, blendWidth);
    [instance] float4 inst3 : TEXCOORD3; //(p, freeToUse);
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 p_pos : TEXCOORD1;
    float3 tbnToWorld0 : TEXCOORD2;
    float3 tbnToWorld1 : TEXCOORD3;
    float3 tbnToWorld2 : TEXCOORD4;
    float3 tbnToWorldPos : TEXCOORD5;
    float3 invScale : TEXCOORD6;
};

[auto][instance] property float4x4 viewProjMatrix;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float4 inPosition = float4(input.position.xyz, 1.0);
    float3 ex = input.inst0.xyz;
    float3 ey = input.inst1.xyz;
    float3 ez = input.inst2.xyz;
    float3 wp = input.inst3.xyz;
    float4x4 worldMat = float4x4(float4(ex, 0.0),
                                 float4(ey, 0.0),
                                 float4(ez, 0.0),
                                 float4(wp, 1.0));
    float4 resPosition = mul(mul(inPosition, worldMat), viewProjMatrix);
    output.position = resPosition;
    output.p_pos = resPosition;

    //GFX_COMPLETE - trivial tamgent space - just move decal matrix to pixel shader (see DTE decal_base.fx)
    //GFX_COMPLETE2 - may be not
    float3 invScale = float3(1.0, 1.0, 1.0) / float3(length(ex), length(ey), length(ez));
    float3 worldNormal = ez * invScale.z;
    float3 worldTangent = ex * invScale.x;
    float3 worldBinormal = ey * invScale.y;

    output.tbnToWorld0 = float3(worldTangent.x, worldBinormal.x, worldNormal.x);
    output.tbnToWorld1 = float3(worldTangent.y, worldBinormal.y, worldNormal.y);
    output.tbnToWorld2 = float3(worldTangent.z, worldBinormal.z, worldNormal.z);

    output.tbnToWorldPos = wp;
    output.invScale = invScale;

    return output;
}
