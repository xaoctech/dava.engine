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

#if defined(VERTEX_FOG) || defined(SPECULAR_LAND)
uniform mat4 worldViewMatrix;
#endif

uniform vec3 cameraPosition;
uniform mat4 worldMatrix;

#if defined(VERTEX_FOG)
    uniform float fogLimit;
	varying float varFogAmoung;
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
		uniform float fogGlowScattering;
		uniform float fogGlowDistance;
		varying float varFogGlowFactor;
	#endif
#endif

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
#endif

#if defined(SPECULAR_LAND) || defined(VERTEX_FOG)
uniform vec4 lightPosition0;
#endif
#if defined(SPECULAR_LAND)
attribute vec3 inNormal;
uniform mat3 normalMatrix;
uniform float lightIntensity0;
uniform float materialSpecularShininess;
varying float varSpecularColor;
#endif

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;

	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
	
#if defined(SPECULAR_LAND) || defined(VERTEX_FOG)
	vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#endif
	
#if defined(SPECULAR_LAND)
    vec3 normal = normalize(normalMatrix * inNormal); // normal in eye coordinates
    
    // Blinn-phong reflection
    vec3 E = normalize(-eyeCoordsPosition);
    vec3 H = normalize(toLightDir + E);
    float nDotHV = max(0.0, dot(normal, H));
    
    varSpecularColor = pow(nDotHV, materialSpecularShininess);
#endif
    
#if defined(VERTEX_FOG)
    float fogFragCoord = length(eyeCoordsPosition);
	
    #if !defined(FOG_LINEAR)
        const float LOG2 = 1.442695;
        varFogAmoung = 1.0 - exp2(-fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
    #else
        varFogAmoung = (fogFragCoord - fogStart) / (fogEnd - fogStart);
    #endif
	
	varFogAmoung = clamp(varFogAmoung, 0.0, fogLimit);
	
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
	#endif
#endif
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
