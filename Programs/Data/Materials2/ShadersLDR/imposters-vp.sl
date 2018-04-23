#define LDR_FLOW 1

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
    float4 texCoords : TEXCOORD0;
    float4 projectedPosition : TEXCOORD1;
#if (WRITE_SHADOW_MAP == 0)
    float3 varToCamera : TEXCOORD2;
    float4 shadowTexCoord : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
#endif
    float frameDiff : TEXCOORD5;
};

vertex_out vp_main(vertex_in input)
{
    float framesCount = 16.0;
    
    float uniformWorldScale = max(worldScale.x, max(worldScale.y, worldScale.z));
    float3 pivotWorldPosition = mul(float4(input.position.xyz, 1.0), worldMatrix);
    float3 up = float3(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]) / worldScale.z;
    
    vertex_out output;
    
#if (WRITE_SHADOW_MAP)
    float3 toCamera = lightPosition0.xyz;
#else
    output.varToCamera = cameraPosition - pivotWorldPosition;
    float3 toCamera = normalize(output.varToCamera);
#endif
    
    float3 side = normalize(cross(up, toCamera));
    
    float3 worldPosition = pivotWorldPosition;
    worldPosition += side * (input.data.x * uniformWorldScale);
    worldPosition += up * (input.data.y * uniformWorldScale);
    
    float3 rotatedCamera = mul(float4(toCamera * float3(1.0, -1.0, 0.0), 0.0), worldInvTransposeMatrix);
    
    float angle = 0.5 - atan2(rotatedCamera.y, rotatedCamera.x) / (2.0 * _PI);
    float a0 = floor(angle * IMPOSTER_FRAME_COUNT + 0.0) / IMPOSTER_FRAME_COUNT;
    float a1 = floor(angle * IMPOSTER_FRAME_COUNT + 1.0) / IMPOSTER_FRAME_COUNT;
    
#if (WRITE_SHADOW_MAP == 0)
    output.worldPosition = worldPosition;
    output.shadowTexCoord = mul(float4(worldPosition, 1.0), shadowView);
    output.normal = toCamera * float3(1.0, 1.0, 0.0);
    output.bitangent = float3(0.0, 0.0, 1.0);
    output.tangent = normalize(cross(output.bitangent, output.normal));
#endif
    
    output.projectedPosition = mul(float4(worldPosition, 1.0), viewProjMatrix);
    output.texCoords.xy = float2(a0 + input.data.z / framesCount, input.data.w);
    output.texCoords.zw = float2(a1 + input.data.z / framesCount, input.data.w);
    output.frameDiff = (angle - a0) * framesCount;
    output.position = output.projectedPosition;
    
    return output;
}
