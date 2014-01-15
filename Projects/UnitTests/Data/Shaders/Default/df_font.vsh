#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform mat4 modelViewProjectionMatrix;
attribute vec4 inPosition;
attribute vec2 inTexCoord0;

varying vec2 v_texCoord;

void main() {
    gl_Position = modelViewProjectionMatrix * inPosition;
    v_texCoord = inTexCoord0;
}