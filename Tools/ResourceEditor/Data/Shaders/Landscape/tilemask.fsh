#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

uniform sampler2D tileTexture0;
uniform sampler2D tileTexture1;
uniform sampler2D tileTexture2;
uniform sampler2D tileTexture3;
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

varying vec2 varTexCoordOrig;
varying vec2 varTexCoord0;
varying vec2 varTexCoord1;
varying vec2 varTexCoord2;
varying vec2 varTexCoord3;
varying vec3 varCameraToVertex;

void main()
{
    vec3 color0 = texture2D(tileTexture0, varTexCoord0).rgb;
    vec3 color1 = texture2D(tileTexture1, varTexCoord1).rgb;
    vec3 color2 = texture2D(tileTexture2, varTexCoord2).rgb;
    vec3 color3 = texture2D(tileTexture3, varTexCoord3).rgb;
    
    vec4 mask = texture2D(tileMask, varTexCoordOrig);
    vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

    vec3 color = (mask.r * color0 + mask.g * color1.rgb + mask.b * color2.rgb + mask.a * color3.rgb);
    gl_FragColor = vec4(mask.rgb, 1.0);
}
