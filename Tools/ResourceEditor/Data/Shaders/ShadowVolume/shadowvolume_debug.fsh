#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

void main()
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
