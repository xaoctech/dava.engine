#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/heightmap.h"

#if LANDSCAPE_USE_INSTANCING

vertex_in
{
    [vertex] float4 data0 : POSITION; // position + edgeShiftDirection
    [vertex] float4 data1 : NORMAL; // edge mask
    [vertex] float2 data2 : TANGENT; // edgeVertexIndex + edgeMaskNull

    [instance] float4 data3 : TEXCOORD0; // patch position + scale + texture-page blend
    [instance] float4 data4 : TEXCOORD1; // neighbour patch lodOffset
    [instance] float4 data5 : TEXCOORD2; // texture coords offset + scale for vt-page0
    [instance] float4 data6 : TEXCOORD3; // texture coords offset + scale for vt-page1

        #if LANDSCAPE_LOD_MORPHING
    [instance] float4 data7 : TEXCOORD4; // neighbour patch morph
    [instance] float2 data8 : TEXCOORD5; // patch lod + morph
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
    float4 projectedPosition : TEXCOORD0;
};

#if LANDSCAPE_USE_INSTANCING
[auto][a] property float3 boundingBoxSize;
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

    //Calculate vertecies offset for fusing neighboring patches
    float lodOffset = dot(edgeMask, neighbourPatchLodOffset);
    float edgeShiftAmount = pow(2.0, lodOffset);
    in_pos += edgeShiftDirection * fmod(edgeVertexIndex, edgeShiftAmount);

    float2 relativePosition = patchOffsetScale.xy + in_pos.xy * patchOffsetScale.z; //[0.0, 1.0]

    float height = SampleHeightAccurate(relativePosition);

    float3 vx_position = float3(relativePosition - 0.5, height) * boundingBoxSize;
    
#else

    float3 vx_position = input.pos.xyz;
    
#endif

    float4 worldSpacePosition = mul(float4(vx_position.x, vx_position.y, vx_position.z, 1.0), worldMatrix);
    output.projectedPosition = mul(worldSpacePosition, viewProjMatrix);
    output.pos = output.projectedPosition;

    return output;
}
