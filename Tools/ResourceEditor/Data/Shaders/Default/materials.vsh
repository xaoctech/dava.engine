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

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
attribute vec3 inNormal;
#endif 

attribute vec2 inTexCoord0;

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
attribute vec2 inTexCoord1;
#endif


#if defined(VERTEX_LIT)
#endif

#if defined(PIXEL_LIT)
attribute vec3 inTangent;
#endif


// UNIFORMS
uniform mat4 modelViewProjectionMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightPosition0; 
#endif

#if defined(VERTEX_LIT)
uniform float materialSpecularShininess;
#endif


// OUTPUT ATTRIBUTES
varying vec2 varTexCoord0;

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
varying vec2 varTexCoord1;
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
varying vec3 varHalfVec;
varying vec3 varEyeVec;
#endif

#if defined(SETUP_LIGHTMAP)
uniform int lightmapSize;
varying lowp float varLightmapSize;
#endif

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
#if defined(VERTEX_LIT)
    vec4 ecPosition = modelViewMatrix * inPosition;
    vec3 ecPosition3 = vec3(ecPosition);
    vec3 normal = normalize(normalMatrix * inNormal); // normal in eye coordinates
    vec3 VP = lightPosition0 - ecPosition3;
    float attenuation = length(VP);
    attenuation = 500.0 / (attenuation * attenuation); // use inverse distance for distance attenuation
    VP = normalize(VP);
    
    varDiffuseColor = max(0.0, dot(normal, VP)) * attenuation;

    vec3 eye = vec3(0.0, 0.0, 1.0);
    vec3 halfVector = normalize(VP + eye);
    float nDotHV = max(0.0, dot(normal, halfVector));
    varSpecularColor = pow(nDotHV, materialSpecularShininess) * attenuation;
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (normalMatrix * inNormal);
	vec3 t = normalize (normalMatrix * inTangent);
	vec3 b = cross (n, t);

    vec3 vertexPosition = vec3(modelViewMatrix *  inPosition);
    vec3 lightDir = normalize(lightPosition0 - vertexPosition);

	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (lightDir, t);
	v.y = dot (lightDir, b);
	v.z = dot (lightDir, n);
	varLightVec = normalize (v);

	v.x = dot (vertexPosition, t);
	v.y = dot (vertexPosition, b);
	v.z = dot (vertexPosition, n);
	varEyeVec = normalize (v);

    vertexPosition = normalize(vertexPosition);

	/* Normalize the halfVector to pass it to the fragment shader */

	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((vertexPosition + lightDir) / 2.0); 
	vec3 halfVector = normalize(vertexPosition + lightDir);
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);

	// No need to normalize, t,b,n and halfVector are normal vectors.
	//normalize (v);
	varHalfVec = v;
#endif

	varTexCoord0 = inTexCoord0;
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
	varTexCoord1 = inTexCoord1;
	#if defined(SETUP_LIGHTMAP)
		varLightmapSize = lightmapSize;
	#endif
#endif
}
