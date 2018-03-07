#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
    float4 texCoordTop : TEXCOORD1;
    float4 texCoordBottom : TEXCOORD2;
    float2 uniformTexCoords : TEXCOORD3;
#if (TECH_COMBINE || TECH_BLOOM_THRESHOLD)
    float luminanceHistoryValue : COLOR0;
#endif

#if (ENABLE_TXAA)
    float4 texClmp : TEXCOORD4;
#endif
};

#if (TECH_COMBINE || TECH_BLOOM_THRESHOLD)
uniform sampler2D luminanceHistoryTexture;
#endif

[material][a] property float2 srcRectOffset;
[material][a] property float2 srcRectSize;
[material][a] property float2 srcTexSize;
[material][a] property float2 destRectOffset;
[material][a] property float2 destRectSize;
[material][a] property float2 destTexSize;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float2 texPos = input.position.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.uniformTexCoords = texPos;
    output.varTexCoord0 = (texPos * srcRectSize + srcRectOffset + centerPixelMapping) / srcTexSize;

#if (ENABLE_TXAA)
    float destTexSizeInv = 1.0f / destTexSize.x;
    float centerPixelMappingMul = centerPixelMapping.x * destTexSizeInv;
    output.texClmp.xy = destRectOffset * destTexSizeInv + centerPixelMappingMul;
    output.texClmp.zw = (destRectSize + destRectOffset) * destTexSizeInv + centerPixelMappingMul;
#endif

    output.position = float4(input.position.xy, 1.0, 1.0);

    float2 offset = 0.5 / srcTexSize;
    output.texCoordTop.xy = output.varTexCoord0 + float2(-offset.x, -offset.y);
    output.texCoordTop.zw = output.varTexCoord0 + float2(+offset.x, -offset.y);
    output.texCoordBottom.xy = output.varTexCoord0 + float2(-offset.x, +offset.y);
    output.texCoordBottom.zw = output.varTexCoord0 + float2(+offset.x, +offset.y);

#if TECH_COMBINE
    output.position.y *= projectionFlipped;
#endif
    
#if (TECH_COMBINE || TECH_BLOOM_THRESHOLD)
    output.luminanceHistoryValue = tex2Dlod(luminanceHistoryTexture, float2(0.5, 0.5), 0.0).x;
#endif

    return output;
}
