#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 worldViewProjMatrix;
uniform mediump vec2 texture0Tiling;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;

#if defined(VERTEX_FOG) || defined(SPECULAR)
uniform mat4 worldViewMatrix;
#endif

uniform vec3 cameraPosition;
uniform mat4 worldMatrix;

#if defined(VERTEX_FOG)
    uniform vec3 fogColor;
    uniform float fogLimit;
    
    uniform float fogAtmosphereDistance;
    uniform samplerCube atmospheremap;
    
    varying float varFogAmoung;
    varying vec3 varFogColor;
    #if defined(FOG_LINEAR)
		uniform float fogStart;
		uniform float fogEnd;
	#else
		uniform float fogDensity;
    #endif
	#if defined(FOG_HALFSPACE)
		uniform float fogHalfspaceHeight;
		uniform float fogHalfspaceFalloff;
		uniform float fogHalfspaceDensity;
		uniform float fogHalfspaceLimit;
	#endif
	#if defined(FOG_GLOW)
        uniform vec3 fogGlowColor;
        uniform float fogGlowDistance;
		uniform float fogGlowScattering;
		varying float varFogGlowFactor;
	#endif
#endif

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
#endif

#if defined(SPECULAR) || defined(VERTEX_FOG)
uniform vec4 lightPosition0;
#endif
#ifdef SPECULAR
uniform mat3 worldViewInvTransposeMatrix;
attribute vec3 inNormal;
attribute vec3 inTangent;

uniform vec3 lightAmbientColor0;
uniform vec3 lightColor0;
uniform mat3 normalMatrix;

uniform float inSpecularity;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;

varying vec3 varSpecularColor;
varying float varNdotH;

const float _PI = 3.141592654;

float FresnelShlick(float NdotL, float Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, fresnel_exponent);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, fresnel_exponent));
}

#endif

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;

	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
	
#if defined(SPECULAR) || defined(VERTEX_FOG)
    vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
    toLightDir = normalize(toLightDir);
#endif
	
#if defined(SPECULAR)
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
    
    vec3 toCameraNormalized = normalize(-eyeCoordsPosition);
    vec3 H = normalize(toLightDir + toCameraNormalized);
    
    float NdotL = max (dot (normal, toLightDir), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightDir, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);
    
    vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    float specularity = inSpecularity;
    
	//varDiffuseColor = NdotL / _PI;
    
    float Dbp = NdotL;
    float Geo = 1.0 / LdotH * LdotH;
    
    varSpecularColor = Dbp * Geo * fresnelOut * specularity * lightColor0;
    varNdotH = NdotH;
#endif
    
#if defined(VERTEX_FOG)
    float fogDistance = length(eyeCoordsPosition);
	
    // calculating fog amoung, depending on distance 
    #if !defined(FOG_LINEAR)
        varFogAmoung = 1.0 - exp(-fogDensity * fogDistance);
    #else
        varFogAmoung = (fogDistance - fogStart) / (fogEnd - fogStart);
    #endif
	varFogAmoung = clamp(varFogAmoung, 0.0, fogLimit);
    
    // calculating view direction in world space, point of view in world space
    #if defined(FOG_HALFSPACE) || defined(FOG_ATMOSPHERE_MAP)
        #if defined(MATERIAL_GRASS_TRANSFORM)
            vec3 viewPointInWorldSpace = vec3(worldMatrix * pos);
        #else
            vec3 viewPointInWorldSpace = vec3(worldMatrix * inPosition);
        #endif
        vec3 viewDirectionInWorldSpace = viewPointInWorldSpace - cameraPosition;
    #endif
    
    // calculating fog color
    #if defined(FOG_ATMOSPHERE_MAP)
        float fogAtmosphereAttenuation = clamp(fogDistance / fogAtmosphereDistance, 0.0, 1.0);
        vec3 viewDirection = normalize(vec3(worldMatrix * inPosition) - cameraPosition);
        lowp vec4 atmosphereColor = textureCubeLod(atmospheremap, viewDirection, 0);
        varFogColor = mix(fogColor, atmosphereColor.rgb, fogAtmosphereAttenuation);
    #else
        varFogColor = fogColor;
    #endif
    
    // calculating halfSpaceFog amoung
    #if defined(FOG_HALFSPACE)
        #if defined(FOG_HALFSPACE_LINEAR)
            float fogK = (cameraPosition.z < fogHalfspaceHeight) ? 1.0 : 0.0;
            float fogFdotP = viewPointInWorldSpace.z - fogHalfspaceHeight;
            float fogFdotC = cameraPosition.z - fogHalfspaceHeight;
            
            vec3 fogAV = viewDirectionInWorldSpace * fogHalfspaceDensity;
            float fogC1 = fogK * (fogFdotP + fogFdotC);
            float fogC2 = (1.0 - 2.0 * fogK) * fogFdotP;
            
            float fogG = min(fogC2, 0.0);
            fogG = -length(fogAV) * (fogC1 - fogG * fogG / abs(viewDirectionInWorldSpace.z));
            
            float halfSpaceFogAmoung = clamp(1.0 - exp2(-fogG), 0.0, fogHalfspaceLimit);
        #else
            float fogK = viewDirectionInWorldSpace.z / fogDistance;
            float fogB = cameraPosition.z - fogHalfspaceHeight;
            
            float halfSpaceFogAmoung = clamp(fogHalfspaceDensity * exp(-fogHalfspaceFalloff * fogB) * (1.0 - exp(-fogHalfspaceFalloff * fogK * fogDistance)) / fogK, 0.0, fogHalfspaceLimit);
        #endif
        varFogAmoung = max(varFogAmoung, halfSpaceFogAmoung);
    #endif
    
    #if defined(FOG_GLOW)
        toLightDir = normalize(toLightDir);
        float fogGlowDistanceAttenuation = clamp(fogDistance / fogGlowDistance, 0.0, 1.0);
        varFogGlowFactor = pow(dot(toLightDir, normalize(eyeCoordsPosition)) * 0.5 + 0.5, fogGlowScattering) * fogGlowDistanceAttenuation;
        varFogColor = mix(varFogColor, fogGlowColor, varFogGlowFactor);
    #endif
#endif

	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
