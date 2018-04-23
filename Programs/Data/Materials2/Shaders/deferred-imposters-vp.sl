#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

vertex_in
{
    float3 position : POSITION; // imposter center
    float4 data : TEXCOORD4; // imposter size
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 projectedPosition : TEXCOORD0;
    float4 texCoords : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 bitangent : TEXCOORD4;
    float frameDiff : TEXCOORD5;
};

vertex_out vp_main(vertex_in input)
{
    float uniformWorldScale = max(worldScale.x, max(worldScale.y, worldScale.z));
    float3 pivotWorldPosition = mul(float4(input.position.xyz, 1.0), worldMatrix);
    float3 up = float3(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]) / worldScale.z;
    
    float3 toCamera = normalize(cameraPosition - pivotWorldPosition);
    float3 side = normalize(cross(up, toCamera));
    
    float3 worldPosition = pivotWorldPosition;
    worldPosition += side * (input.data.x * uniformWorldScale);
    worldPosition += up * (input.data.y * uniformWorldScale);

    float3 rotatedCamera = normalize(mul(float4(toCamera * float3(1.0, -1.0, 0.0), 0.0), worldInvTransposeMatrix).xyz);

    float angle = 0.5 - atan2(rotatedCamera.y, rotatedCamera.x) / (2.0 * _PI);
    float a0 = floor(angle * IMPOSTER_FRAME_COUNT + 0.0) / IMPOSTER_FRAME_COUNT;
    float a1 = floor(angle * IMPOSTER_FRAME_COUNT + 1.0) / IMPOSTER_FRAME_COUNT;

    vertex_out output;
    {
        output.normal = toCamera * float3(1.0, 1.0, 0.0);
        output.bitangent = float3(0.0, 0.0, 1.0);
        output.tangent = normalize(cross(output.bitangent, output.normal));
        
        output.projectedPosition = mul(float4(worldPosition, 1.0), viewProjMatrix);
        output.texCoords.xy = float2(a0 + input.data.z / IMPOSTER_FRAME_COUNT, input.data.w);
        output.texCoords.zw = float2(a1 + input.data.z / IMPOSTER_FRAME_COUNT, input.data.w);
        output.frameDiff = (angle - a0) * IMPOSTER_FRAME_COUNT;
        output.position = output.projectedPosition;
    }
    return output;
}
