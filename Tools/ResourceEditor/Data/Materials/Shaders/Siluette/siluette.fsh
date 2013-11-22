#ifdef GL_ES
precision lowp float;
#endif

uniform vec4 siluetteColor;

void main()
{
	gl_FragColor = siluetteColor;
}
