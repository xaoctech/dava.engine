fragment_in
{
    float2 uv : TEXCOORD0;
};

fragment_out
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

uniform sampler2D maskTexture;
uniform sampler2D buffer0Texture;
uniform sampler2D buffer1Texture;

[material][a] property float2 srcTexSize;

fragment_out fp_main(fragment_in input)
{
    float4 maskSample = tex2D(maskTexture, input.uv);
    float4 buf0Sample = tex2D(buffer0Texture, input.uv);
    float4 buf1Sample = tex2D(buffer1Texture, input.uv);

    float totalWeight = 0.0;
    float4 averageBuf0 = 0.0;
    float4 averageBuf1 = 0.0;

    int filterSize = 48;
    float maxDistance = length(float2(filterSize, filterSize));
    for (int v = -filterSize; v <= filterSize; ++v)
    {
        for (int u = -filterSize; u <= filterSize; ++u)
        {
            float2 offset = float2(float(u), float(v));
            float distanceWeight = pow(1.0 / max(1.0, length(offset)), 16.0);
            
            float maskValue = tex2D(maskTexture, input.uv + offset / srcTexSize.xy).x;
            float sampleWeight = maskValue * distanceWeight;
            
            totalWeight += sampleWeight;

            float4 buf0Smp = tex2D(buffer0Texture, input.uv + offset / srcTexSize.xy);
            averageBuf0 += buf0Smp * sampleWeight;
            
            float4 buf1Smp = tex2D(buffer1Texture, input.uv + offset / srcTexSize.xy);
            averageBuf1 += buf1Smp * sampleWeight;
        }
    }
    averageBuf0 = averageBuf0 / totalWeight;
    averageBuf1 = averageBuf1 / totalWeight;

    fragment_out output;
    {
        output.albedo.xyz = lerp(averageBuf0.xyz, buf0Sample.xyz, float(maskSample.x > 0.0));
        output.albedo.w = buf0Sample.w;
        
        output.normal = lerp(averageBuf1, buf1Sample, float(maskSample.x > 0.0));
    }
    return output;
}
