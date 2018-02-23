// http://blog.selfshadow.com/publications/blending-in-detail/

uniform sampler2D normalDetail;
[material][a] property float2 detailTiling = float2(1.0f, 1.0f);
[material][a] property float2 blendToMacroNormalMinMaxDistance = float2(20.0f, 40.0f);
[material][a] property float normalBlendFactor = 0.0f;
#if USE_DETAIL_NORMAL_AO
[material][a] property float detailAOScale = 1.0f;
#endif

float3 BlendNormals(float3 n1, float3 n2, float t)
{
    float2 pd = lerp(n1.xy / (n1.z + 0.00001f), n2.xy / (n2.z + 0.00001f), t);
    float3 r = normalize(float3(pd, 1.0f));
    return r;
}

float3 BlendLinear(float3 n1, float3 n2)
{
    return normalize(n1 + n2);
}

float3 BlendOverlay(float3 n1, float3 n2)
{
    n1 = n1 * float3(2.0f, 2.0f, 2.0f);
    float3 a = float3(n1.x >= 0.0f ? -1.0f : 1.0f, n1.y >= 0.0f ? -1.0f : 1.0f, n1.z >= 0.0f ? -1.0f : 1.0f);
    float3 b = max(-a, float3(0.0f));
    n1 = 2.0f * a + n1;
    n2 = (n2 * 0.5f + 0.5f) * a + b;

    return n1 * n2 - a;
}

float3 BlendPartialDerivative(float3 n1, float3 n2)
{
    float3 r = normalize(float3(n1.xy * n2.z + n2.xy * n1.z, n1.z * n2.z));
    return r;
}

float3 BlendWhiteout(float3 n1, float3 n2)
{
    float3 r = normalize(float3(n1.xy + n2.xy, n1.z * n2.z));
    return r;
}

float3 BlendUDN(float3 n1, float3 n2)
{
    float3 r = normalize(float3(n1.xy + n2.xy, n1.z));
    return r;
}

float3 BlendReorientedNormal(float3 n1, float3 n2)
{
    float3 t = float3(n1.xy, n2.z + 1.0f);
    float3 u = float3(-n2.xy, n2.z);
    float3 r = normalize(t * dot(t, u) / (t.z + 0.00001f) - u);
    return r;
}

struct NormalSampleResult
{
    float3 normal;
    float z; // roughness;
    float w; // metallness;
    float ao;
};

NormalSampleResult SampleNormal(float2 inputUv, float distToCam, float nScale, float aoIn)
{
    float4 macroSample = tex2D(normalmap, inputUv.xy);
    float4 detailSample = tex2D(normalDetail, inputUv.xy * detailTiling);
    NormalSampleResult sampleResult;

    float2 macroxy = (2.0f * macroSample.xy - 1.0f) * nScale;
    float3 macroNorm = float3(macroxy, sqrt(1.0f - saturate(dot(macroxy, macroxy))));

    float2 detailxy = (2.0f * detailSample.xy - 1.0f) * nScale;
    float3 detailNorm = float3(detailxy, sqrt(1.0f - saturate(dot(detailxy, detailxy))));

#if (NORMAL_BLEND_MODE == 0)
    sampleResult.normal = BlendUDN(macroNorm, detailNorm);
#elif (NORMAL_BLEND_MODE == 1)
    sampleResult.normal = BlendWhiteout(macroNorm, detailNorm);
#elif (NORMAL_BLEND_MODE == 2)
    sampleResult.normal = BlendPartialDerivative(macroNorm, detailNorm);
#elif (NORMAL_BLEND_MODE == 3)
    sampleResult.normal = BlendOverlay(macroNorm, detailNorm);
#elif (NORMAL_BLEND_MODE == 4)
    sampleResult.normal = BlendLinear(macroNorm, detailNorm);
#elif (NORMAL_BLEND_MODE == 5)
    sampleResult.normal = BlendReorientedNormal(macroNorm, detailNorm);
#endif
    float w = (distToCam - blendToMacroNormalMinMaxDistance.x) / (blendToMacroNormalMinMaxDistance.y - blendToMacroNormalMinMaxDistance.x);
    w = saturate(w);

    sampleResult.normal = BlendNormals(sampleResult.normal, macroNorm, w);
    sampleResult.z = macroSample.z;
    sampleResult.w = macroSample.w;

#if USE_DETAIL_NORMAL_AO
    float aoFromNm = saturate(max(albedoMinAOValue, detailSample.z * detailAOScale));
    float aoAtClose = saturate(aoFromNm * aoIn * 2.0f);
    sampleResult.ao = lerp(aoAtClose, aoIn, w);
#else
    sampleResult.ao = aoIn;
#endif

    return sampleResult;
}