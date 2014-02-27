<CONFIG>
albedo = 0
normalmap = 1
dynamicReflection = 2
dynamicRefraction = 3
<FRAGMENT_SHADER>

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D albedo;
varying mediump vec2 varTexCoord0;
varying mediump vec2 varTexCoord1;
varying mediump float eyeDist;
uniform samplerCube cubemap;
uniform lowp vec3 reflectionTintColor;
uniform lowp vec3 refractionTintColor;
uniform float fresnelBias;
uniform float fresnelPow;
uniform float eta;
uniform float cubmapOffset;

uniform float reflectionDistortion;
uniform float refractionDistortion;

uniform mediump vec2 rcpScreenSize;
uniform sampler2D dynamicReflection;
uniform sampler2D dynamicRefraction;

#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
#endif

#if defined(PIXEL_LIT)
uniform sampler2D normalmap; // [1]:ONCE
uniform float materialSpecularShininess;
uniform float lightIntensity0; 
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
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif


float FresnelShlick(float NdotL, float fresnelBias, float fresnelPow)
{
	float fresnel_minimum = fresnelBias;
	float fresnel_exponent = fresnelPow;
    
	return fresnel_minimum + (1.0 - fresnel_minimum) * pow(1.0 - NdotL, fresnel_exponent);
}

float Fresnel(float NdotL, float fresnelBias, float fresnelPow)
{
    float facing = (1.0 - NdotL);
    return max(fresnelBias + (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);
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

#endif

    // DRAW PHASE
#if defined(VERTEX_LIT)
    vec3 color = materialLightAmbientColor + varDiffuseColor * materialLightDiffuseColor
                                           + (varSpecularColor * textureColor0.a) * materialLightSpecularColor;
    color *= textureColor0.rgb;
#elif defined(PIXEL_LIT)
        // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal1 = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    vec3 normal2 = 2.0 * texture2D (normalmap, varTexCoord1).rgb - 1.0;
    vec3 normal = normalize (normal1 + normal2);
    //normal = vec3(0,0,1);

    float attenuation = lightIntensity0;
#if defined(DISTANCE_ATTENUATION)
    attenuation /= (varPerPixelAttenuation * varPerPixelAttenuation);
#endif
    
    vec3 cameraToPointInTangentSpaceNorm = normalize(cameraToPointInTangentSpace);
    // compute diffuse lighting
    float lambertFactor = max (dot (-cameraToPointInTangentSpaceNorm, normal), 0.0);
    // compute specular
    float shininess = pow (max (dot (normalize(varHalfVec), normal), 0.0), materialSpecularShininess);
    
#endif


#if defined(REFLECTION)
#if defined(VERTEX_LIT)
    lowp vec4 reflectionColor = textureCube(cubemap, reflectionDirectionInWorldSpace); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor = reflectionColor * 0.9;
#elif defined(PIXEL_LIT)
    /*mediump vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpaceNorm, normal);

    mediump vec3 reflectionVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * reflectionVectorInTangentSpace);
    reflectionVectorInWorldSpace.z += cubmapOffset;
    lowp vec3 reflectionColor = textureCube(cubemap, reflectionVectorInWorldSpace).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    
    mediump vec3 refractedVectorInTangentSpace = refract(cameraToPointInTangentSpace, normal, eta);
    mediump vec3 refractedVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * refractedVectorInTangentSpace);
    lowp vec3 refractionColor = textureCube(cubemap, refractedVectorInWorldSpace).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));*/
	
	lowp vec2 waveOffset = normal.xy/max(1.0, eyeDist);
	mediump vec2 screenPos = gl_FragCoord.xy*rcpScreenSize;
	lowp vec3 reflectionColor = texture2D(dynamicReflection, screenPos+waveOffset*reflectionDistortion).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));*/	
	screenPos.y=1-screenPos.y;
	lowp vec3 refractionColor = texture2D(dynamicRefraction, screenPos+waveOffset*refractionDistortion).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));*/

    
    float facing = 1.0 - lambertFactor;
    float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);
    
    //gl_FragColor = vec4(varHalfVec*0.5+vec3(0.5, 0.5, 0.5), 1);
    //gl_FragColor = vec4(waterColor * mix(refractionColor, reflectionColor, fresnel) + vec3(shininess), fresnel*0.6+0.4);
    //gl_FragColor = vec4(shininess*materialLightSpecularColor, 1.0);
    //gl_FragColor = vec4(waterColor * reflectionColor + vec3(shininess * materialLightSpecularColor), fresnel);
    //refractionColor + (1.0 - fresnel) + reflectionColor * fresnel;//vec4((waterColor * (lambertFactor + shininess)), 1.0) * (1.0 - fresnel) + fresnel * reflectionColor;
    //gl_FragColor = vec4(fresnel,fresnel,fresnel,1.0);
	//gl_FragColor = vec4(texture2D(refrTex, gl_FragCoord.xz/100.0).rgb, 1.0);
	//gl_FragColor = vec4(1.0, 0.2, 1.0, 1.0);
	gl_FragColor = vec4(mix(refractionColor*refractionTintColor, reflectionColor*reflectionTintColor, fresnel) + vec3(shininess), 1.0);	
	//gl_FragColor = vec4(reflectionColor, 1.0);
	//gl_FragColor = vec4(refractionColor, 1.0);
#endif
#endif
    
#if defined(VERTEX_FOG)
    gl_FragColor.rgb = mix(fogColor, gl_FragColor.rgb, varFogFactor);
#endif
}