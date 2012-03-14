#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 modelViewProjectionMatrix;
uniform lowp vec2 texture0Tiling;
uniform lowp vec2 texture1Tiling;
uniform lowp vec2 texture2Tiling;
uniform lowp vec2 texture3Tiling;

varying lowp vec2 varTexCoordOrig;
varying lowp vec2 varTexCoord0;
varying lowp vec2 varTexCoord1;
varying lowp vec2 varTexCoord2;
varying lowp vec2 varTexCoord3;

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;

	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
	varTexCoord1 = inTexCoord0 * texture1Tiling;
	varTexCoord2 = inTexCoord0 * texture2Tiling;
	varTexCoord3 = inTexCoord0 * texture3Tiling;
}
