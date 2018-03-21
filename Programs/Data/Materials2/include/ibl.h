#ensuredefined INTEGRATE_BRDF_LOOKUP 0
#ensuredefined DOWNSAMPLING 0
#ensuredefined DIFFUSE_CONVOLUTION 0
#ensuredefined DIFFUSE_SPHERICAL_HARMONICS 0
#ensuredefined SPECULAR_CONVOLUTION 0

float3 DirectionForCurrentBasis(float2 uv, Convolution cnv)
{
    float2 halfPixelOffset = centerPixelMapping / cnv.cubemap_face_size;
    float3 direction = normalize(
    cnv.cubemap_basis_a * ((uv.x + halfPixelOffset.x) * 2.0 - 1.0) +
    cnv.cubemap_basis_b * ((uv.y + halfPixelOffset.y) * 2.0 - 1.0) +
    cnv.cubemap_basis_c);
    return direction;
}

#if (INTEGRATE_BRDF_LOOKUP)

float4 IntegrateBRDFLookupTexture(float2 texCoord)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float NdotV = texCoord.x;

    float r1 = max(MIN_ROUGHNESS, texCoord.y);
    float r2 = r1 * r1;
    float r4 = r2 * r2;

    float3 V = float3(0.0, sqrt(1.0 - NdotV * NdotV), NdotV);
    float3 N = float3(0.0, 0.0, 1.0);

    for (float i = 0.0; i < HAMMERSLEY_SET_SIZE; i += 1.0)
    {
        float sampleU = i / HAMMERSLEY_SET_SIZE;
        float2 Xi = tex2Dlod(hammersleySet, float2(sampleU, 0.0), 0.0).xy;

        float3 H = ImportanceSampleGGX(Xi, r4);
        float3 L = 2.0 * dot(V, H) * H - V;

        float LdotN = dot(L, N);
        if (LdotN > 0.0)
        {
            float HdotN = saturate(dot(H, N));
            float VdotH = saturate(dot(V, H));
            float brdf_over_pdf = G_Smith(NdotV, LdotN, r4) * VdotH / (HdotN * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            result.x += brdf_over_pdf * (1.0 - Fc);
            result.y += brdf_over_pdf * Fc;
        }

        L = ImportanceSampleCosine(Xi);
        LdotN = saturate(dot(L, N));
        if (LdotN > 0.0)
        {
            float cosTheta = saturate(dot(L, normalize(L + V)));
            result.z += BurleyDiffuse(LdotN, NdotV, cosTheta, r2);
        }
    }
    return float4(result / HAMMERSLEY_SET_SIZE, 1.0);
}

#elif (DOWNSAMPLING)

float4 Downsample(float2 texcoord_, Convolution cv)
{
    return texCUBElod(smp_src, DirectionForCurrentBasis(texcoord_, cv), cv.cubemap_src_mip_level);
}

#elif (DIFFUSE_CONVOLUTION)

float4 ConvoluteDiffuse(float2 texcoord_, Convolution conv)
{
    float3 t0[6];
    float3 t1[3];
    float3 t2[3];

    t0[0] = float3(1.0, 0.0, 0.0);
    t0[1] = float3(-1.0, 0.0, 0.0);
    t0[2] = float3(0.0, 1.0, 0.0);
    t0[3] = float3(0.0, -1.0, 0.0);
    t0[4] = float3(0.0, 0.0, 1.0);
    t0[5] = float3(0.0, 0.0, -1.0);

    t1[0] = float3(0.0, 1.0, 0.0);
    t1[1] = float3(0.0, 0.0, 1.0);
    t1[2] = float3(1.0, 0.0, 0.0);

    t2[0] = float3(0.0, 0.0, 1.0);
    t2[1] = float3(1.0, 0.0, 0.0);
    t2[2] = float3(0.0, 1.0, 0.0);

    int sampledLevel = 3; /* assuming level 0 has 256x256 size, hopefully level 3 will be 32x32 */
    int w = 32;
    int h = 32;
    float invFaceSize = 1.0 / float(w);
    float texelSolidAngle = 4.0 * _PI * invFaceSize * invFaceSize / 6.0;

    float3 n = DirectionForCurrentBasis(texcoord_, conv);

    float du = 2.0 / float(w - 1);
    float dv = 2.0 / float(h - 1);
    float3 integralResult = 0.0;
    for (int face = 0; face < 6; ++face)
    {
        float3 a0 = t0[face];
        float3 a1 = t1[face / 2];
        float3 a2 = t2[face / 2];

        float v = -1.0;
        for (int y = 0; y < h; ++y)
        {
            float u = -1.0;
            for (int x = 0; x < w; ++x)
            {
                float3 direction = normalize(a0 + a1 * u + a2 * v);
                float cs = dot(n, direction);
                if (cs >= 0.0)
                {
                    float3 smp = texCUBElod(smp_src, direction, sampledLevel).xyz;
                    integralResult += cs * texelSolidAngle * smp;
                }
                u += du;
            }
            v += dv;
        }
    }

    float scale = (1.0 + invFaceSize) / _PI;
    return float4(integralResult * scale, 1.0);
}

#elif (DIFFUSE_SPHERICAL_HARMONICS)

struct SphericalHarmonics
{
    float3 sh0;
    float3 sh1;
    float3 sh2;
    float3 sh3;
    float3 sh4;
    float3 sh5;
    float3 sh6;
    float3 sh7;
    float3 sh8;
};

