vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

[auto][global] property float2 viewportSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 renderTargetSize;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 in_pos = input.position.xyz;
    float4 inPosition = float4(in_pos, 1.0);
    output.position = inPosition;

    float2 texPos = in_pos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.texCoord = (texPos * viewportSize + viewportOffset) / renderTargetSize;

    return output;
}
