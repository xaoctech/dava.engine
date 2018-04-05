#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"
#include "include/heightmap.h"

vertex_in
{
    [vertex] float3 position : POSITION;
    [vertex] float4 texCoord0 : TEXCOORD0; //uv, tint-value, random-radius
    [vertex] float4 pivot : TEXCOORD1; //patch-space [0...1]
    [vertex] float3 normal : NORMAL;
    [vertex] float3 tangent : TANGENT;
    [vertex] float3 binormal : BINORMAL;

    [instance] float4 patchOffsetScale : TEXCOORD3; //patch-offset, patch-scale, decor-scale
    [instance] float4 decorPageCoords : TEXCOORD4;
    [instance] float3 flipRotate : TEXCOORD5;
    [instance] float2 randomRotate : TEXCOORD6;
};

vertex_out
{
    float4 position : SV_POSITION;

    float4 uv : TEXCOORD0;
    float3 tbnToWorld0 : TEXCOORD1;
    float3 tbnToWorld1 : TEXCOORD2;
    float4 tbnToWorld2 : TEXCOORD3;
    float4 projectedPosition : TEXCOORD4;
#if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
#endif
#if (VERTEX_COLOR)
    float4 vertexColor : COLOR1;
#endif
};

uniform sampler2D decorationtexture;

#if VERTEX_COLOR
uniform sampler2D decorationcolortexture;
#endif

[auto][a] property float3 boundingBoxSize;

[material][instance] property float decorationindex = 0.0;

#if ORIENT_ON_LANDSCAPE
[material][instance] property float orientvalue = 0.0;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 position = input.position.xyz;
    float3 normal = input.normal.xyz;
    float3 tangent = input.tangent.xyz;
    float3 binormal = input.binormal.xyz;

    float2 pivot = input.pivot.xy;
    float2 circleCenter = input.pivot.zw;
    float2 patchOffset = input.patchOffsetScale.xy;
    float patchScale = input.patchOffsetScale.z;
    float decorScale = input.patchOffsetScale.w;

    float tintValue = input.texCoord0.z;

#if DECORATION_GPU_RANDOMIZATION == 1
    pivot = Random(patchOffset + pivot);
#elif DECORATION_GPU_RANDOMIZATION == 2
    float randomRadius = input.texCoord0.w;
    pivot = circleCenter + RandomCircle(patchOffset + pivot) * randomRadius;
#endif

    //patch flip-rotate
    float3 flipRotate = input.flipRotate.xyz;

    pivot = pivot * 2.0 - float2(1.0, 1.0);
    pivot = RotateXY(pivot, flipRotate.xy);
    pivot.y *= flipRotate.z;
    pivot = pivot * 0.5 + float2(0.5, 0.5);

    //rotation random
    float2 randomRotate = input.randomRotate; //[sin, cos]
    position.xy = RotateXY(position.xy, randomRotate.xy);
    normal.xy = RotateXY(normal.xy, randomRotate.xy);
    tangent.xy = RotateXY(tangent.xy, randomRotate.xy);
    binormal.xy = RotateXY(binormal.xy, randomRotate.xy);

    float2 relativePosition = patchOffset + pivot * patchScale; //landscape-space [0.0, 1.0]
        
#if ORIENT_ON_LANDSCAPE
    {
        float2 nxy = SampleTangentAccurate(relativePosition);

        nxy = 2.0 * nxy - 1.0;
        nxy *= orientvalue;
        float3 normal = float3(nxy, sqrt(1.0 - dot(nxy, nxy)));
        float3 tangent = cross(float3(0.0, 1.0, 0.0), normal);
        float3 binormal = cross(normal, tangent);

        float3x3 rotation = float3x3(tangent, binormal, normal);
        position = mul(position, rotation);
    }
#endif

    float height = SampleHeightAccurate(relativePosition);

    float2 decorTexCoord = input.decorPageCoords.xy + pivot * input.decorPageCoords.zw;
    float4 decarationSample = tex2Dlod(decorationtexture, decorTexCoord, 0.0);

    int decorationIndex = int(decorationindex);
    int decorationMaskIndex = int(decarationSample.r * 256.0 + 0.5);
    float decoration = ((decorationIndex == decorationMaskIndex) ? 1.0 : 0.0) * decarationSample.g;

    float3 pivotObjectSpace = float3(relativePosition - 0.5, height) * boundingBoxSize;
    float3 vx_position = position * decoration * decorScale + pivotObjectSpace;

    output.position = mul(float4(vx_position.x, vx_position.y, vx_position.z, 1.0), worldViewProjMatrix);
    output.projectedPosition = output.position;
    output.uv = float4(input.texCoord0.xy, relativePosition.x, 1.0 - relativePosition.y);
    
#if VERTEX_COLOR
    float4 albedoSample = tex2Dlod(decorationcolortexture, float2(relativePosition.x, 1.0 - relativePosition.y), 0.0);
    output.vertexColor = float4(albedoSample.rgb, tintValue);
#endif

    float3 worldNormal = normalize(mul(float4(normal, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldTangent = normalize(mul(float4(tangent, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldBinormal = normalize(mul(float4(binormal, 0.0), worldInvTransposeMatrix).xyz); 
    
#if FLIP_BACKFACE_NORMALS
    float4 worldSpacePosition = mul(float4(vx_position.xyz, 1.0), worldMatrix);
    float3 toCamera = cameraPosition - worldSpacePosition.xyz;
    float normalsScale = sign(dot(worldNormal, toCamera));
    worldNormal *= normalsScale;
    worldTangent *= normalsScale;
    worldBinormal *= normalsScale;
#endif

    output.tbnToWorld0 = float3(worldTangent.x, worldBinormal.x, worldNormal.x);
    output.tbnToWorld1 = float3(worldTangent.y, worldBinormal.y, worldNormal.y);
    output.tbnToWorld2.xyz = float3(worldTangent.z, worldBinormal.z, worldNormal.z);
    output.tbnToWorld2.w = 1.0;

    return output;
}
