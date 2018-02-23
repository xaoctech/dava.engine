#if (FLIP_BACKFACE_NORMALS)
resolve.n *= sign(input.varToCamera.w);
#endif

#if (VERTEX_BAKED_AO)
bakedAo *= input.vertexBakedAO;
#endif

#if (USE_BAKED_LIGHTING)
float2 bakedShadowAOSample = tex2D(shadowaotexture, input.varTexCoord.zw* shadowaoUV.zw + shadowaoUV.xy).xy;
float bakedShadow = ApplyCanvasCheckers(bakedShadowAOSample.x, input.varTexCoord.zw, shadowaoSize);
bakedAo *= bakedShadowAOSample.y;
#else
float bakedShadow = 1.0;
#endif

resolve.vLength = length(input.varToCamera.xyz);
resolve.v = input.varToCamera.xyz / resolve.vLength;
resolve.NdotV = dot(resolve.n, resolve.v);
resolve.baseColor = SRGBToLinear(float3(baseColorSample.xyz) * baseColorScale.xyz);
resolve.roughness = normalMapSample.z * roughnessScale;
resolve.directionalLightStaticShadow = bakedShadow;
resolve.ambientOcclusion = saturate((bakedAo + aoBias) * aoScale);
resolve.directionalLightDirection = lightPosition0.xyz;
resolve.directionalLightViewSpaceCoords = input.shadowTexCoord;
resolve.directionalLightColor = lightColor0.xyz / globalLuminanceScale;
resolve.worldPosition = input.worldPosition.xyz;
resolve.cameraPosition = cameraPosition;
resolve.fogParameters = fogParameters.xyz;

#if (PARALLAX_CORRECTED)
resolve.localProbeCaptureWorldToLocalMatrix = localProbeCaptureWorldToLocalMatrix;
resolve.localProbeCapturePositionInWorldSpace = localProbeCapturePositionInWorldSpace;
#endif

#if (TRANSMITTANCE)
resolve.metallness = 0.0;
resolve.transmittanceSample = normalMapSample.w;
#else
resolve.metallness = normalMapSample.w * metallnessScale;
resolve.transmittanceSample = 0.0;
#endif

SurfaceValues surface = BuildSurfaceValues(resolve.baseColor, resolve.roughness, resolve.metallness, resolve.ambientOcclusion, resolve.NdotV, resolve.transmittanceSample);

resolve.environmentDiffuseSample = 0.0;
#if (!IB_REFLECTIONS_PREPARE)
    #if (USE_DOMINANT_DIRECTION)
float3 diffuseSampleDirection = GetDiffuseDominantDirection(resolve.n, resolve.v, resolve.NdotV, surface.roughness);
    #else
float3 diffuseSampleDirection = resolve.n;
    #endif
resolve.environmentDiffuseSample += sphericalHarmonics[0].xyz * (0.282095);
resolve.environmentDiffuseSample += sphericalHarmonics[1].xyz * (0.488603 * diffuseSampleDirection.y);
resolve.environmentDiffuseSample += sphericalHarmonics[2].xyz * (0.488603 * diffuseSampleDirection.z);
resolve.environmentDiffuseSample += sphericalHarmonics[3].xyz * (0.488603 * diffuseSampleDirection.x);
resolve.environmentDiffuseSample += sphericalHarmonics[4].xyz * (1.092548 * diffuseSampleDirection.x * diffuseSampleDirection.y);
resolve.environmentDiffuseSample += sphericalHarmonics[5].xyz * (1.092548 * diffuseSampleDirection.y * diffuseSampleDirection.z);
resolve.environmentDiffuseSample += sphericalHarmonics[6].xyz * (0.315392 * (3.0 * diffuseSampleDirection.z * diffuseSampleDirection.z - 1.0));
resolve.environmentDiffuseSample += sphericalHarmonics[7].xyz * (1.092548 * diffuseSampleDirection.x * diffuseSampleDirection.z);
resolve.environmentDiffuseSample += sphericalHarmonics[8].xyz * (0.546274 * (diffuseSampleDirection.x * diffuseSampleDirection.x - diffuseSampleDirection.y * diffuseSampleDirection.y));
#endif

float2 screenSpaceCoords = input.projectedPosition.xy / input.projectedPosition.w * ndcToUvMapping.xy + ndcToUvMapping.zw;
ShadowParameters shadow;
shadow.cascadesProjectionScale = directionalShadowMapProjectionScale;
shadow.cascadesProjectionOffset = directionalShadowMapProjectionOffset;
shadow.rotationKernel = tex2D(noiseTexture64x64, screenSpaceCoords* viewportSize / 64.0).xy * 2.0 - 1.0;
shadow.filterRadius = shadowMapParameters.xy;
shadow.shadowMapSize = shadowMapParameters.zw;

float3 result = ResolveFinalColor(resolve, surface, shadow);

#if (ENABLE_POINT_LIGHTS)
int pointLightsCount = int(lightingParameters.y);
for (int i = 0; i < pointLightsCount; ++i)
{
    float4 pointLightPosition = pointLights[2 * i];
    float4 pointLightColor = pointLights[2 * i + 1];
    result += ComputePointLight(lightPosition, pointLightColor, resolve, surface);
}
#endif

#if (VIEW_MODE & VIEW_TEXTURE_MIP_LEVEL_BIT)
float2 firstMipSize = tex2Dsize(albedo, 0);
float mip = QueryLodLevel(input.varTexCoord.xy, firstMipSize);

float3 mipColors[5] = {
    float3(1.0, 1.0, 1.0), /* 0 */
    float3(0.0, 1.0, 0.0), /* 1 */
    float3(0.0, 0.0, 1.0), /* 2 */
    float3(1.0, 0.0, 1.0), /* 3 */
    float3(1.0, 0.0, 0.0) /* 4 */
};
float base = floor(mip);
float next = min(4.0, base + 1.0);
result = lerp(mipColors[int(base)], mipColors[int(next)], mip - base) + (result * 0.00001);

#elif defined(LDR_FLOW) && (IB_REFLECTIONS_PREPARE == 0)

result = PerformToneMapping(result, cameraDynamicRange, 1.0);

#endif

output.color = float4(result, 1.0);
