//uniform sampler2D u_texture;
uniform sampler2D texture0;

varying vec2 v_texCoord;

uniform float smoothing;
uniform vec4 color;

void main() {
    float distance = texture2D(texture0, v_texCoord).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    alpha = min(alpha, color.a);
    gl_FragColor = vec4(color.rgb, alpha);
}