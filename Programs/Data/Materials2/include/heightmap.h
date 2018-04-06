float SampleHeightMorphed(float2 uv, float morph, float lod)
{

#if HEIGHTMAP_INTERPOLATION

    float lodSize = heightmapSize.x / pow(2.0, lod);
    float texelSize = 1.0 / lodSize;

    float4 sample00 = tex2Dlod(heightmap, uv, lod);
    float4 sample10 = tex2Dlod(heightmap, uv + float2(texelSize, 0.0), lod);
    float4 sample01 = tex2Dlod(heightmap, uv + float2(0.0, texelSize), lod);
    float4 sample11 = tex2Dlod(heightmap, uv + float2(texelSize, texelSize), lod);

    #if HEIGHTMAP_FORMAT == 0 //RGBA8888

    float2 h00 = float2(dot(sample00.xy, float2(0.0038910506, 0.9961089494)), dot(sample00.zw, float2(0.0038910506, 0.9961089494)));
    float2 h10 = float2(dot(sample10.xy, float2(0.0038910506, 0.9961089494)), dot(sample10.zw, float2(0.0038910506, 0.9961089494)));
    float2 h01 = float2(dot(sample01.xy, float2(0.0038910506, 0.9961089494)), dot(sample01.zw, float2(0.0038910506, 0.9961089494)));
    float2 h11 = float2(dot(sample11.xy, float2(0.0038910506, 0.9961089494)), dot(sample11.zw, float2(0.0038910506, 0.9961089494)));

    #elif HEIGHTMAP_FORMAT == 6 //RG1616

    float2 h00 = sample00.xy;
    float2 h10 = sample10.xy;
    float2 h01 = sample01.xy;
    float2 h11 = sample11.xy;

    #elif HEIGHTMAP_FORMAT == 5 //RGBA4444

    float2 h00 = float2(dot(sample00, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053)), 0.0);
    float2 h10 = float2(dot(sample10, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053)), 0.0);
    float2 h01 = float2(dot(sample01, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053)), 0.0);
    float2 h11 = float2(dot(sample11, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053)), 0.0);
    morph = 1.0;

    #endif

    float2 bilinearFactor = frac(uv * lodSize);
    float2 linear0 = lerp(h00, h10, bilinearFactor.x);
    float2 linear1 = lerp(h01, h11, bilinearFactor.x);
    float2 bilinear = lerp(linear0, linear1, bilinearFactor.y);
    return lerp(bilinear.y, bilinear.x, morph);

#else

    #if HEIGHTMAP_FORMAT == 0 //RGBA8888

    float samplePixelOffset = 0.5 * pow(2.0, lod - heightmapSize.y);
    float4 sample0 = tex2Dlod(heightmap, float2(uv + samplePixelOffset), lod);
    float2 sampleMorphed = lerp(sample0.zw, sample0.xy, morph);
    return dot(sampleMorphed, float2(0.0038910506, 0.9961089494));

    #elif HEIGHTMAP_FORMAT == 6 //RG1616

    float samplePixelOffset = 0.5 * pow(2.0, lod - heightmapSize.y);
    float4 sample0 = tex2Dlod(heightmap, float2(uv + samplePixelOffset), lod);
    return lerp(sample0.g, sample0.r, morph);

    #elif HEIGHTMAP_FORMAT == 5 //RGBA4444

    float4 sample0 = tex2Dlod(heightmap, float2(uv + 0.5 / heightmapSize.x), 0.0);
    return dot(sample0, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));

    #endif

#endif
}

float SampleHeightAccurate(float2 uv)
{

#if HEIGHTMAP_INTERPOLATION

    float texelSize = 1.0 / heightmapSize.x;

    float4 sample00 = tex2Dlod(heightmap, uv, 0.0);
    float4 sample10 = tex2Dlod(heightmap, uv + float2(texelSize, 0.0), 0.0);
    float4 sample01 = tex2Dlod(heightmap, uv + float2(0.0, texelSize), 0.0);
    float4 sample11 = tex2Dlod(heightmap, uv + float2(texelSize, texelSize), 0.0);

    #if HEIGHTMAP_FORMAT == 0 //RGBA8888

    float h00 = dot(sample00.xy, float2(0.0038910506, 0.9961089494));
    float h10 = dot(sample10.xy, float2(0.0038910506, 0.9961089494));
    float h01 = dot(sample01.xy, float2(0.0038910506, 0.9961089494));
    float h11 = dot(sample11.xy, float2(0.0038910506, 0.9961089494));

    #elif HEIGHTMAP_FORMAT == 6 //RG1616

    float h00 = sample00.x;
    float h10 = sample10.x;
    float h01 = sample01.x;
    float h11 = sample11.x;

    #elif HEIGHTMAP_FORMAT == 5 //RGBA4444

    float h00 = dot(sample00, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h10 = dot(sample10, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h01 = dot(sample01, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));
    float h11 = dot(sample11, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));

    #endif

    float2 bilinearFactor = frac(uv * heightmapSize.x);
    float linear0 = lerp(h00, h10, bilinearFactor.x);
    float linear1 = lerp(h01, h11, bilinearFactor.x);
    return lerp(linear0, linear1, bilinearFactor.y);

#else

    float4 sample0 = tex2Dlod(heightmap, float2(uv + 0.5 / heightmapSize.x), 0.0);

    #if HEIGHTMAP_FORMAT == 0 //RGBA8888

    return dot(sample0.xy, float2(0.0038910506, 0.9961089494));

    #elif HEIGHTMAP_FORMAT == 6 //RG1616

    return sample0.r;

    #elif HEIGHTMAP_FORMAT == 5 //RGBA4444

    return dot(sample0, float4(0.0002288853, 0.0036621653, 0.0585946441, 0.9375143053));

    #endif

#endif
}

float2 SampleTangentMorphed(float2 uv, float morph, float lod)
{
#if HEIGHTMAP_FORMAT == 5 //RGBA4444

    float halfTexel = 0.5 / heightmapSize.x;
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, 0.0);
    return float2(dot(sample0.xy, float2(0.0588235294, 0.9411764706)), dot(sample0.zw, float2(0.0588235294, 0.9411764706))); //4444-format to 88

#else //RGBA8888

    float halfTexel = 0.5 * pow(2.0, lod - heightmapSize.y);
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, lod);
    return lerp(sample0.zw, sample0.xy, morph);

#endif
}

float2 SampleTangentAccurate(float2 uv)
{

#if HEIGHTMAP_FORMAT == 5 //RGBA4444

    float halfTexel = 0.5 / heightmapSize.x;
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, 0.0);
    return float2(dot(sample0.xy, float2(0.0588235294, 0.9411764706)), dot(sample0.zw, float2(0.0588235294, 0.9411764706))); //4444-format to 88

#else //RGBA8888

    float halfTexel = 0.5 / heightmapSize.x;
    float4 sample0 = tex2Dlod(tangentmap, uv + halfTexel, 0.0);
    return sample0.xy;

#endif
}