SphericalHarmonics ConvoluteSphericalHarmonics(float dummy /* to trick sl-parser ^_^ */)
{
    float3 t0[6];
    float3 t1[3];
    float3 t2[3];

    t0[0] = float3(1.0, 0.0, 0.0);
    t0[1] = float3(-1.0, 0.0, 0.0);
    t0[2] = float3(0.0, 1.0, 0.0);
    t0[3] = float3(0.0, -1.0, 0.0);
    t0[4] = float3(0.0, 0.0, 1.0);
    t0[5] = float3(0.0, 0.0, -1.0);

    t1[0] = float3(0.0, 1.0, 0.0);
    t1[1] = float3(0.0, 0.0, 1.0);
    t1[2] = float3(1.0, 0.0, 0.0);

    t2[0] = float3(0.0, 0.0, 1.0);
    t2[1] = float3(1.0, 0.0, 0.0);
    t2[2] = float3(0.0, 1.0, 0.0);

    int w = 32;
    int h = 32;
    int sampledLevel = 3;
    float texelSolidAngle = 4.0 * _PI / (6.0 * float(w * h));
    float du = 2.0 / float(w - 1);
    float dv = 2.0 / float(h - 1);

    SphericalHarmonics result;

    for (int face = 0; face < 6; ++face)
    {
        float3 a0 = t0[face];
        float3 a1 = t1[face / 2];
        float3 a2 = t2[face / 2];

        float v = -1.0;
        for (int y = 0; y < h; ++y)
        {
            float u = -1.0;
            for (int x = 0; x < w; ++x)
            {
                float3 direction = normalize(a0 + a1 * u + a2 * v);
                float3 radiance = texCUBElod(smp_src, direction, sampledLevel).xyz;
                result.sh0 += radiance;
                result.sh1 += radiance * (direction.y);
                result.sh2 += radiance * (direction.z);
                result.sh3 += radiance * (direction.x);
                result.sh4 += radiance * (direction.x * direction.y);
                result.sh5 += radiance * (direction.y * direction.z);
                result.sh6 += radiance * (3.0 * direction.z * direction.z - 1.0);
                result.sh7 += radiance * (direction.x * direction.z);
                result.sh8 += radiance * (direction.x * direction.x - direction.y * direction.y);
                u += du;
            }
            v += dv;
        }
    }

    result.sh0 *= ((1.0 / 1.0) * 0.282095) * texelSolidAngle;
    result.sh1 *= ((2.0 / 3.0) * 0.488603) * texelSolidAngle;
    result.sh2 *= ((2.0 / 3.0) * 0.488603) * texelSolidAngle;
    result.sh3 *= ((2.0 / 3.0) * 0.488603) * texelSolidAngle;
    result.sh4 *= ((1.0 / 4.0) * 1.092548) * texelSolidAngle;
    result.sh5 *= ((1.0 / 4.0) * 1.092548) * texelSolidAngle;
    result.sh6 *= ((1.0 / 4.0) * 0.315392) * texelSolidAngle;
    result.sh7 *= ((1.0 / 4.0) * 1.092548) * texelSolidAngle;
    result.sh8 *= ((1.0 / 4.0) * 0.546274) * texelSolidAngle;

    return result;
}

#elif (SPECULAR_CONVOLUTION)

float4 ConvoluteSpecular(float2 texcoord_, Convolution conv)
{
    float t = max(MIN_ROUGHNESS, conv.cubemap_dst_mip_level / conv.cubemap_dst_last_mip);
    float tSquared = t * t;
    float roughness = tSquared * tSquared;

    float invFaceSize = 1.0 / conv.cubemap_face_size;
    float invSamples = 1.0 / float(HAMMERSLEY_SET_SIZE);
    float texelSolidAngle = 4.0 * _PI * invFaceSize * invFaceSize / 6.0;
    float lastValidMip = conv.cubemap_src_last_mip - 2.0;

    float3 n = DirectionForCurrentBasis(texcoord_, conv);
    float3 v = n;
    float3 up = abs(n.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tX = normalize(cross(up, n));
    float3 tY = cross(n, tX);

    float4 result = 0.0;
    for (float i = 0; i < HAMMERSLEY_SET_SIZE; i += 1.0)
    {
        float sampleU = i / HAMMERSLEY_SET_SIZE;
        float2 Xi = tex2Dlod(hammersleySet, float2(sampleU, 0.0), 0.0).xy;

        float3 h = ImportanceSampleGGX(Xi, roughness);
        h = tX * h.x + tY * h.y + n * h.z;

        float3 l = 2.0 * dot(v, h) * h - v;

        float LdotN = saturate(dot(l, n));
        float NdotH = saturate(dot(n, h));
        float LdotH = saturate(dot(l, h));
        float invPdf = 4.0 * LdotH / (D_GGX(NdotH, roughness) * NdotH);
        float sampleSolidAngle = invSamples * invPdf;
        float sampledLevel = clamp(conv.cubemap_src_mip_level + 0.5 * log2(2.0 * sampleSolidAngle / texelSolidAngle), 0.0, lastValidMip);
        result.xyz += LdotN * texCUBElod(smp_src, l, sampledLevel).xyz;
        result.w += LdotN;
    }
    result = EncodeRGBM(result.xyz / result.w);
    return result;
}
#else
    #error Invalid Shader Configuration
#endif
