#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/heightmap.h"

#if LANDSCAPE_USE_INSTANCING

vertex_in
{
    [vertex] float4 data0 : POSITION; // position + edgeShiftDirection
    [vertex] float4 data1 : NORMAL; // edge mask
    [vertex] float4 data2 : TANGENT; // edgeVertexIndex + fence + avgShift

    [instance] float4 data3 : TEXCOORD0; // patch position + scale + texture-page blend
    [instance] float4 data4 : TEXCOORD1; // neighbour patch lodOffset
    [instance] float4 data5 : TEXCOORD2; // texture coords offset + scale for vt-page0
    [instance] float4 data6 : TEXCOORD3; // texture coords offset + scale for vt-page1
    [instance] float4 data7 : TEXCOORD4; // neighbour page blend
    
#if LANDSCAPE_LOD_MORPHING
    [instance] float4 data8 : TEXCOORD5; // neighbour patch morph
    [instance] float2 data9 : TEXCOORD6; // patch lod + morph
#endif
};

#else

vertex_in
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

#endif

vertex_out
{
    float4 pos : SV_POSITION;
    
#if LANDSCAPE_USE_INSTANCING
    float3 normal : NORMAL0;
    float4 texCoord0 : TEXCOORD0;
    float3 texCoord1 : TEXCOORD1;
    float4 shadowTexCoord : TEXCOORD2;
    float3 varToCamera : TEXCOORD3;
#else
    float4 texCoord0 : TEXCOORD0;
#endif
};

#if LANDSCAPE_USE_INSTANCING
[auto][a] property float3 boundingBoxSize;
[auto][a] property float tessellationHeight;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    
#if LANDSCAPE_USE_INSTANCING

    float2 in_pos = input.data0.xy;
    float2 edgeShiftDirection = input.data0.zw;
    float4 edgeMask = input.data1;
    float edgeVertexIndex = input.data2.x;

    float3 patchOffsetScale = input.data3.xyz;
    float4 neighbourPatchLodOffset = input.data4;

    float edgeMaskNull = 1.0 - dot(edgeMask, float4(1.0, 1.0, 1.0, 1.0));

    //Calculate vertecies offset for fusing neighboring patches
    float lodOffset = dot(edgeMask, neighbourPatchLodOffset);
    float edgeShift = fmod(edgeVertexIndex, pow(2.0, lodOffset));
    in_pos += edgeShiftDirection * edgeShift;

    float2 relativePosition = patchOffsetScale.xy + in_pos.xy * patchOffsetScale.z; //[0.0, 1.0]

    float morphAmount = 0.0;
    float sampleLod = 0.0;
    
#if LANDSCAPE_LOD_MORPHING

    float4 neighbourPatchMorph = input.data8;

    float baseLod = input.data9.x;
    float patchMorph = input.data9.y;

    // Calculate 'zero-multiplier' that provide fetch zero-mip for vertecies at the edges with climbs beyound height-texture.
    float2 zeroLod = step(1.0, relativePosition);
    float zeroLodMul = 1.0 - min(1.0, zeroLod.x + zeroLod.y);

    // Calculate fetch parameters
    sampleLod = max((baseLod + lodOffset) * zeroLodMul, 0.0);
    morphAmount = dot(edgeMask, neighbourPatchMorph) + patchMorph * edgeMaskNull;
    
#endif

    float height = SampleHeightMorphed(relativePosition, morphAmount, sampleLod);
    float2 nxy = SampleTangentMorphed(relativePosition, morphAmount, sampleLod);

    output.texCoord0.xy = input.data5.xy + in_pos.xy * input.data5.zw;
    output.texCoord0.zw = input.data6.xy + in_pos.xy * input.data6.zw;
    output.texCoord1.xy = float2(relativePosition.x, 1.0 - relativePosition.y);
    output.texCoord1.z = input.data3.w;

    height -= input.data2.y * 0.05 / boundingBoxSize.z; //fences

    nxy = nxy * 2.0 - 1.0;
    float3 normal = float3(nxy, sqrt(1.0 - saturate(dot(nxy, nxy))));

    float3 vx_position = float3(relativePosition - 0.5, height) * boundingBoxSize;
    output.pos = mul(float4(vx_position.x, vx_position.y, vx_position.z, 1.0), worldViewProjMatrix);

    float4 worldSpacePosition = mul(float4(vx_position.xyz, 1.0), worldMatrix);

    output.normal = normal;
    output.varToCamera = cameraPosition - worldSpacePosition.xyz;
    output.shadowTexCoord = mul(worldSpacePosition, shadowView);
    
#else

    float3 vx_position = input.pos.xyz;
    output.pos = mul(float4(vx_position.x, vx_position.y, vx_position.z, 1.0), worldViewProjMatrix);
    output.texCoord0 = input.uv.xyxy;
    
#endif

    return output;
}
