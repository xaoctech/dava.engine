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

#if defined(VERTEX_LIT)
attribute vec3 inNormal;
#endif 

attribute vec2 inTexCoord0;

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
attribute vec2 inTexCoord1;
#endif


#if defined(VERTEX_LIT)
vec3 lightPosition0 = vec3(1.0, 1.0, 1.0); 
#endif


// UNIFORMS
uniform mat4 modelViewProjectionMatrix;

#if defined(VERTEX_LIT)
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
#endif

// OUTPUT ATTRIBUTES
varying vec2 varTexCoord0;

#if defined(VERTEX_LIT)
varying lowp vec4 varDiffuseColor;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
varying vec2 varTexCoord1;
#endif

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
#if defined(VERTEX_LIT)
    vec3 normal = normalMatrix * inNormal; // normal in eye coordinates
    vec3 lightDirection = normalize(lightPosition0);
    float diffuse = max(0.0, dot(normal, lightDirection));
    varDiffuseColor = vec4(0.0, 0.0, 0.0, 1.0);
#endif
	varTexCoord0 = inTexCoord0;
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
	varTexCoord1 = inTexCoord1;
#endif
}
