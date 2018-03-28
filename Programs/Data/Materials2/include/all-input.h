[auto][global] property float cameraTargetLuminance;
[auto][global] property float distantDepthValue;
[auto][global] property float projectionFlipped;
[auto][global] property float2 cameraDynamicRange;
[auto][global] property float2 flexibility;
[auto][global] property float2 globalTime; // x - current time. y - previous time.
[auto][global] property float2 pointLightFaceSize;
[auto][global] property float2 renderTargetSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 viewportSize;
[auto][global] property float2 heightmapSize;
[auto][global] property float3 cameraDirection;
[auto][global] property float3 cameraPosition;
[auto][global] property float3 cameraUp;
[auto][global] property float3 localProbeCapturePositionInWorldSpace;
[auto][global] property float3 worldScale;
[auto][global] property float4 fogParameters; /* distance scale, turbidity, anisotropy */
[auto][global] property float4 lightColor0 = float4(1.0, 1.0, 1.0, 1.0);
[auto][global] property float4 lightPosition0;
[auto][global] property float4 lightingParameters = float4(0.0, 0.0, 0.0, 0.0); /* (x: unused; y: point lights count; z: unused; w: unused )*/
[auto][global] property float4 shadowMapParameters; /* PCF filter radius, cascades blend size, filter size for transmittance, empty */
[auto][global] property float4 wind;
[auto][global] property float4 prevWind;
[auto][global] property float4x4 directionalShadowMapProjectionOffset;
[auto][global] property float4x4 directionalShadowMapProjectionScale;
[auto][global] property float4x4 invProjMatrix;
[auto][global] property float4x4 invViewMatrix;
[auto][global] property float4x4 invViewProjMatrix;
[auto][global] property float4x4 invWorldViewMatrix;
[auto][global] property float4x4 localProbeCaptureWorldToLocalMatrix;
[auto][global] property float4x4 prevWorldMatrix;
[auto][global] property float4x4 projMatrix;
[auto][global] property float4x4 shadowView;
[auto][global] property float4x4 viewMatrix;
[auto][global] property float4x4 viewProjMatrix;
[auto][global] property float4x4 worldInvTransposeMatrix;
[auto][global] property float4x4 worldMatrix;
[auto][global] property float4x4 worldViewMatrix;
[auto][global] property float4x4 worldViewProjMatrix;

[auto][jpos] property float4 jointPositions[MAX_JOINTS] : "bigarray"; // (x, y, z, scale)
[auto][jrot] property float4 jointQuaternions[MAX_JOINTS] : "bigarray";
[auto][jposp] property float4 prevJointPositions[MAX_JOINTS] : "bigarray"; // (x, y, z, scale)
[auto][jrotp] property float4 prevJointQuaternions[MAX_JOINTS] : "bigarray";

[auto][pl] property float4 pointLights[POINT_LIGHTS_BUFFER_SIZE] : "bigarray"; /* (xyz: position, w: linear attenuation), (xyz: color, w: exponential attenuation) */
[auto][sh] property float4 sphericalHarmonics[9] : "bigarray";

[material][a] property float albedoAlphaStep = 0.125;
[material][a] property float albedoMinAOValue = 0.5;
[material][a] property float aoBias = 0.0;
[material][a] property float aoScale = 1.0;
[material][a] property float cubemap_dst_last_mip;
[material][a] property float cubemap_dst_mip_level;
[material][a] property float cubemap_face_size;
[material][a] property float cubemap_src_last_mip;
[material][a] property float cubemap_src_mip_level;
[material][a] property float dynamicHammersleySetIndex;
[material][a] property float dynamicHammersleySetSize;

#if (VIEW_MODE & VIEW_LIGHTMAP_CANVAS_BIT)
[material][a] property float lightmapSize = 1.0;
#else
    #define lightmapSize 1.0
#endif
[material][a] property float lightmapTextureSize = 1.0;
[material][a] property float maskSoftness = 0.05;
[material][a] property float maskThreshold = 0.5;
[material][a] property float metallnessScale = 1.0;
[material][a] property float normalScale = 1.0;
[material][a] property float rain_intensity = 1.0;
[material][a] property float roughnessScale = 1.0;
[material][a] property float2 uvOffset = float2(0.0, 0.0);
[material][a] property float2 uvScale = float2(1.0, 1.0);
[material][a] property float3 cubemap_basis_a;
[material][a] property float3 cubemap_basis_b;
[material][a] property float3 cubemap_basis_c;
[material][a] property float4 baseColorScale = float4(1.0, 1.0, 1.0, 1.0);
[material][a] property float4 color = float4(1.0, 1.0, 1.0, 1.0);
[material][a] property float4 cursorCoordSize = float4(0, 0, 1, 1);
[material][a] property float4 environmentColor = float4(1.0, 1.0, 1.0, 1.0);
[material][a] property float4 levelColor = float4(0, 0, 0, 0);
[material][a] property float4 params_flow = float4(0.2, 0.0, 0.2, 0.0); // normal strength, uv scale, scrool speed
[material][a] property float4 params_ripples = float4(1.0, 0.9, 1.5, 30.0); // normal strength, uv scale, scrool speed

uniform sampler2D albedo;
uniform sampler2D cursorTexture;
uniform sampler2D gBuffer0;
uniform sampler2D gBuffer0_copy;
uniform sampler2D gBuffer1;
uniform sampler2D gBuffer1_copy;
uniform sampler2D gBuffer2;
uniform sampler2D gBuffer2_copy;
uniform sampler2D gBuffer3;
uniform sampler2D hammersleySet;
uniform sampler2D indirectSpecularLookup;
uniform sampler2D landCoverTexture;
uniform sampler2D noiseTexture64x64;
uniform sampler2D normalmap;
uniform sampler2D pointShadowMap;
uniform sampler2D shadowaotexture;
uniform sampler2D toolTexture;
uniform sampler2D heightmap;
uniform sampler2D tangentmap;
uniform sampler2DShadow directionalShadowMap;
uniform samplerCUBE globalReflection;
uniform samplerCUBE localReflection;
uniform samplerCUBE pointLightLookup;
uniform samplerCUBE smp_src;
