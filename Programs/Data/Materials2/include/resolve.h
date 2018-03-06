#if (PARALLAX_CORRECTED)
float3 GetIBLSpecularReflection(float3 reflectedDirection, ResolveInputValues input, float sampledLod)
{
    float3 globalColor = texCUBElod(globalReflection, reflectedDirection, sampledLod).xyz;
    float3 reflectedDirectionInLocalSpace = mul(float4(reflectedDirection, 0.0), input.localProbeCaptureWorldToLocalMatrix).xyz;
    float3 positionInLocalSpace = mul(float4(input.worldPosition, 1.0), input.localProbeCaptureWorldToLocalMatrix).xyz;
    float3 unitSize = float3(1.0f, 1.0f, 1.0f);
    float3 firstPlaneIntersect = (unitSize - positionInLocalSpace) / reflectedDirectionInLocalSpace;
    float3 secondPlaneIntersect = (-unitSize - positionInLocalSpace) / reflectedDirectionInLocalSpace;
    float3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);
    float distanceToPlane = min(furthestPlane.x, min(furthestPlane.y, furthestPlane.z));
    // Use distance in world space directly to recover intersection
    float3 intersectionPositionInWorldSpace = input.worldPosition + reflectedDirection.xyz * distanceToPlane;
    float3 parallaxDirectionInWorldSpace = (intersectionPositionInWorldSpace - input.localProbeCapturePositionInWorldSpace);
    float bz = 1.0 - PARALLAX_BLEND_ZONE;
    float3 posAbs = clamp(abs(positionInLocalSpace), float3(bz, bz, bz), float3(1.0, 1.0, 1.0));
    float blendFactor = max(max(posAbs.x, posAbs.y), posAbs.z);
    float blend = lerp(1.0, 0.0, (blendFactor - (1.0 - PARALLAX_BLEND_ZONE)) / PARALLAX_BLEND_ZONE);
    float3 reflectedColorLocal = texCUBElod(localReflection, parallaxDirectionInWorldSpace, 0.0).xyz;
    return lerp(globalColor, reflectedColorLocal, blend);
}
#endif

