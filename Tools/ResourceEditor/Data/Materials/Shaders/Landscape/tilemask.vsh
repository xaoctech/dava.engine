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
    
    uniform float fogDistance;
    uniform samplerCube fogGlowCubemap;
    
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
    float fogFragCoord = length(eyeCoordsPosition);
	
    #if !defined(FOG_LINEAR)
        //const float LOG2 = 1.442695;
        //varFogAmoung = 1.0 - exp2(-fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
        varFogAmoung = 1.0 - exp(-fogDensity * fogFragCoord);
    #else
        varFogAmoung = (fogFragCoord - fogStart) / (fogEnd - fogStart);
    #endif
	varFogAmoung = clamp(varFogAmoung, 0.0, fogLimit);
    
    // fog color
    float fogDistanceAttenuation = clamp(fogFragCoord / fogDistance, 0.0, 1.0);
    vec3 viewDirection = normalize(vec3(worldMatrix * inPosition) - cameraPosition);
    lowp vec4 cubemapColor = textureCubeLod(fogGlowCubemap, viewDirection, 0);
    varFogColor = mix(fogColor, cubemapColor.rgb, fogDistanceAttenuation);
	
	#if defined(FOG_HALFSPACE)
		float halfSpaceFogAmoung;
		vec3 C = cameraPosition;
		vec3 P = vec3(worldMatrix * inPosition);
		vec3 V = (P - C);
		
		#if defined(FOG_HALFSPACE_LINEAR)
			float fogK = (C.z < fogHalfspaceHeight ) ? 1.0 : 0.0;
			
			float FdotP = P.z - fogHalfspaceHeight;
			float FdotC = C.z - fogHalfspaceHeight;
			
			vec3 aV = V * fogHalfspaceDensity;
			float c1 = fogK * (FdotP + FdotC);
			float c2 = (1.0 - 2.0 * fogK) * FdotP;
			
			float g = min(c2, 0.0);
			g = -length(aV) * (c1 - g * g / abs(V.z));
			
			halfSpaceFogAmoung = clamp(1.0 - exp2(-g), 0.0, fogHalfspaceLimit);
		#else
			float fogK = (P.z - C.z) / fogFragCoord;
			float fogB = C.z - fogHalfspaceHeight;
			halfSpaceFogAmoung = clamp(fogHalfspaceDensity * exp(-fogHalfspaceFalloff * fogB) * (1.0 - exp(-fogHalfspaceFalloff * fogFragCoord * fogK)) / fogK, 0.0, fogHalfspaceLimit);
		#endif
		
		varFogAmoung = max(varFogAmoung, halfSpaceFogAmoung);
	#endif
	
	#if defined(FOG_GLOW)
		toLightDir = normalize(toLightDir);
        float fogGlowDistanceAttenuation = clamp(fogFragCoord / fogGlowDistance, 0.0, 1.0);
		varFogGlowFactor = pow(dot(toLightDir, normalize(eyeCoordsPosition)) * 0.5 + 0.5, fogGlowScattering) * fogGlowDistanceAttenuation;
        varFogColor = mix(varFogColor, fogGlowColor, varFogGlowFactor);
	#endif
#endif
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
