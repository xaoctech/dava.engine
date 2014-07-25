#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif



// INPUT ATTRIBUTES
attribute vec4 inPosition;
attribute vec3 inNormal;


#if defined(PIXEL_LIT)
attribute vec3 inTangent;
#endif

attribute vec2 inTexCoord0;

#if defined(MATERIAL_DECAL)
attribute vec2 inTexCoord1;
varying vec2 varTexCoordDecal;
#endif

// UNIFORMS
uniform mat4 worldViewProjMatrix;


uniform mat4 worldViewMatrix;


#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG)
uniform vec4 lightPosition0;
#endif

#if defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;
uniform float lightIntensity0; 
uniform float materialSpecularShininess;
#endif

#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
varying vec3 eyeDist;
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
#endif

#if defined(PIXEL_LIT)||defined(MATERIAL_DECAL)	
varying highp vec2 varTexCoord0;
varying highp vec2 varTexCoord1;
uniform mediump vec2 normal0ShiftPerSecond;
uniform mediump vec2 normal1ShiftPerSecond;
uniform mediump float normal0Scale;
uniform mediump float normal1Scale;
uniform float globalTime;
#endif

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

uniform vec3 cameraPosition;
uniform mat4 worldMatrix;
uniform mat3 worldInvTransposeMatrix;





#if defined(VERTEX_LIT)
	varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
	varying mediump vec3 cameraToPointInTangentSpace;
	varying mediump mat3 tbnToWorldMatrix;
#endif





void main()
{
    
    gl_Position = worldViewProjMatrix * inPosition;	

#if defined(VERTEX_LIT)    
    vec3 viewDirectionInWorldSpace = vec3(worldMatrix * inPosition) - cameraPosition;
    vec3 normalDirectionInWorldSpace = normalize(vec3(worldInvTransposeMatrix * inNormal));
    reflectionDirectionInWorldSpace = reflect(viewDirectionInWorldSpace, normalDirectionInWorldSpace);
	#if defined(MATERIAL_DECAL)
		varTexCoordDecal = inTexCoord1;		
	#endif	
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG)
	vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = cross (n, t);

	#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
		eyeDist = eyeCoordsPosition;
	#endif
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (toLightDir, t);
	v.y = dot (toLightDir, b);
	v.z = dot (toLightDir, n);
	varLightVec = v;       

    v.x = dot (eyeCoordsPosition, t);
	v.y = dot (eyeCoordsPosition, b);
	v.z = dot (eyeCoordsPosition, n);
	cameraToPointInTangentSpace = v;
    
    vec3 binormTS = cross(inNormal, inTangent);
    tbnToWorldMatrix = mat3(inTangent, binormTS, inNormal);
#endif
	
#if defined(PIXEL_LIT)||defined(MATERIAL_DECAL)	
	varTexCoord0 = inTexCoord0 * normal0Scale + normal0ShiftPerSecond * globalTime;
    varTexCoord1 = vec2(inTexCoord0.x+inTexCoord0.y, inTexCoord0.x-inTexCoord0.y) * normal1Scale + normal1ShiftPerSecond * globalTime;
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
		#if defined(MATERIAL_GRASS_TRANSFORM)
			vec3 P = vec3(worldMatrix * pos);
		#else
			vec3 P = vec3(worldMatrix * inPosition);
		#endif
		vec3 V = (P - C);
		
		#if defined(FOG_HALFSPACE_LINEAR)
			float fogK = (C.z < fogHalfspaceHeight) ? 1.0 : 0.0;
			
			float FdotP = P.z - fogHalfspaceHeight;
			float FdotC = C.z - fogHalfspaceHeight;
			
			vec3 aV = V * fogHalfspaceDensity;
			float c1 = fogK * (FdotP + FdotC);
			float c2 = (1.0 - 2.0 * fogK) * FdotP;
			
			float g = min(c2, 0.0);
			g = -length(aV) * (c1 - g * g / abs(V.z));
			
			halfSpaceFogAmoung = clamp(1.0 - exp2(-g), 0.0, fogHalfspaceLimit);
		#else
			//float ExponentialFogParametersX = fogDensity * exp2(-fogHalfspaceDensity * (C.z - fogHalfspaceHeight));
			//float EffectiveZ = (abs(V.z) > 0.001) ? V.z : 0.001;
			//float Falloff = max( -127.0f, fogHalfspaceDensity * EffectiveZ );	// if it's lower than -127.0, then exp2() goes crazy in OpenGL's GLSL.
			//float ExponentialHeightLineIntegralShared = ExponentialFogParametersX * (1.0f - exp2(-Falloff) ) / Falloff;
			//varFogAmoung = 1.0 - exp2(-ExponentialHeightLineIntegralShared);
			
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

    
}
