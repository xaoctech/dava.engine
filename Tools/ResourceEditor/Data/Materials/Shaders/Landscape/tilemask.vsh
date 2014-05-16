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
	varying float varFogFactor;
    #if !defined(FOG_LINEAR)
    uniform float fogDensity;
    #else
    uniform float fogStart;
    uniform float fogEnd;
    #endif
	#if defined(FOG_HALFSPACE)
	uniform float fogHalfspaceHeight;
	uniform float fogHalfspaceDensity;
	#endif
	#if defined(FOG_GLOW)
	uniform float fogGlowScattering;
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
        varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
		varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
    #else
        varFogFactor = 1.0 - clamp((fogFragCoord - fogStart) / (fogEnd - fogStart), 0.0, fogLimit);
    #endif
	
	vec3 C = cameraPosition;
	vec3 P = vec3(worldMatrix * inPosition);
	vec3 V = (P - C);

	float H = 80;
	float fogK = 0;
	if(C.z < H ) fogK = 1;
	
	float FdotP = P.z - H;
	float FdotC = C.z - H;
	
	vec3 aV = V * fogDensity;// * fogDensity;
	float c1 = fogK * (FdotP + FdotC);
	float c2 = (1 - 2 * fogK) * FdotP;
	
	float g = min(c2, 0.0);
	g = -length(aV) * (c1 - g * g / abs(V.z));
	
	varFogFactor = clamp(exp2(-g), 0, 1);
	varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);


	//float fogF = clamp(fogK - (P.z - 30) / fogFragCoord, 0.0, 1.0);
	//float L = fogF * fogFragCoord;
	//varFogFactor = clamp(exp2(-L * fogDensity), 0, 1.0);
	//varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
	
	//float aV = 0.0004 * fogFragCoord;
	//float c1 = fogK * (dot(F, P) + dot(F, C));
	//float c2 = (1 - 2 * fogK) * dot(F, P);
	//float dv = abs(dot(F, V));
	
	//float g = min(c2, 0);
	//g = -aV * (c1 - g * g / dv);

	//varFogFactor = 1.0 - clamp(exp2(-g), 0, 1.0);
	//varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
	//varFogFactor = fogK;
	
	//varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
	//varFogFactor = L;
	
	//varFogFactor = (1.0 * exp(-C.z * fogDensity) * (1.0 - exp(-fogFragCoord * V.z * fogDensity)) / V.z);
	//varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
	
	//float B = ;
	//varFogFactor = 1.0 - 10 * exp(-C.z * fogDensity) * (1.0 - exp(-V.z * fogFragCoord * fogDensity)) / V.z;
	//varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
	
	
	#if defined(FOG_GLOW)
		toLightDir = normalize(toLightDir);
		varFogGlowFactor = pow(dot(toLightDir, normalize(eyeCoordsPosition)) * 0.5 + 0.5, fogGlowScattering);
	#endif
#endif
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
