struct ResolveInputValues
{
    float4 directionalLightViewSpaceCoords;
    float3 worldPosition;
    float3 cameraPosition;
    float3 fogParameters;
    float3 directionalLightDirection;
    float3 directionalLightColor;
    float3 baseColor;
    float3 environmentDiffuseSample;
    float3 n;
    float3 v;
    float NdotV;
    float vLength;
    float roughness;
    float metallness;
    float ambientOcclusion;
    float directionalLightStaticShadow;
    float transmittanceSample;

#if (PARALLAX_CORRECTED)
    float4x4 localProbeCaptureWorldToLocalMatrix;
    float3 localProbeCapturePositionInWorldSpace;
#endif
};

struct SurfaceValues
{
    float3 baseColor;
    float3 f0;
    float3 f90;
    float roughness;
    float metallness;
    float ambientOcclusion;
    float transmittance;
    float specularOcclusion;
};

struct BRDFValues
{
    float NdotV;
    float NdotL;
    float NdotH;
    float LdotH;
    float LdotV;
};

struct Convolution
{
    float3 cubemap_basis_a;
    float3 cubemap_basis_b;
    float3 cubemap_basis_c;
    float cubemap_dst_last_mip;
    float cubemap_src_last_mip;
    float cubemap_dst_mip_level;
    float cubemap_src_mip_level;
    float cubemap_face_size;
    float dynamicHammersleySetSize;
    float dynamicHammersleySetIndex;
};

struct ShadowParameters
{
    float4x4 cascadesProjectionScale;
    float4x4 cascadesProjectionOffset;
    float2 filterRadius;
    float2 rotationKernel;
    float2 shadowMapSize;
};

struct RainDecalParameters
{
    float rain_intensity;
    float time;
    float4 ripples;
    float4 flow;
};
