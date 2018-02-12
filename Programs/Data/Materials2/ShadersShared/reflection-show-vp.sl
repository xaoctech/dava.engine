#include "include/common.h"
#include "include/shading-options.h"
#include "include/structures.h"

vertex_in
{
    float3 position : POSITION;
    float2 texCoord0 : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

vertex_out
{
    float4 position : SV_POSITION;

    float2 varTexCoord0 : TEXCOORD0;
    float3 varToCamera : TEXCOORD1;
    float3 tangentToFinal0 : TEXCOORD3;
    float3 tangentToFinal1 : TEXCOORD4;
    float3 tangentToFinal2 : TEXCOORD5;
};

[auto][a] property float4x4 worldInvTransposeMatrix;
[auto][a] property float4x4 worldMatrix;
[auto][a] property float4x4 viewProjMatrix;

[auto][a] property float4 lightPosition0;
[auto][a] property float3 cameraPosition;
[material][a] property float3 capturePositionLocalShift = float3(0.0, 0.0, 0.0);

vertex_out vp_main(vertex_in input)
{
    float4 worldPosition = mul(float4(input.position.xyz + capturePositionLocalShift, 1.0), worldMatrix);

    float3 worldNormal = normalize(mul(float4(input.normal, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldTangent = normalize(mul(float4(input.tangent, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldBinormal = normalize(mul(float4(input.binormal, 0.0), worldInvTransposeMatrix).xyz);

    vertex_out output;
    output.varTexCoord0 = input.texCoord0;
    output.varToCamera = cameraPosition - worldPosition.xyz;
    output.tangentToFinal0 = float3(worldTangent.x, worldBinormal.x, worldNormal.x);
    output.tangentToFinal1 = float3(worldTangent.y, worldBinormal.y, worldNormal.y);
    output.tangentToFinal2 = float3(worldTangent.z, worldBinormal.z, worldNormal.z);
    output.position = mul(worldPosition, viewProjMatrix);
    return output;
}
