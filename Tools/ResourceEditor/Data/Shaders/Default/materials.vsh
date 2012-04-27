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
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT)
uniform float materialSpecularShininess;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
//implemented only for lightmaps in code
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
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
varying float varPerPixelAttenuation;
#endif

#if defined(SETUP_LIGHTMAP)
uniform float lightmapSize;
varying lowp float varLightmapSize;
#endif


void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
#if defined(VERTEX_LIT)
    vec3 eyeCoordsPosition = vec3(modelViewMatrix * inPosition);
    vec3 normal = normalize(normalMatrix * inNormal); // normal in eye coordinates
    vec3 lightDir = lightPosition0 - eyeCoordsPosition;
    float attenuation = length(lightDir);
    attenuation = lightIntensity0 / (attenuation * attenuation); // use inverse distance for distance attenuation
    lightDir = normalize(lightDir);
    
    varDiffuseColor = max(0.0, dot(normal, lightDir)) * attenuation;

    // Blinn-phong reflection
    vec3 E = normalize(-eyeCoordsPosition);
    vec3 H = normalize(lightDir + E);
    float nDotHV = max(0.0, dot(normal, H));
    
    /*
        Phong Reflection
        vec3 E = normalize(-eyeCoordsPosition);
        vec3 L = lightDir;
        vec3 R = reflect(-L, normal);
        float nDotHV = max(0.0, dot(E, R));
    */
    
    varSpecularColor = pow(nDotHV, materialSpecularShininess) * attenuation;
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (normalMatrix * inNormal);
	vec3 t = normalize (normalMatrix * inTangent);
	vec3 b = -cross (n, t);

    vec3 vertexPosition = vec3(modelViewMatrix *  inPosition);
    
    vec3 lightDir = lightPosition0 - vertexPosition;
    varPerPixelAttenuation = length(lightDir);
    lightDir = normalize(lightDir);
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (lightDir, t);
	v.y = dot (lightDir, b);
	v.z = dot (lightDir, n);
	varLightVec = normalize (v);

    // vertexPosition = -vertexPosition;
	// v.x = dot (vertexPosition, t);
	// v.y = dot (vertexPosition, b);
	// v.z = dot (vertexPosition, n);
	// varEyeVec = normalize (v);

    vertexPosition = normalize(-vertexPosition);

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
	
	#if defined(SETUP_LIGHTMAP)
		varLightmapSize = lightmapSize;
		varTexCoord1 = inTexCoord1;
	#else
		varTexCoord1 = uvScale*inTexCoord1+uvOffset;
	#endif
#endif
}
