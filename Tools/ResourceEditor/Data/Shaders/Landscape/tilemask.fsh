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

varying lowp vec2 varTexCoordOrig;
varying lowp vec2 varTexCoord0;
varying lowp vec2 varTexCoord1;
varying lowp vec2 varTexCoord2;
varying lowp vec2 varTexCoord3;

void main()
{
    lowp vec3 color0 = texture2D(tileTexture0, varTexCoord0).rgb;
    lowp vec3 color1 = texture2D(tileTexture1, varTexCoord1).rgb;
    lowp vec3 color2 = texture2D(tileTexture2, varTexCoord2).rgb;
    lowp vec3 color3 = texture2D(tileTexture3, varTexCoord3).rgb;
    
    lowp vec4 mask = texture2D(tileMask, varTexCoordOrig);
    lowp vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

    lowp vec3 color = (mask.r * color0 + mask.g * color1 + mask.b * color2 + mask.a * color3)*lightMask.rgb;
    gl_FragColor = vec4(color, 1.0);
}
