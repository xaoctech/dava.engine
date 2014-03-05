<CONFIG>
uniform sampler2D albedo = 0;
uniform sampler2D cubemap = 2;
uniform sampler2D decal = 1;
uniform sampler2D detail = 1;
uniform sampler2D lightmap = 1;
uniform sampler2D normalmap = 2;

uniform float inGlossiness = 0.5;
uniform float inSpecularity = 1.0;
uniform vec3 metalFresnelReflectance = vec3(0.5, 0.5, 0.5);
<FRAGMENT_SHADER>

#extension GL_ARB_shader_texture_lod : enable

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

//#define ALPHABLEND
//#define FLATCOLOR
//#define VERTEX_FOG

const float _PI = 3.141592654;

// DECLARATIONS
#if defined(MATERIAL_TEXTURE)
uniform sampler2D albedo;
varying mediump vec2 varTexCoord0;
#elif defined(MATERIAL_SKYBOX)
uniform samplerCube cubemap;
varying mediump vec3 varTexCoord0;
#endif

#if defined(REFLECTION)
uniform samplerCube cubemap;
#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
#endif
#endif

#if defined(MATERIAL_DECAL)
uniform sampler2D decal;
#endif

#if defined(MATERIAL_DETAIL)
uniform sampler2D detail;
#endif

//#if defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
#if defined(MATERIAL_LIGHTMAP)
uniform sampler2D lightmap;
#endif

//#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY) || defined(FRAME_BLEND)
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND)
varying highp vec2 varTexCoord1;
#endif

#if defined(PIXEL_LIT)
uniform sampler2D normalmap;
uniform float materialSpecularShininess;
uniform float lightIntensity0;
uniform float inSpecularity;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform vec3 materialLightAmbientColor;     // engine pass premultiplied material * light ambient color
uniform vec3 materialLightDiffuseColor;     // engine pass premultiplied material * light diffuse color
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
uniform float inGlossiness;
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;

#if defined(BLINN_PHONG)
varying lowp float varSpecularColor;
#elif defined(NORMALIZED_BLINN_PHONG)
varying lowp vec3 varSpecularColor;
varying float varNdotH;
#endif
#endif

#if defined(PIXEL_LIT)
varying vec3 varToLightVec;

    #if defined(FAST_NORMALIZATION)
    varying vec3 varHalfVec;
    #endif

varying vec3 varToCameraVec;
uniform vec4 lightPosition0;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif

#if defined(SPEED_TREE_LEAF)
uniform lowp vec3 treeLeafColorMul;
uniform lowp float treeLeafOcclusionOffset;
uniform lowp float treeLeafOcclusionMul;
#endif

#if defined(SETUP_LIGHTMAP)
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(FLATCOLOR)
uniform lowp vec4 flatColor;
#endif


float FresnelShlick(float NdotL, float Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, fresnel_exponent);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, fresnel_exponent));
}

