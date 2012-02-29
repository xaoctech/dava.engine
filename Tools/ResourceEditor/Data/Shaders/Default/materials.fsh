#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


// DECLARATIONS
uniform sampler2D texture0;
varying mediump vec2 varTexCoord0;

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) 
uniform sampler2D texture1;
varying mediump vec2 varTexCoord1;
#endif 

#if defined(PIXEL_LIT)
uniform sampler2D normalMapTexture;
uniform float materialSpecularShininess;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform vec3 materialLightAmbientColor;     // engine pass premultiplied material * light ambient color
uniform vec3 materialLightDiffuseColor;     // engine pass premultiplied material * light diffuse color
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif 

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
varying vec3 varHalfVec;
varying vec3 varEyeVec;
#endif


void main()
{
    // FETCH PHASE
#if defined(GLOSS)
    lowp vec4 textureColor0 = texture2D(texture0, varTexCoord0);
#else
    lowp vec3 textureColor0 = texture2D(texture0, varTexCoord0).rgb;
#endif
#if defined(OPAQUE)
    if (textureColor0.a < 0.9)discard;
#endif
    
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
    lowp vec3 textureColor1 = texture2D(texture1, varTexCoord1).rgb;
#endif

    // DRAW PHASE
#if defined(VERTEX_LIT)
	vec3 color = (materialLightAmbientColor + varDiffuseColor * materialLightDiffuseColor) * textureColor0 + varSpecularColor * materialLightSpecularColor;
#elif defined(PIXEL_LIT)
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
    vec3 normal = 2.0 * texture2D (normalMapTexture, varTexCoord0).rgb - 1.0;
    normal = normalize (normal);

    // compute diffuse lighting
    float lambertFactor= max (dot (varLightVec, normal), 0.0);

    // compute ambient
    vec3 color = materialLightAmbientColor + materialLightDiffuseColor * lambertFactor;	
	color *= textureColor0.rgb;

#if defined(SPECULAR)
	//if (lambertFactor > 0.0)
	{
		// In doom3, specular value comes from a texture 
		float shininess = pow (max (dot (varHalfVec, normal), 0.0), materialSpecularShininess);
		#if defined(GLOSS)
    		color += materialLightSpecularColor * (shininess * textureColor0.a);
    	#else 
		    color += materialLightSpecularColor * shininess;
    	#endif
	}
#endif
	
#elif defined(MATERIAL_TEXTURE)
    vec3 color = textureColor0;
#elif defined(MATERIAL_DECAL)
    vec3 color = textureColor0 * textureColor1;
#elif defined(MATERIAL_DETAIL)
    vec3 color = textureColor0 * textureColor1 * 2.0;
#endif

    gl_FragColor = vec4(color, 1.0);
}
