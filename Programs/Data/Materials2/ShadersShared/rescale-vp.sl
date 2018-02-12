vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[auto][global] property float2 viewportSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 renderTargetSize;
[auto][global] property float projectionFlipped;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 in_pos = input.position.xyz;
    output.position = float4(in_pos, 1.0);

    float2 texPos = in_pos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.uv = (texPos * viewportSize + centerPixelMapping + viewportOffset) / renderTargetSize;

    output.position.y *= -projectionFlipped; //GFX_COMPLETE - still note sure why, but in this case it corresponds to depthbuffer

    return output;
}
