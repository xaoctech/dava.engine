#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 modelViewProjectionMatrix;
uniform mediump vec2 texture0Tiling;
uniform mediump vec2 texture1Tiling;
uniform mediump vec2 texture2Tiling;
uniform mediump vec2 texture3Tiling;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;
varying mediump vec2 varTexCoord1;
varying mediump vec2 varTexCoord2;
varying mediump vec2 varTexCoord3;

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
#endif

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;

	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
	varTexCoord1 = inTexCoord0 * texture1Tiling;
	varTexCoord2 = inTexCoord0 * texture2Tiling;
	varTexCoord3 = inTexCoord0 * texture3Tiling;
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
