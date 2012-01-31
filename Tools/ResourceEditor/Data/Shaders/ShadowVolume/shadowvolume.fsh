#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

varying vec3 varColor;

void main()
{
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	gl_FragColor = vec4(varColor * 0.5 + 0.5, 1.0);
}
