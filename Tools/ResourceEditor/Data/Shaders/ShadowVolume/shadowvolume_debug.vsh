attribute vec4 inPosition;

uniform mat4 modelViewProjectionMatrix;

void main()
{
	vec4 tv = vec4(inPosition.xyz * 2.0, 1.0);
	gl_Position = modelViewProjectionMatrix * tv;
}
