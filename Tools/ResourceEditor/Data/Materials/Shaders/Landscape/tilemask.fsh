<CONFIG>
uniform sampler2D tileTexture0 = 2;
uniform sampler2D tileMask = 1;
uniform sampler2D colorTexture = 0;
uniform sampler2D specularMap = 6;
uniform samplerCube fogGlowCubemap = 7;
<FRAGMENT_SHADER>
#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

#ifdef SPECULAR
uniform sampler2D specularMap;
uniform float inGlossiness;

varying vec3 varSpecularColor;
varying float varNdotH;
#endif

uniform lowp vec3 tileColor0;
uniform lowp vec3 tileColor1;
uniform lowp vec3 tileColor2;
uniform lowp vec3 tileColor3;

uniform sampler2D tileTexture0;
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
uniform sampler2D cursorTexture;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogAmoung;
#if defined(FOG_GLOW)
uniform vec3 fogGlowColor;
uniform float fogK;
uniform samplerCube fogGlowCubemap;
varying float varFogGlowFactor;
varying float varFogGlowDistanceAttenuation;
varying vec3 viewDirection;
#endif
#endif

void main()
{
    lowp vec4 color0 = texture2D(tileTexture0, varTexCoord0).rgba;
    lowp vec4 mask = texture2D(tileMask, varTexCoordOrig);
    lowp vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

    lowp vec3 color = (color0.r*mask.r*tileColor0 + color0.g*mask.g*tileColor1 + color0.b*mask.b*tileColor2 + color0.a*mask.a*tileColor3) * lightMask.rgb * 2.0;
    
#ifdef EDITOR_CURSOR
	vec4 colorCursor = texture2D(cursorTexture, varTexCoordCursor);
	color *= 1.0-colorCursor.a;
	color += colorCursor.rgb*colorCursor.a;
#endif
	
#ifdef SPECULAR
	float glossiness = pow(5000.0, inGlossiness * color0.a);
    float specularNorm = (glossiness + 2.0) / 8.0;
    color += varSpecularColor * pow(varNdotH, glossiness) * specularNorm;
#endif
    //color = vec3(1.0);

#if defined(VERTEX_FOG)
	#if defined(FOG_GLOW)
        lowp vec4 atmoColor = textureCube(fogGlowCubemap, viewDirection);
        vec3 ccc = mix(atmoColor.rgb, fogGlowColor, varFogGlowFactor);
        vec3 realFogColor = mix(fogColor, ccc, varFogGlowDistanceAttenuation)  * fogK;
        
		//vec3 realFogColor = mix(fogColor, fogGlowColor, varFogGlowFactor);
		gl_FragColor.rgb = mix(color, realFogColor, varFogAmoung);
	#else
		gl_FragColor.rgb = mix(color, fogColor, varFogAmoung);
	#endif
#else
    gl_FragColor = vec4(color, 1.0);
#endif
    //gl_FragColor = vec4(varSpecularColor, 1.0);
}
