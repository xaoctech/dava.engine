/*
 * Here we always expect 4 rows in hammersley set texture. The sizes of the hammersley sets can differ. 
 */
float2 SampleDynamicHammersleySet(float i)
{
    return tex2Dlod(hammersleySet, float2(i / HAMMERSLEY_SET_SIZE, 0.125 + dynamicHammersleySetIndex * 0.25), 0.0).xy;
}

float2 SampleHammersleySet(float i)
{
    return tex2Dlod(hammersleySet, float2(i / HAMMERSLEY_SET_SIZE, 0.875), 0.0).xy;
}

float4 DiffuseReferenceUsingImportanceSampling(float3 n, float3 v, float r1)
{
    float3 result = float3(0.0, 0.0, 0.0);

    float NdotV = saturate(dot(n, v));
    float3 TangentX = PurrPendicularVector(n);
    float3 TangentY = cross(n, TangentX);

    for (float i = 0.0; i < HAMMERSLEY_SET_SIZE; i += 1.0)
    {
        float2 Xi = SampleHammersleySet(i);
        float3 l = ImportanceSampleCosine(Xi);
        l = TangentX * l.x + TangentY * l.y + n * l.z;

        // float bsdf = BURLEY / _PI;
        // float pdf = LdotN / _PI;
        // result += (TEX) * (bsdf / pdf) * LdotN ----> TEX * (BURLEY * LdotN / _PI) / (LdotN / _PI) ----> BURLEY
        // LdotN and _PI cancels out
        float LdotN = saturate(dot(n, l));
        float LdotH = saturate(dot(l, normalize(l + v)));
        float bsdf_over_pdf = BurleyDiffuse(LdotN, NdotV, LdotH, r1);
        result += texCUBElod(globalReflection, l, 2.0).xyz * bsdf_over_pdf;
    }

    return float4(result / HAMMERSLEY_SET_SIZE, 0.0);
}

float4 SpecularReferenceUsingImportanceSampling(float3 n, float3 v, float r1, float3 f0, float3 f90)
{
    float3 result = float3(0.0, 0.0, 0.0);

    r1 = clamp(r1, MIN_ROUGHNESS, 1.0);
    float r2 = r1 * r1;
    float r4 = r2 * r2;

    float NdotV = saturate(dot(n, v));
    float3 TangentX = PurrPendicularVector(n);
    float3 TangentY = cross(n, TangentX);

    for (int i = 0; i < HAMMERSLEY_SET_SIZE; ++i)
    {
        float2 Xi = SampleHammersleySet(i);
        float3 h = ImportanceSampleGGX(Xi, r4);
        h = TangentX * h.x + TangentY * h.y + n * h.z;
        float3 l = normalize(2.0 * dot(v, h) * h - v);

        float NdotL = dot(n, l);
        if (NdotL > 0.0)
        {
            // brdf = D * G * F / (4.0 * NdotV * NdotL)
            // pdf = D * NdotH / (4.0 * VdotH);
            // result += TEX * (brdf / pdf) * NdotL ----->
            //  TEX * (D * G * F / (4.0 * NdotV * NdotL)) / (D * NdotH / (4.0 * VdotH)) * NdotL ----->
            //  TEX * (4.0 * VdotH * NdotL * D * G * F) / (4.0 * NdotV * NdotL * D * NdotH) ----->
            //  TEX * (VdotH * G * F) / (NdotV * NdotH)
            float LdotH = saturate(dot(l, h));
            float VdotH = saturate(dot(l, h));
            float NdotH = saturate(dot(n, h));
            float G = G_Smith(NdotV, NdotL, r2);
            float3 F = FresnelSchlickVector(f0, f90, LdotH);
            float brdf_over_pdf = VdotH * F * G / (NdotH * NdotV);
            result += texCUBElod(globalReflection, l, 0.0).xyz * brdf_over_pdf;
        }
    }

    return float4(result / HAMMERSLEY_SET_SIZE, 1.0);
}
