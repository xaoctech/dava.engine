attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 modelViewProjectionMatrix;
uniform vec3 cameraPosition;
varying vec4 varColor;
varying vec2 varTexCoordOrig;
varying vec2 varTexCoord0;
varying vec2 varTexCoord1;

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
	varTexCoordOrig = inTexCoord0;
	varTexCoord0 = inTexCoord0 * 10.0;
	varTexCoord1 = inTexCoord0 * 20.0;
}
