#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec3 inTangent;

attribute vec2 inTexCoord0;
varying mediump vec2 varTexCoord0;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	varTexCoord0 = inTexCoord0;
	gl_Position = projectionMatrix * (inPosition + vec4(modelViewMatrix[3].xyz, 0.0)) + modelViewProjectionMatrix * vec4(inTangent, 0.0);
}
