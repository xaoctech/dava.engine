{
#if (VERTEX_BAKED_AO)
    bakedAo *= input.vertexBakedAO;
#endif

    float bakedShadow = 1.0;
    
#if (USE_BAKED_LIGHTING)
    float2 bakedShadowAOSample = tex2D(shadowaotexture, input.varTexCoord.zw).xy;
    bakedShadow *= bakedShadowAOSample.x;
    bakedAo *= bakedShadowAOSample.y;
#endif

    float3 environmentDiffuseSample = 0.0;

#if (IB_REFLECTIONS_PREPARE == 0)
    environmentDiffuseSample = sphericalHarmonics[0].xyz * (0.282095);
#endif

    float LdotN = max(0.0, dot(resolve.n, lightPosition0.xyz));
    
#if (TRANSMITTANCE)
    LdotN = LdotN * 0.75 + 0.25;
#endif

    float3 result = baseColorSample.xyz * (bakedShadow * LdotN + bakedAo * environmentDiffuseSample);

    output.color = float4(result, 1.0);
}
