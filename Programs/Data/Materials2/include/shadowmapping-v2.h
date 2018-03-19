/*
 * Directional light implementation
 */
float SampleShadow(float3 lightSpaceCoords, float3 projectionScale, float3 projectionOffset, ShadowParameters sp)
{
    float shadow = 0.0;

#if (SHADOW_PCF > 0)
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

        float3 projectedCoords = lightSpaceCoords * projectionScale + projectionOffset;
        projectedCoords.x += dot(poissonDistribution[i].xy, r0);
        projectedCoords.y += dot(poissonDistribution[i].xy, r1);

        shadow += FP_SHADOW(tex2Dcmp(directionalShadowMap, projectedCoords));
    }
    shadow *= 1.0 / float(SHADOW_PCF);

#else

    float3 projectedCoords = lightSpaceCoords * projectionScale + projectionOffset;
    shadow = FP_SHADOW(tex2Dcmp(directionalShadowMap, projectedCoords));

#endif

    return shadow;
}

float ComputeCascadesBlendValue(float3 lvs, float3 scale, float3 offset)
{
    float blendSize = 0.25;

    float3 c0 = float3((0.0 - offset.x), (1.0 - offset.y), (1.0 - offset.z)) / scale;
    float3 c1 = float3((1.0 - offset.x), (0.0 - offset.y), (0.0 - offset.z)) / scale;
    float3 center = 0.5 * (c0 + c1);
    float3 size = 0.5 * (c1 - c0);

    float3 s = max(0.0, abs(lvs - center) - (size - blendSize));
    return max(s.x, max(s.y, s.z)) / blendSize;
}

float JitterBlend(float blendAmount, float normalizedNoiseValue)
{
    return float(blendAmount > normalizedNoiseValue);
}

float SampleDirectionalShadow(float3 lightViewSpaceCoords, ShadowParameters ds)
{
    float shadow = 1.0;
    float outBlendAmount = 1.0;

#if (SHADOW_CASCADES == 0.0)

    shadow = 1.0;

#elif (SHADOW_CASCADES > 1.0)
    float3 c0 = lightViewSpaceCoords * ds.cascadesProjectionScale[0] + ds.cascadesProjectionOffset[0];
    float3 c1 = lightViewSpaceCoords * ds.cascadesProjectionScale[1] + ds.cascadesProjectionOffset[1];
#if (SHADOW_CASCADES > 2.0)
    float3 c2 = lightViewSpaceCoords * ds.cascadesProjectionScale[2] + ds.cascadesProjectionOffset[2];
#if (SHADOW_CASCADES > 3.0)
    float3 c3 = lightViewSpaceCoords * ds.cascadesProjectionScale[3] + ds.cascadesProjectionOffset[3];
#endif
#endif

    int cascade = -1;
    int nextCascade = -1;
    float nextCascadeCoordsValid = 0.0;
    float lastCascade = 0.0;
    {
        float3 minCoord = float3(ds.filterRadius.xx / ds.shadowMapSize.x, ndcToZMapping.x * 2.0 - 2.0); /* 0.5 on GL -> -1.0, 1.0 on other API -> 0.0 */
        float3 maxCoord = float3(1.0 - minCoord.x, 1.0 / float(SHADOW_CASCADES) - minCoord.y, 1.0);

        bool cascade0 = all(equal(clamp(c0, minCoord, maxCoord), c0));

        c1.y -= 1.0 / float(SHADOW_CASCADES);
        bool cascade1 = all(equal(clamp(c1, minCoord, maxCoord), c1));

        bool cascade2 = false;
        bool cascade3 = false;

    #if (SHADOW_CASCADES > 2.0)
        c2.y -= 2.0 / float(SHADOW_CASCADES);
        cascade2 = all(equal(clamp(c2, minCoord, maxCoord), c2));
    #if (SHADOW_CASCADES > 3.0)
        c3.y -= 3.0 / float(SHADOW_CASCADES);
        cascade3 = all(equal(clamp(c3, minCoord, maxCoord), c3));
    #endif
    #endif

        if (cascade0)
        {
            cascade = 0;
            nextCascade = 1;
            nextCascadeCoordsValid = cascade1 ? 1.0 : 0.0;
        }
        else if (cascade1)
        {
            cascade = 1;
            nextCascade = 2;
            nextCascadeCoordsValid = cascade2 ? 1.0 : 0.0;
        }
    #if (SHADOW_CASCADES > 2.0)
        else if (cascade2)
        {
            cascade = 2;
            nextCascade = 3;
            nextCascadeCoordsValid = cascade3 ? 1.0 : 0.0;
        }
    #if (SHADOW_CASCADES > 3.0)
        else if (cascade3)
        {
            cascade = 3;
            nextCascade = 3;
            lastCascade = 1.0;
            nextCascadeCoordsValid = 1.0;
        }
    #endif
    #endif
    }

    if (cascade >= 0)
    {
        float cascadesBlend;
        cascadesBlend = ComputeCascadesBlendValue(lightViewSpaceCoords, ds.cascadesProjectionScale[cascade], ds.cascadesProjectionOffset[cascade]);
        cascadesBlend *= nextCascadeCoordsValid;

        float jitteredCascadesBlend = JitterBlend(cascadesBlend, ds.rotationKernel.x * 0.5 + 0.5);
        float3 cascadeProjectionScale = lerp(ds.cascadesProjectionScale[cascade], ds.cascadesProjectionScale[nextCascade], jitteredCascadesBlend);
        float3 cascadeProjectionOffset = lerp(ds.cascadesProjectionOffset[cascade], ds.cascadesProjectionOffset[nextCascade], jitteredCascadesBlend);
        shadow = SampleShadow(lightViewSpaceCoords, cascadeProjectionScale, cascadeProjectionOffset, ds);

        outBlendAmount = cascadesBlend * lastCascade;
    }

#else /* (SHADOW_CASCADES == 1.0) */

    float3 cascadeScale = ds.cascadesProjectionScale[0];
    float3 cascadeOffset = ds.cascadesProjectionOffset[0];
    shadow = SampleShadow(lightViewSpaceCoords, cascadeScale, cascadeOffset, ds);
    {
        float3 selectedCascadeCoords = lightViewSpaceCoords * cascadeScale + cascadeOffset;
        outBlendAmount = ComputeCascadesBlendValue(lightViewSpaceCoords, cascadeScale, cascadeOffset);
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
