// #ensuredefined SHADOW_CASCADES          4.0
// #ensuredefined SHADOW_PCF               1

/*
 * Directional light implementation
 */
float SampleShadow(float4 lightSpaceCoords, float4 projectionScale, float4 projectionOffset, ShadowParameters sp)
{
    float shadow = 0.0;

#if (SHADOW_PCF)
    float3 poissonDistribution[8];
    poissonDistribution[0] = float3(+0.95581, -0.18159, sp.filterRadius.y * 0.0 / 7.0);
    poissonDistribution[1] = float3(+0.50147, -0.35807, sp.filterRadius.y * 1.0 / 7.0);
    poissonDistribution[2] = float3(+0.69607, +0.35559, sp.filterRadius.y * 2.0 / 7.0);
    poissonDistribution[3] = float3(-0.00368, -0.59150, sp.filterRadius.y * 3.0 / 7.0);
    poissonDistribution[4] = float3(+0.15930, +0.08975, sp.filterRadius.y * 4.0 / 7.0);
    poissonDistribution[5] = float3(-0.65030, +0.05818, sp.filterRadius.y * 5.0 / 7.0);
    poissonDistribution[6] = float3(+0.11915, +0.78449, sp.filterRadius.y * 6.0 / 7.0);
    poissonDistribution[7] = float3(-0.34296, +0.51575, sp.filterRadius.y * 7.0 / 7.0);

    float originalZ = lightSpaceCoords.z;
    float cosTheta = sp.rotationKernel.x * sp.filterRadius.x;
    float sinTheta = sp.rotationKernel.y * sp.filterRadius.x;
    float2 r0 = float2(cosTheta, -sinTheta) / sp.shadowMapSize.x;
    float2 r1 = float2(sinTheta, cosTheta) / sp.shadowMapSize.y;

    for (int i = 0; i < SHADOW_PCF; ++i)
    {
        lightSpaceCoords.z = originalZ + poissonDistribution[i].z;

        float4 projectedCoords = lightSpaceCoords * projectionScale + projectionOffset;
        projectedCoords.x += dot(poissonDistribution[i].xy, r0);
        projectedCoords.y += dot(poissonDistribution[i].xy, r1);

        shadow += FP_SHADOW(tex2Dcmp(directionalShadowMap, projectedCoords.xyz));
    }
    shadow *= 1.0 / float(SHADOW_PCF);

#else

    float4 projectedCoords = lightSpaceCoords * projectionScale + projectionOffset;
    shadow = FP_SHADOW(tex2Dcmp(directionalShadowMap, projectedCoords.xyz));

#endif

    return shadow;
}

