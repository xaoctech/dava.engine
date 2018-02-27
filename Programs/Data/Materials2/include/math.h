bool isNan(float val)
{
    return ((val < 0.0) || (val > 0.0) || (val == 0.0)) ? false : true;
}

bool isNan(float2 val)
{
    return isNan(val.x) || isNan(val.y);
}

bool isNan(float3 val)
{
    return isNan(val.x) || isNan(val.y) || isNan(val.z);
}

bool isNan(float4 val)
{
    return isNan(val.x) || isNan(val.y) || isNan(val.z) || isNan(val.w);
}

float3 FresnelSchlick(float f0, float f90, float t)
{
    return f0 + (f90 - f0) * pow(1.0 - t, 5.0);
}

float3 FresnelSchlickVector(float3 f0, float3 f90, float t)
{
    return f0 + (f90 - f0) * pow(1.0 - t, 5.0);
}

float G_Schlick_1(float cosTheta, float squaredRoughness)
{
    float k = 0.5 * squaredRoughness;
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

float G_Schlick(float cosThetaI, float cosThetaO, float squaredRoughness)
{
    float G1 = G_Schlick_1(cosThetaI, squaredRoughness);
    float G2 = G_Schlick_1(cosThetaO, squaredRoughness);
    return G1 * G2;
}

float G_Smith_1(float cosTheta, float squaredRoughness)
{
    float cosTheta2 = cosTheta * cosTheta;
    float tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + squaredRoughness * tanTheta2));
}

float G_Smith(float cosThetaI, float cosThetaO, float squaredRoughness)
{
    float G1 = G_Smith_1(cosThetaI, squaredRoughness);
    float G2 = G_Smith_1(cosThetaO, squaredRoughness);
    return G1 * G2;
}

float G_Smith_CombinedWithBRDF(float cosThetaI, float cosThetaO, float squaredRoughness)
{
    float t1 = squaredRoughness / (cosThetaI * cosThetaI);
    float t2 = squaredRoughness / (cosThetaO * cosThetaO);
    float G1 = 1.0 / (1.0 + sqrt(1.0 - squaredRoughness + t1));
    float G2 = 1.0 / (1.0 + sqrt(1.0 - squaredRoughness + t2));
    return G1 * G2 / cosThetaI;
}

float G_SmithCorrelated_CombinedWithBRDF(float cosThetaI, float cosThetaO, float squaredRoughness)
{
    float nv2 = cosThetaI * cosThetaI;
    float nl2 = cosThetaO * cosThetaO;
    float G1 = cosThetaO * sqrt(nv2 + squaredRoughness * (1.0 - nv2));
    float G2 = cosThetaI * sqrt(nl2 + squaredRoughness * (1.0 - nl2));
    return 0.5 * cosThetaO / (G1 + G2);
}

float D_GGX(float cosTheta, float squaredRoughness)
{
    float d = (cosTheta * squaredRoughness - cosTheta) * cosTheta + 1.0;
    return squaredRoughness / (d * d * _PI);
}