void main()
{
    // FETCH PHASE
#if defined(MATERIAL_TEXTURE)
    
    #if defined(PIXEL_LIT) || defined(ALPHATEST) || defined(ALPHABLEND) || defined(VERTEX_LIT)
        lowp vec4 textureColor0 = texture2D(albedo, varTexCoord0);
    #else
        lowp vec3 textureColor0 = texture2D(albedo, varTexCoord0).rgb;
    #endif
    
    #if defined(FRAME_BLEND)
        lowp vec4 blendFrameColor = texture2D(albedo, varTexCoord1);
        textureColor0 = mix(textureColor0, blendFrameColor, varTime);
    #endif
    
#elif defined(MATERIAL_SKYBOX)
    lowp vec4 textureColor0 = textureCube(cubemap, varTexCoord0);
#endif
    
#if defined(MATERIAL_TEXTURE)
    #if defined(ALPHATEST)
        float alpha = textureColor0.a;
        #if defined(VERTEX_COLOR)
            alpha *= varVertexColor.a;
        #endif
        if (alpha < 0.5)discard;
    #endif
    #if defined(ALPHATESTVALUE)
        float alpha = textureColor0.a;
        #if defined(VERTEX_COLOR)
            alpha *= varVertexColor.a;
        #endif
        if (alpha < ALPHATESTVALUE)discard;
    #endif
#endif
    
#if defined(MATERIAL_DECAL)
    lowp vec3 textureColor1 = texture2D(decal, varTexCoord1).rgb;
#endif
    
#if defined(MATERIAL_DETAIL)
    lowp vec3 textureColor1 = texture2D(detail, varTexCoord1).rgb;
#endif
    
//#if defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
#if defined(MATERIAL_LIGHTMAP)
    lowp vec3 textureColor1 = texture2D(lightmap, varTexCoord1).rgb;
#endif
    
//#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
    #if defined(SETUP_LIGHTMAP)
        vec3 lightGray = vec3(0.75, 0.75, 0.75);
        vec3 darkGray = vec3(0.25, 0.25, 0.25);

        bool isXodd;
        bool isYodd;
        if(fract(floor(varTexCoord1.x*varLightmapSize)/2.0) == 0.0)
        {
            isXodd = true;
        }
        else
        {
            isXodd = false;
        }
        if(fract(floor(varTexCoord1.y*varLightmapSize)/2.0) == 0.0)
        {
            isYodd = true;
        }
        else
        {
            isYodd = false;
        }
    
        if((isXodd && isYodd) || (!isXodd && !isYodd))
        {
            textureColor1 = lightGray;
        }
        else
        {
            textureColor1 = darkGray;
        }
    #endif
#endif
    
    // DRAW PHASE
#if defined(VERTEX_LIT)
    
#if defined(BLINN_PHONG)
    vec3 color = vec3(0.0);
    #if defined(VIEW_AMBIENT)
        color += materialLightAmbientColor;
    #endif

    #if defined(VIEW_DIFFUSE)
        color += varDiffuseColor * materialLightDiffuseColor;
    #endif

    #if defined(VIEW_ALBEDO)
        color *= textureColor0.rgb;
    #endif

    #if defined(VIEW_SPECULAR)
        color += (varSpecularColor * textureColor0.a) * materialLightSpecularColor;
    #endif
    
#elif defined(NORMALIZED_BLINN_PHONG)
   
    vec3 color = vec3(0.0);
    #if defined(VIEW_AMBIENT) && !defined(MATERIAL_LIGHTMAP)
        color += materialLightAmbientColor;
    #endif
        
    #if defined(VIEW_DIFFUSE)
        #if defined(MATERIAL_LIGHTMAP)
            color = textureColor1.rgb * 2.0;
        #else
            color += varDiffuseColor;
        #endif
    #endif
        
    #if defined(VIEW_ALBEDO)
        color *= textureColor0.rgb;
    #endif
    
    #if defined(VIEW_SPECULAR)
        float glossiness = pow(5000.0, inGlossiness * textureColor0.a); //textureColor0.a;
        //float glossiness = pow(5000.0, 0.5 * textureColor0.a); //textureColor0.a;
        float specularNorm = (glossiness + 2.0) / 8.0;
    
        #if defined(MATERIAL_LIGHTMAP)
            color += varSpecularColor * pow(varNdotH, glossiness) * specularNorm * textureColor1.rgb / 2.0;
        #else
            color += varSpecularColor * pow(varNdotH, glossiness) * specularNorm;
        #endif
    #endif
    
#endif

    
#elif defined(PIXEL_LIT)
    // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    normal = normalize (normal);
    //normal = vec3(0.0, 0.0, 1.0);
    
    float attenuation = lightIntensity0;
    #if defined(DISTANCE_ATTENUATION)
        attenuation /= (varPerPixelAttenuation * varPerPixelAttenuation);
    #endif
    
#if !defined(FAST_NORMALIZATION)
    vec3 toLightNormalized = normalize(varToLightVec);
    vec3 toCameraNormalized = normalize(varToCameraVec);
    vec3 H = toCameraNormalized + toLightNormalized;
    H = normalize(H);

    // compute diffuse lighting
    float NdotL = max (dot (normal, toLightNormalized), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightNormalized, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);
#else
    // Kwasi normalization :-)
    // compute diffuse lighting
    vec3 normalizedHalf =normalize(varHalfVec);
    
    float NdotL = max (dot (normal, varToLightVec), 0.0);
    float NdotH = max (dot (normal, normalizedHalf), 0.0);
    float LdotH = max (dot (varToLightVec, normalizedHalf), 0.0);
    float NdotV = max (dot (normal, varToCameraVec), 0.0);
#endif
    
#if defined(NORMALIZED_BLINN_PHONG)
    vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    float specularity = inSpecularity;
    float glossiness = inGlossiness * textureColor0.a;
    float glossPower = pow(5000.0, glossiness); //textureColor0.a;
    
   	//float glossiness = inGlossiness * 0.999;
	//glossiness = 200.0 * glossiness / (1.0 - glossiness);

	float diffuse = NdotL / _PI;// * (1.0 - fresnelIn * specularity);
    
    //float Dbp = (glossiness + 2.0) / (8.0) * pow(NdotH, glossiness) * NdotL;
    // specular cutoff
	float spec_cutoff = 1.0 - NdotL;
	spec_cutoff *= spec_cutoff;
	spec_cutoff *= spec_cutoff;
	spec_cutoff *= spec_cutoff;
	spec_cutoff = 1.0 - spec_cutoff;
    
    //float specularNorm = (glossiness + 2.0) * (glossiness + 4.0) / (8.0 * _PI * (pow(2.0, -glossiness / 2.0) + glossiness));
    float specularNorm = (glossPower + 2.0) / 8.0;
    float Dbp = specularNorm * pow(NdotH, glossPower) * NdotL;
    float Geo = 1.0 / LdotH * LdotH;

    vec3 specular = Dbp * Geo * fresnelOut * specularity;

//    float GeoNormalization = NdotL * NdotV;
//    float GeoFactor = GeoNormalization;
//    float Geo = 1.0 / LdotH * LdotH;
    
#else
    vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    float specularity = inSpecularity;
    float glossiness = inGlossiness;
    
	// calculate diffuse light
	vec3 diffuse = NdotL / _PI * (1.0 - fresnelIn * specularity);
    
	// specular cutoff
	float spec_cutoff = 1.0 - NdotL;
	spec_cutoff *= spec_cutoff;
	spec_cutoff *= spec_cutoff;
	spec_cutoff *= spec_cutoff;
	spec_cutoff = 1.0 - spec_cutoff;
    
	// calculate specular power, specular normalization coefficient
	glossiness *= 0.999;
	float specular_power = 200.0 * glossiness / (1.0 - glossiness);
	float specular_normalization = 	(specular_power + 2.0) * (specular_power + 4.0) /
    (8.0 * _PI * (pow(2.0, -specular_power / 2.0) + specular_power));
    
	// calculate specular light
	vec3 specular = pow(NdotH, specular_power) * fresnelOut * specular_normalization * specularity * spec_cutoff;
#endif
    
    vec3 color = vec3(0.0);
    
    #if defined(VIEW_AMBIENT) && !defined(MATERIAL_LIGHTMAP)
        color += materialLightAmbientColor;
    #endif
    
    #if defined(VIEW_DIFFUSE)
        #if defined(MATERIAL_LIGHTMAP)
            color = textureColor1.rgb * 2.0;
        #else
            color += diffuse;
        #endif
    #endif
    
    #if defined(VIEW_ALBEDO)
        color *= textureColor0.rgb;
    #endif
    
    #if defined(VIEW_SPECULAR)
        color += specular * textureColor1.rgb;
    #endif

#elif defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
    vec3 color = textureColor1.rgb;
#elif defined(MATERIAL_VIEW_TEXTURE_ONLY)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_DETAIL)
    vec3 color = textureColor0.rgb * textureColor1.rgb * 2.0;
#elif defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_DETAIL)

    //ATTENTION:
    //BE CAREFUL TO MODIFY BOTH PARTS OF THIS CONDITION
    //THEY SHOULD BE IDENTICAL IN MATH!
    #if defined(VIEW_DIFFUSE) || defined(VIEW_ALBEDO)
    
        #if defined(VIEW_ALBEDO)
    
            #if defined(VIEW_DIFFUSE)
                vec3 color = textureColor0.rgb * textureColor1.rgb * 2.0;
            #else
                vec3 color = textureColor0.rgb;
            #endif
    
        #else
            vec3 color = textureColor1.rgb;
        #endif
    
    #else
        vec3 color = textureColor0.rgb * textureColor1.rgb * 2.0;
    #endif

#elif defined(MATERIAL_TEXTURE)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_SKYBOX)
    vec4 color = textureColor0;
#else
    vec3 color = vec3(1.0);
#endif
    
#if defined(ALPHABLEND) && defined(MATERIAL_TEXTURE)
    gl_FragColor = vec4(color, textureColor0.a);
#elif defined(MATERIAL_SKYBOX)
    gl_FragColor = color;
#else
    gl_FragColor = vec4(color, 1.0);
#endif
    
#if defined(SPEED_TREE_LEAF)
    gl_FragColor *= vec4(varVertexColor.rgb * treeLeafColorMul * treeLeafOcclusionMul + vec3(treeLeafOcclusionOffset), varVertexColor.a);
#elif defined(VERTEX_COLOR)
    gl_FragColor *= varVertexColor;
#endif
    
#if defined(FLATCOLOR)
    gl_FragColor *= flatColor;
#endif
    
    
#if defined(REFLECTION)
#if defined(VERTEX_LIT)
    lowp vec4 reflectionColor = textureCube(cubemap, reflectionDirectionInWorldSpace); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor = reflectionColor * 0.9;
#elif defined(PIXEL_LIT)
    vec3 fresnelRefl = FresnelShlickVec3(NdotV, metalFresnelReflectance);

    mediump vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpace, normal);
    mediump vec3 reflectionVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * reflectionVectorInTangentSpace);
    lowp vec4 reflectionColor = textureCubeLod(cubemap, reflectionVectorInWorldSpace, (1.0 - inGlossiness) * 7.0); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor.rgb += fresnelRefl * reflectionColor.rgb * specularity;//* textureColor0.rgb;
#endif
#endif
    
    
#if defined(VERTEX_FOG)
    gl_FragColor.rgb = mix(fogColor, gl_FragColor.rgb, varFogFactor);
#endif
}