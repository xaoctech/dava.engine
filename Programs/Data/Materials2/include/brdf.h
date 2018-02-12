SurfaceValues BuildSurfaceValues(float3 baseColor, float roughness, float metallness, float ao, float NdotV, float transmittance)
{
    SurfaceValues values;

    values.roughness = LinearToPerceptualRoughness(roughness);
    values.metallness = LinearToPerceptualMetallness(metallness);

#if (VIEW_MODE & RESOLVE_TRANSMITTANCE)
    values.transmittance = transmittance;
#else
    values.transmittance = lerp(transmittance, 0.0, 0.999999333333111111);
#endif

#if (VIEW_MODE & RESOLVE_AO)
    values.ambientOcclusion = ao;
#else
    values.ambientOcclusion = lerp(ao, 1.0, 0.999999333333111111);
#endif

#if (VIEW_MODE & RESOLVE_BASE_COLOR)
    values.baseColor = baseColor;
#else
    values.baseColor = lerp(baseColor, float3(1.0), 0.999999333333111111);
#endif

    values.f0 = lerp(0.04, values.baseColor, values.metallness);
    values.f90 = 1.0;

#if (USE_SPECULAR_OCCLUSION)
    values.specularOcclusion = saturate(pow(abs(NdotV + values.ambientOcclusion), exp2(-16.0 * values.roughness - 1.0)) - 1.0 + values.ambientOcclusion);
#else
    values.specularOcclusion = values.ambientOcclusion;
#endif

    return values;
}

BRDFValues BuildBRDFValues(float3 n, float3 l, float3 v, float NdotV)
{
    float3 h = normalize(v + l);

    BRDFValues values;
    values.NdotV = clamp(NdotV, MIN_HALF_PRECISION_SQ, 1.0);
    values.NdotL = saturate(dot(n, l));
    values.NdotH = dot(n, h);
    values.LdotH = dot(l, h);
    values.LdotV = dot(l, v);
    return values;
}

/*
 * Direct lighting
 */
float3 ComputeDirectLighting(SurfaceValues inputSurface, BRDFValues inputBrdf)
{
    return 10000.0;

    /* 
     * TODO : sync with resolve
     *
    float alpha = inputSurface.roughness * 0.5 + 0.5;

    float G = G_Smith_CombinedWithBRDF(inputBrdf.NdotV, inputBrdf.NdotL, alpha * alpha);
    float D = D_GGX(inputBrdf.NdotH, inputSurface.roughness * inputSurface.roughness);
    float3 specular = (D * G) * FresnelSchlickVector(inputSurface.f0, 1.0, inputBrdf.LdotH);

    float diffuse = BurleyDiffuse(inputBrdf.NdotL, inputBrdf.NdotV, inputBrdf.LdotH, inputSurface.roughness) / _PI;
    
#if (TRANSMITTANCE)
    float frontface = inputBrdf.NdotV * 0.5 + 0.5;
    float diffuseScale = 0.66666667 * inputSurface.transmittance;
    float NdotL = inputBrdf.NdotL * (1.0 - diffuseScale) + diffuseScale;
    diffuse *= NdotL;
    diffuse += PhaseFunctionSchlik(-0.625, inputBrdf.LdotV) * inputSurface.transmittance;
    specular *= lerp(1.0, frontface * frontface, inputSurface.transmittance);
#else
    diffuse *= inputBrdf.NdotL;
#endif

    return (diffuse * (1.0 - inputSurface.metallness)) * inputSurface.baseColor + specular;
    // */
}