float3 ResolveFinalColor(ResolveInputValues input, SurfaceValues surfaceParameters, ShadowParameters shadowParameters)
{
    float specularSampleLod = input.roughness * 7.0;

#if (IB_REFLECTIONS_PREPARE)
    float3 environmentSpecularSample = 0.0;
#elif (USE_DOMINANT_DIRECTION)
    float3 specularDominantDirection = GetSpecularDominantDirection(input.n, -reflect(input.v, input.n), surfaceParameters.roughness);
    float3 environmentSpecularSample = DecodeRGBM(texCUBElod(globalReflection, specularDominantDirection, specularSampleLod));
#else
    float3 environmentSpecularSample = DecodeRGBM(texCUBElod(globalReflection, float3(-reflect(input.v, input.n)), specularSampleLod));
#endif

    float transmittanceEnabled = step(EPSILON, surfaceParameters.transmittance);
    shadowParameters.filterRadius.y *= transmittanceEnabled;
    float shadowValue = SampleDirectionalShadow(input.directionalLightViewSpaceCoords, shadowParameters);

    BRDFValues directionalBrdf = BuildBRDFValues(input.n, input.directionalLightDirection, input.v, input.NdotV);
    float invMetallness = 1.0 - surfaceParameters.metallness;

    float3 result = 0.0;
    float3 directLighting = 0.0;
    {
        float alpha = (surfaceParameters.roughness * 0.5 + 0.5);

        float G = G_Smith_CombinedWithBRDF(directionalBrdf.NdotV, directionalBrdf.NdotL, alpha * alpha);
        float D = D_GGX(directionalBrdf.NdotH, surfaceParameters.roughness * surfaceParameters.roughness);
        float T = lerp(1.0, directionalBrdf.NdotV, transmittanceEnabled);
        float3 F = FresnelSchlickVector(surfaceParameters.f0, surfaceParameters.f90, directionalBrdf.LdotH);
        float3 specular = F * (D * G * T);

        float NdotL = lerp(directionalBrdf.NdotL, 1.0, 0.5 * surfaceParameters.transmittance);
        float diffuse = (NdotL + PhaseFunctionSchlick(directionalBrdf.LdotV, 0.625) * surfaceParameters.transmittance) * invMetallness / _PI;
        
    #if (DIFFUSE_BURLEY)
        diffuse *= BurleyDiffuse(NdotL, directionalBrdf.NdotV, directionalBrdf.LdotH, surfaceParameters.roughness);
    #endif
       
#if (VIEW_MODE & RESOLVE_DIFFUSE)
        directLighting += surfaceParameters.baseColor * diffuse;
#endif

#if (VIEW_MODE & RESOLVE_SPECULAR)
        directLighting += specular;
#endif

        directLighting *= input.directionalLightColor * (shadowValue * input.directionalLightStaticShadow);
    }
    
#if (VIEW_MODE & RESOLVE_DIRECT_LIGHT)
    result += directLighting;
#else
    result = lerp(result, directLighting, MIN_HALF_PRECISION);
#endif

    float3 indirectLighting = 0.0;
    {
    #if (USE_DFG_APPROXIMATION)
        float4 brdfLookup = AnalyticDFGApproximation(directionalBrdf.NdotV, input.roughness /* approximation require roughness not to be squared */);
    #else
        float4 brdfLookup = tex2D(indirectSpecularLookup, float2(directionalBrdf.NdotV, surfaceParameters.roughness));
    #endif

        float indirectDiffuseScale = brdfLookup.z * invMetallness * (surfaceParameters.ambientOcclusion + surfaceParameters.transmittance);
        float3 indirectDiffuse = input.environmentDiffuseSample * surfaceParameters.baseColor * indirectDiffuseScale;
        float3 indirectSpecular = environmentSpecularSample * (surfaceParameters.f0 * brdfLookup.x + surfaceParameters.f90 * brdfLookup.y);

        /*
         * WARNING : temporal solution for supressing specular highlights on leafs
         * WARNING : should be removed when screen-space reflections will exists
         * WARNING : ^_^
         */
        indirectSpecular *= lerp(1.0, surfaceParameters.baseColor, step(EPSILON, surfaceParameters.transmittance));

    #if (VIEW_MODE & RESOLVE_DIFFUSE)
        indirectLighting += indirectDiffuse;
    #endif

    #if (VIEW_MODE & RESOLVE_SPECULAR)
        indirectLighting += indirectSpecular * surfaceParameters.specularOcclusion;
    #endif
    }
    
#if (VIEW_MODE & RESOLVE_IBL_LIGHT)
    result += indirectLighting;
#else
    result = lerp(result, indirectLighting, MIN_HALF_PRECISION);
#endif
    
#if (ENABLE_ATMOSPHERE_SCATTERING)
    {
        float lightIntensity = dot(input.directionalLightColor, float3(0.2126, 0.7152, 0.0722));
        float3 viewDirection = -input.v;
        float3 lightDirection = input.directionalLightDirection;
        float3 planetPosition = float3(0.0, 0.0, EARTH_RADIUS);
        float3 pOrigin = planetPosition + input.cameraPosition;
        float3 pTarget = pOrigin + input.v * input.vLength * fogParameters.x;
        float2 ph = ScatteringPhaseFunctions(viewDirection, lightDirection, fogParameters.z);

        float3 inScattering = InScattering(pOrigin, pTarget, lightDirection, ph, fogParameters.y);
        float3 outScattering = OutScattering(pOrigin, pTarget, lightDirection, ph, fogParameters.y);

        result = result * outScattering + inScattering * lightIntensity;
    }
#endif
    
#if (VIEW_MODE & VIEW_BY_COMPONENT_BIT)

    float3 viewValue = 0.0;
    {
#if ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_BASE_COLOR)
        viewValue = input.baseColor;
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_ROUGHNESS)
        viewValue = surfaceParameters.roughness;
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_METALLNESS)
        viewValue = input.metallness;
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_AO)
        viewValue = input.ambientOcclusion;
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_STATIC_SHADOWS)
        viewValue = float3(input.directionalLightStaticShadow, shadowValue, 1.0);
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_NORMALS)
        viewValue = input.n * 0.5 + 0.5;
#elif ((VIEW_MODE & VIEW_BY_COMPONENT_MASK) == VIEW_TRANSMITTANCE)
        viewValue = surfaceParameters.transmittance;
#endif
    }
    result = lerp(result, viewValue, 0.999999333333111111);
    
#endif

    /*
    if (isNan(result.x) || isNan(result.y) || isNan(result.z))
        result.xyz = input.directionalLightColor * float3(100.0, 0.0, 100.0);
    */

    return float3(result);
}

#if (ENABLE_POINT_LIGHTS) || (ENABLE_DEFERRED_LIGHTS)

float3 ComputePointLight(float4 lightPosition, float4 lightColor, ResolveInputValues resolveInput, SurfaceValues surfaceInput)
{
    float lightRadius = lightPosition.w;

    float3 directionToLight = float3(float3(lightPosition.xyz) - resolveInput.worldPosition);
    float distanceToLight = length(directionToLight);
    directionToLight /= distanceToLight;

    float3 result = float3(0.0, 0.0, 1000.0);
    if (distanceToLight <= lightRadius)
    {
        BRDFValues pointBrdf = BuildBRDFValues(resolveInput.n, directionToLight, resolveInput.v, resolveInput.NdotV);
        float shadow = 1.0;
        float attenuation = shadow * DistanceAttenuation(lightRadius, distanceToLight);
        float3 lighting = ComputeDirectLighting(surfaceInput, pointBrdf);
        result = lighting * lightColor.xyz * attenuation;
    }

    return result;
}

#endif
