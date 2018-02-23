vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
};

[material][a] property float2 srcRectOffset;
[material][a] property float2 srcRectSize;
[material][a] property float2 srcTexSize;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    output.varTexCoord0 = input.position.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.position = float4(input.position.xy, 1.0, 1.0);
    return output;
}