float SampleHeight8888Morphed(float2 uv, float morph, float lod)
{
    float lodSize = heightmapSize.x / pow(2.0, lod);
    float texelSize = 1.0 / lodSize;

    float4 sample00 = tex2Dlod(heightmap, uv, lod);
    float4 sample10 = tex2Dlod(heightmap, uv + float2(texelSize, 0.0), lod);
    float4 sample01 = tex2Dlod(heightmap, uv + float2(0.0, texelSize), lod);
    float4 sample11 = tex2Dlod(heightmap, uv + float2(texelSize, texelSize), lod);

    float2 h00 = float2(dot(sample00.xy, float2(0.0038910506, 0.9961089494)), dot(sample00.zw, float2(0.0038910506, 0.9961089494)));
    float2 h10 = float2(dot(sample10.xy, float2(0.0038910506, 0.9961089494)), dot(sample10.zw, float2(0.0038910506, 0.9961089494)));
    float2 h01 = float2(dot(sample01.xy, float2(0.0038910506, 0.9961089494)), dot(sample01.zw, float2(0.0038910506, 0.9961089494)));
    float2 h11 = float2(dot(sample11.xy, float2(0.0038910506, 0.9961089494)), dot(sample11.zw, float2(0.0038910506, 0.9961089494)));

    float2 bilinearFactor = frac(uv * lodSize);
    float2 linear0 = lerp(h00, h10, bilinearFactor.x);
    float2 linear1 = lerp(h01, h11, bilinearFactor.x);
    float2 bilinear = lerp(linear0, linear1, bilinearFactor.y);
    return lerp(bilinear.y, bilinear.x, morph);
}

float SampleHeight8888Accurate(float2 uv)
{
    float texelSize = 1.0 / heightmapSize.x;

    float4 sample00 = tex2Dlod(heightmap, uv, 0.0);
    float4 sample10 = tex2Dlod(heightmap, uv + float2(texelSize, 0.0), 0.0);
    float4 sample01 = tex2Dlod(heightmap, uv + float2(0.0, texelSize), 0.0);
    float4 sample11 = tex2Dlod(heightmap, uv + float2(texelSize, texelSize), 0.0);

    float h00 = dot(sample00.xy, float2(0.0038910506, 0.9961089494));
    float h10 = dot(sample10.xy, float2(0.0038910506, 0.9961089494));
    float h01 = dot(sample01.xy, float2(0.0038910506, 0.9961089494));
    float h11 = dot(sample11.xy, float2(0.0038910506, 0.9961089494));

    float2 bilinearFactor = frac(uv * heightmapSize.x);
    float linear0 = lerp(h00, h10, bilinearFactor.x);
    float linear1 = lerp(h01, h11, bilinearFactor.x);
    return lerp(linear0, linear1, bilinearFactor.y);
}

float2 SampleTangent8888Morphed(float2 uv, float morph, float lod)
{
    float halfTexel = 0.5 * pow(2.0, lod - heightmapSize.y);
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, lod);
    return lerp(sample0.zw, sample0.xy, morph);
}

float2 SampleTangent8888Accurate(float2 uv)
{
    float halfTexel = 0.5 / heightmapSize.x;
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, 0.0);
    return sample0.xy;
}

float SampleHeight4444(float2 uv)
{
    float texelSize = 1.0 / heightmapSize.x;

    float4 sample00 = tex2Dlod(heightmap, uv, 0.0);
    float4 sample10 = tex2Dlod(heightmap, uv + float2(texelSize, 0.0), 0.0);
    float4 sample01 = tex2Dlod(heightmap, uv + float2(0.0, texelSize), 0.0);
    float4 sample11 = tex2Dlod(heightmap, uv + float2(texelSize, texelSize), 0.0);

    float h00 = dot(sample00, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h10 = dot(sample10, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h01 = dot(sample01, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h11 = dot(sample11, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));

    float2 bilinearFactor = frac(uv * heightmapSize.x);
    float linear0 = lerp(h00, h10, bilinearFactor.x);
    float linear1 = lerp(h01, h11, bilinearFactor.x);
    return lerp(linear0, linear1, bilinearFactor.y);
}

float2 SampleTangent4444(float2 uv)
{
    float halfTexel = 0.5 / heightmapSize.x;
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, 0.0);
    return float2(dot(sample0.xy, float2(0.0588235294, 0.9411764706)), dot(sample0.zw, float2(0.0588235294, 0.9411764706))); //4444-format to 88
}