float SampleDirectionalShadow(float4 lightViewSpaceCoords, ShadowParameters ds)
{
    float edgeBlendSize = 0.03125; /* blend edge size, equals to 32 pixels for 1024 shadow map */

    float shadow = 1.0;
    float outBlendAmount = 1.0;

#if (SHADOW_CASCADES == 0.0)

    shadow = 1.0;

#elif (SHADOW_CASCADES > 1.0)
    int cascade = -1;
    float4 allCascadeCoords[4];
    allCascadeCoords[0] = lightViewSpaceCoords * ds.cascadesProjectionScale[0] + ds.cascadesProjectionOffset[0];
    allCascadeCoords[1] = lightViewSpaceCoords * ds.cascadesProjectionScale[1] + ds.cascadesProjectionOffset[1];
    allCascadeCoords[2] = lightViewSpaceCoords * ds.cascadesProjectionScale[2] + ds.cascadesProjectionOffset[2];
    allCascadeCoords[3] = lightViewSpaceCoords * ds.cascadesProjectionScale[3] + ds.cascadesProjectionOffset[3];
    {
        float3 c0 = allCascadeCoords[0].xyz;
        float3 c1 = allCascadeCoords[1].xyz;
        float3 c2 = allCascadeCoords[2].xyz;
        float3 c3 = allCascadeCoords[3].xyz;

        float3 minCoord = float3(ds.filterRadius.xx / ds.shadowMapSize.x, ndcToZMapping.x * 2.0 - 2.0); /* 0.5 on GL -> -1.0, 1.0 on other API -> 0.0 */
        float3 maxCoord = float3(1.0 - minCoord.x, 1.0 / float(SHADOW_CASCADES) - minCoord.y, 1.0);

        c1.y -= 1.0 / float(SHADOW_CASCADES);
        c2.y -= 2.0 / float(SHADOW_CASCADES);
        c3.y -= 3.0 / float(SHADOW_CASCADES);

        if (all(equal(clamp(c0, minCoord, maxCoord), c0)))
        {
            cascade = 0;
        }
        else if (all(equal(clamp(c1, minCoord, maxCoord), c1)))
        {
            cascade = 1;
        }
#if (SHADOW_CASCADES > 2.0)
        else if (all(equal(clamp(c2, minCoord, maxCoord), c2)))
        {
            cascade = 2;
        }
#if (SHADOW_CASCADES > 3.0)
        else if (all(equal(clamp(c3, minCoord, maxCoord), c3)))
        {
            cascade = 3;
        }
#endif
#endif
    }

    if (cascade >= 0)
    {
        float2 cascadeCoords = allCascadeCoords[cascade];
        float2 internalCoords = float2(cascadeCoords.x, cascadeCoords.y * SHADOW_CASCADES - floor(cascadeCoords.y * SHADOW_CASCADES));

        float nextCascade = float(cascade) + 1.0;
        float lastCascadeIndex = float(SHADOW_CASCADES) - 1.0;
        float samplingIntermediateCascade = step(nextCascade, lastCascadeIndex);

        float blendPos = 0.5 - max(abs(internalCoords.x - 0.5), abs(internalCoords.y - 0.5));
        float blendAmount = 1.0 - saturate(blendPos / edgeBlendSize - 0.25);
        float blendJitter = float(blendAmount * blendAmount > ds.rotationKernel.x * 0.5 + 0.5);
        float blend = blendJitter * samplingIntermediateCascade;

        int nextCascadeIndex = int(min(nextCascade, lastCascadeIndex));
        float4 cascadeProjectionScale = lerp(ds.cascadesProjectionScale[cascade], ds.cascadesProjectionScale[nextCascadeIndex], blend);
        float4 cascadeProjectionOffset = lerp(ds.cascadesProjectionOffset[cascade], ds.cascadesProjectionOffset[nextCascadeIndex], blend);
        shadow = SampleShadow(lightViewSpaceCoords, cascadeProjectionScale, cascadeProjectionOffset, ds);

        outBlendAmount = blendAmount * (1.0 - samplingIntermediateCascade);
    }

#else /* (SHADOW_CASCADES == 1.0) */

    shadow = SampleShadow(lightViewSpaceCoords, ds.cascadesProjectionScale[0], ds.cascadesProjectionOffset[0], ds);
    {
        float4 internalCoords = lightViewSpaceCoords * ds.cascadesProjectionScale[0] + ds.cascadesProjectionOffset[0];
        float blendPos = 0.5 - max(abs(internalCoords.x - 0.5), abs(internalCoords.y - 0.5));
        outBlendAmount = 1.0 - saturate(blendPos / edgeBlendSize - 0.25);
        outBlendAmount = saturate(outBlendAmount + step(1.0, internalCoords.z));
    }

#endif

    return saturate(shadow + outBlendAmount * outBlendAmount);
}

#if (ENABLE_POINT_LIGHTS)

/*
 * Point light implementation
 */
float3 fixCubemapSampleDirection(float3 dir, float pointLightLookupTextureFaceSize)
{
    float scale = pointLightLookupTextureFaceSize / (pointLightLookupTextureFaceSize - 1.0);

    float3 absDir = abs(dir);
    float3 mask = step(absDir.xyz, max(absDir.yzx, absDir.zxy));
    return dir * lerp(float3(scale, scale, scale), float3(1.0, 1.0, 1.0), mask);
}

float BilinearPointShadowSample(float2 uv, float z, float2 lowerBound, float2 upperBound)
{
    float2 uv0 = floor(uv * pointLightsShadowMapSize) * pointLightsShadowMapTexelSize;
    return step(z, tex2D(pointShadowMap, clamp(uv0, lowerBound, upperBound)).x);
}

float SamplePointLigth(float3 lightToWorldPoint, float distanceToLight, int lightIndex, float2 rotationKernel, float2 pointLightFaceSize)
{
    if (lightIndex == -1)
        return 1.0;

    float2 pointLightDuDvSize = pointLightFaceSize / pointLightsShadowMapSize;
    float2 pointLightGridSize = pointLightsShadowMapSize / pointLightFaceSize;

    float4 lookup = texCUBE(pointLightLookup, fixCubemapSampleDirection(-lightToWorldPoint));
    int faceIndex = int(lookup.z * 8.0);

    int lookupIndex = 6 * lightIndex + faceIndex;
    int sampleRow = lookupIndex / int(pointLightGridSize.x);
    int sampleCol = fmod((float)lookupIndex, pointLightGridSize.x);

    float2 lookupBase = float2((float)sampleCol, (float)sampleRow) / pointLightGridSize;
    float2 lookupUv = lookupBase + lookup.xy * pointLightDuDvSize;
    float2 upperBound = (lookupBase + pointLightDuDvSize) - 0.5 * pointLightsShadowMapTexelSize;

    return BilinearPointShadowSample(lookupUv, distanceToLight, lookupBase, upperBound);
}

#endif