float3 ImportanceSampleGGX(float2 xi, float roughnessTo4)
{
    float cosTheta = min(sqrt((1.0 - xi.y) / (1.0 + roughnessTo4 * xi.y - xi.y)), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = 2.0 * _PI * xi.x;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float3 ImportanceSampleCosine(float2 xi)
{
    float cosTheta = sqrt(xi.y);
    float sinTheta = sqrt(1.0 - xi.y);
    float phi = 2.0 * _PI * xi.x;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float3 ImportanceSampleUniform(float2 xi)
{
    float cosTheta = xi.y;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = 2.0 * _PI * xi.x;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float3 PurrPendicularVector(float3 nrm)
{
    float nz = float(abs(nrm.z) > 0.99);
    float3 q = float3(nz, 0.0, 1.0 - nz);
    return normalize(cross(nrm, q));
}

float2 UnwrapDirectionToEquirectangular(float3 dir)
{
    float theta = (asin(dir.z) + 0.5 * _PI) / _PI;
    float phi = (atan2(dir.y, dir.x) + _PI) / (2.0 * _PI);
    return 1.0 - float2(phi, theta);
}

float3 WrapEquirectangularToDirection(float2 uv)
{
    float phi = (2.0 - uv.x) * _PI;
    float theta = uv.y * (0.5 * _PI);
    float sinTheta = sin(theta);
    float cosTheta = sqrt(1.0 - sinTheta * sinTheta);
    return float3(cos(phi) * cosTheta, sin(phi) * cosTheta, sinTheta);
}

float DistanceAttenuation(float radius, float dist)
{
    float d2 = 0.01 * dist * dist;
    float dr = dist / radius;
    float dr2 = dr * dr;
    float falloff = saturate(1.0 - dr2 * dr2);
    float attenuation = 1.0 / max(d2, POINT_LIGHT_SIZE * POINT_LIGHT_SIZE);
    return attenuation * falloff;
}

/*
 * SRGB <-> Linear conversion
 */
inline float3 SRGBToLinear(float3 value)
{
#if (USE_PRECISE_SRGB_CONVERSION)
    float3 linearLO = value / 1.055;
    float3 linearHI = pow((abs(value) + 0.055) / 1.055, 2.4);
    float r = (value.r <= 0.04045) ? linearLO.r : linearHI.r;
    float g = (value.g <= 0.04045) ? linearLO.g : linearHI.g;
    float b = (value.b <= 0.04045) ? linearLO.b : linearHI.b;
    return float3(r, g, b);
#elif (USE_PLAUSIBLE_SRGB_CONVERSION)
    return pow(value, 2.2);
#else
    return value * value;
#endif
}

float3 LinearTosRGB(float3 linearValue)
{
#if (USE_PRECISE_SRGB_CONVERSION)
    float3 srgbLO = linearValue * 12.92;
    float3 srgbHI = (pow(abs(linearValue), 0.41677) * 1.055) - 0.055;
    float r = (linearValue.r <= 0.0031308) ? srgbLO.r : srgbHI.r;
    float g = (linearValue.g <= 0.0031308) ? srgbLO.g : srgbHI.g;
    float b = (linearValue.b <= 0.0031308) ? srgbLO.b : srgbHI.b;
    return float3(r, g, b);
#elif (USE_PLAUSIBLE_SRGB_CONVERSION)
    return pow(linearValue, 1.0 / 2.2);
#else
    return sqrt(linearValue);
#endif
}

float RGBtoLuminance(float3 rgb)
{
    return dot(float3(0.2126, 0.7152, 0.0722), rgb);
}

/*
 * Distance packing/unpacking (float <-> RGBA8)
 */
float4 PackDistance(float dist)
{
    float4 distancePackVector = float4(1.0, 255.0, 65025.0, 160581375.0);
    float4 enc = frac(distancePackVector * dist);
    return enc - enc.yzww * float4(1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0, 0.0);
}

float UnpackDistance(float4 packedDistance)
{
    float bias = 1.0 / 255.0;
    float4 distancePackVector = float4(1.0, 255.0, 65025.0, 160581375.0);
    return dot(packedDistance, 1.0 / distancePackVector) + bias;
}

float BurleyDiffuse(float cosThetaO, float cosThetaI, float LdotH, float roughness)
{
    // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering
    // by Brent Burley, Walt Disney Animation Studios
    // http://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf

    float Fv = pow(1.0 - cosThetaI, 5.0);
    float Fl = pow(1.0 - cosThetaO, 5.0);
    float Rr = 2.0 * saturate(roughness) * LdotH * LdotH;

    float base = (1.0 - 0.5 * Fv) * (1.0 - 0.5 * Fl);
    float retroreflection = Rr * (Fl + Fv + Fl * Fv * (Rr - 1.0));

    return base + retroreflection;
}

float3 GetDiffuseDominantDirection(float3 n, float3 v, float cosThetaI, float roughness)
{
    float r = saturate(roughness);
    float a = 1.02341 * r - 1.51174;
    float b = -0.511705 * r + 0.755868;
    float lerpFactor = saturate((cosThetaI * a + b) * r);
    return normalize(lerp(n, v, lerpFactor));
}

float3 GetSpecularDominantDirection(float3 nrm, float3 ref, float roughness)
{
    float r = saturate(roughness);
    float smoothness = 1.0 - r;
    float lerpFactor = smoothness * (sqrt(smoothness) + r);
    return lerp(nrm, ref, lerpFactor);
}

float PhaseFunctionRayleigh(float cosTheta)
{
    return (3.0 / 4.0) * (1.0 + cosTheta * cosTheta) / (4.0 * _PI);
}

float PhaseFunctionHenyeyGreenstein(float cosTheta, float g)
{
    float t1 = 3.0 * (1.0 - g * g) / 2.0 * (2.0 + g * g);
    float t2 = (1.0 + cosTheta * cosTheta) / pow(abs(1.0 + g * g - 2.0 * g * cosTheta), 3.0 / 2.0);
    return t1 * t2 / (4.0 * _PI);
}

float PhaseFunctionSchlick(float angleCosine, float g)
{
    float t = 1.0 + g * angleCosine;
    return (1.0 - g * g) / (4.0 * _PI * t * t);
}

float2 ScatteringPhaseFunctions(float3 view, float3 light, float anisotropy)
{
    float angleCosine = dot(view, light);

    float2 result;
    result.x = PhaseFunctionRayleigh(angleCosine);
    result.y = PhaseFunctionHenyeyGreenstein(angleCosine, anisotropy);
    return result;
}

/*************************************
 *
 * NAN STUFF
 *
 *************************************/
float FixNanToZero(float color)
{
    if (isNan(color))
        color = 0.0;
    color = clamp(color, 0.0, 1000.0);
    return color;
}

float3 FixNanToZero(float3 color)
{
    if (isNan(color.r))
        color.r = 1000.0;
    if (isNan(color.g))
        color.g = 1000.0;
    if (isNan(color.b))
        color.b = 1000.0;
    color = clamp(color, float3(0.0, 0.0, 0.0), float3(1000.0, 1000.0, 1000.0));
    return color;
}

float4 FixNanToZero(float4 color)
{
    if (isNan(color.r))
        color.r = 1000.0;
    if (isNan(color.g))
        color.g = 0.0;
    if (isNan(color.b))
        color.b = 1000.0;
    if (isNan(color.a))
        color.a = 0.0;
    color = clamp(color, float4(0.0, 0.0, 0.0, 0.0), float4(1000.0, 1000.0, 1000.0, 1000.0));
    return color;
}

float3 DetectNan(float3 color)
{
    float3 res = float3(0.0, 0.0, 0.0); //color;//
    float3 clamped = clamp(color, float3(0.0, 0.0, 0.0), float3(100000.0, 100000.0, 100000.0));
    if (color.r != clamped.r)
        res = float3(10000.0, 0.0, 0.0);
    if (color.g != clamped.g)
        res = float3(10000.0, 0.0, 0.0);
    if (color.b != clamped.b)
        res = float3(10000.0, 0.0, 0.0);

    return res;
}

float4 EncodeRGBD(float3 inColor)
{
    float maxComponent = max(inColor.x, max(inColor.y, inColor.z));
    float d = max(RGB_ENCODING_MAX_RANGE / maxComponent, 1.0);
    d = saturate(floor(d) / 255.0);
    return float4(inColor.xyz * (d * (255.0 / RGB_ENCODING_MAX_RANGE)), d);
}

float3 DecodeRGBD(float4 inColor)
{
    return inColor.xyz * ((RGB_ENCODING_MAX_RANGE / 255.0) / inColor.w);
}

#if SOFT_SKINNING

float3 JointTransformPosition(float3 inPosition, float4 jQuaternion, float4 jPosition, float jWeight)
{
    float3 tmp = 2.0 * cross(jQuaternion.xyz, inPosition);
    return float3(jPosition.xyz + (inPosition + jQuaternion.w * tmp + cross(jQuaternion.xyz, tmp)) * jPosition.w) * jWeight;
}

float3 JointTransformTangent(float3 tangent, float4 quaternion, float jWeight)
{
    float3 tmp = 2.0 * cross(quaternion.xyz, tangent);
    return tangent + (quaternion.w * tmp + cross(quaternion.xyz, tmp)) * jWeight;
}

#elif HARD_SKINNING

float3 JointTransformTangent(float3 tangent, float4 quaternion)
{
    float3 tmp = 2.0 * cross(quaternion.xyz, tangent);
    return tangent + quaternion.w * tmp + cross(quaternion.xyz, tmp);
}

float3 JointTransformPosition(float3 inPosition, float4 jQuaternion, float4 jPosition)
{
    float3 tmp = 2.0 * cross(jQuaternion.xyz, inPosition);
    return float3(jPosition.xyz + (inPosition + jQuaternion.w * tmp + cross(jQuaternion.xyz, tmp)) * jPosition.w);
}

#endif

float3 rotateVertex(float3 inPos, float3 pivot, float3 rotationAxis, float rotationAngle)
{
    float a = pivot.x;
    float b = pivot.y;
    float c = pivot.z;
    float u = rotationAxis.x;
    float v = rotationAxis.y;
    float w = rotationAxis.z;
    float x = inPos.x;
    float y = inPos.y;
    float z = inPos.z;
    float tr = dot(rotationAxis, inPos);
    float cosTheta = cos(rotationAngle);
    float sinTheta = sin(rotationAngle);
    float3 result = inPos * cosTheta;
    result.x += (a * (v * v + w * w) - u * (b * v + c * w - tr)) * (1.0 - cosTheta) + (-c * v + b * w - y * w + z * v) * sinTheta;
    result.y += (b * (u * u + w * w) - v * (a * u + c * w - tr)) * (1.0 - cosTheta) + (c * u - a * w + x * w - z * u) * sinTheta;
    result.z += (c * (u * u + v * v) - w * (a * u + b * v - tr)) * (1.0 - cosTheta) + (-b * u + a * v - x * v + y * u) * sinTheta;
    return result;
}

float SampleStaticShadow(float sampledValue, float2 uv, float lmSize)
{
#if (VIEW_MODE & VIEW_LIGHTMAP_CANVAS_BIT)
    lmSize = max(1.0, lmSize);
    float cx = floor(fmod(uv.x * lmSize + 0.5, 2.0));
    float cy = floor(fmod(uv.y * lmSize + 0.5, 2.0));
    float checkboard = 0.25 * fmod(cx + fmod(cy, 2.0), 2.0);
    return 0.5 * (sampledValue + checkboard);
#else
    return sampledValue;
#endif
}

float2 Random(float2 seed)
{
    float2 angle = float2(0.0, 0.0);
    angle.x = dot(seed.xy, float2(12.9898, 78.233));
    angle.y = dot(seed.yx, float2(17.3254, 43.968));

    angle = fmod(angle, float2(_PI, _PI));

    return frac(sin(angle) * float2(43758.5453, 85743.3548));
}

float2 RandomCircle(float2 seed)
{
    float3 angle = float3(0.0, 0.0, 0.0);
    angle.x = dot(seed.xy, float2(12.9898, 78.233));
    angle.y = dot(seed.xy, float2(17.3254, 43.968));
    angle.z = dot(seed.xy, float2(14.5462, 61.552));

    angle = fmod(angle, float3(_PI, _PI, _PI));

    float3 random = frac(sin(angle) * float3(43758.5453, 85743.3548, 61261.9634));

    return normalize(random.xy * 2.0 - 1.0) * sqrt(random.z);
}

float2 RotateXY(float2 vec, float2 angleSinCos)
{
    return float2(dot(vec.xy, float2(angleSinCos.y, -angleSinCos.x)), dot(vec.xy, float2(angleSinCos.x, angleSinCos.y)));
}

float3 SoftLightBlend(float3 a, float3 b)
{
    float3 ab2 = 2.0 * a * b;
    return a * (a - ab2) + ab2;
}

float QueryLodLevel(float2 texCoords, float2 texSize)
{
    float2 dx = ddx(texCoords * texSize);
    float2 dy = ddy(texCoords * texSize);
    float dm = max(dot(dx, dx), dot(dy, dy));
    return max(0.0, 0.5 * log2(dm));
}

/// RDB <--> YCoCg color space transitions.
float3 RGBToYCoCg(float3 c)
{
    return float3(c.x / 4.0 + c.y / 2.0 + c.z / 4.0, c.x / 2.0 - c.z / 2.0, -c.x / 4.0 + c.y / 2.0 - c.z / 4.0);
}

float3 YCoCgToRGB(float3 c)
{
    return saturate(float3(c.x + c.y - c.z, c.x + c.z, c.x - c.y - c.z));
}

float4 AnalyticDFGApproximation(float NdotV_, float roughness_)
{
    float b1 = -0.1688;
    float b2 = 1.895;
    float b3 = 0.9903;
    float b4 = -4.853;
    float b5 = 8.404;
    float b6 = -5.069;
    float d0 = 0.6045;
    float d1 = 1.699;
    float d2 = -0.5228;
    float d3 = -3.603;
    float d4 = 1.404;
    float d5 = 0.1939;
    float d6 = 2.661;

    float x = 1.0 - roughness_;
    float y = NdotV_;
    float bias = saturate(min(b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y));
    float delta = saturate(d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x);
    float scale = delta - bias;
    return float4(scale, bias, 1.0, 1.0);
}

float LinearToPerceptualMetallness(in float mtl)
{
    return mtl;
}

float LinearToPerceptualRoughness(in float r)
{
    #define ROUGHNESS_LINEAR_BOUND 0.99
    #define ROUGHENSS_NON_LINEAR_TARGET 3.0

    float mr = max(r, MIN_ROUGHNESS);
    float perceptualRoughness = (mr * mr);

    // remap specular values beyond ROUGHNESS_LINEAR_BOUND
    // from [ROUGHNESS_LINEAR_BOUND ... 1] to [ROUGHNESS_LINEAR_BOUND ... ROUGHENSS_NON_LINEAR_TARGET]
    // this is used to suppress specular highlights on large roughness values
    float nonlinearRoughness = perceptualRoughness + max(0.0, perceptualRoughness - ROUGHNESS_LINEAR_BOUND) * (ROUGHENSS_NON_LINEAR_TARGET / (1.0 - ROUGHNESS_LINEAR_BOUND));

    return nonlinearRoughness;
}

float3 PerformToneMapping(float3 v, float2 dynamicRange, float gain)
{
    dynamicRange *= gain;
    float3 compressed = (v - dynamicRange.x) / (dynamicRange.y - dynamicRange.x);
    float3 scaled = 1.0 - exp(-compressed);
    float3 srgb = LinearTosRGB(scaled);
    return srgb;
}
