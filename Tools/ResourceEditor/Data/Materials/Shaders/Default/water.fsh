<CONFIG>
normalmap = 0
cubemap = 1
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


uniform samplerCube cubemap;
uniform lowp vec3 reflectionTintColor;

#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)

uniform sampler2D normalmap; // [1]:ONCE

uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
varying vec3 varLightVec;

varying mediump vec2 varTexCoord0;
varying mediump vec2 varTexCoord1;

uniform lowp vec3 refractionTintColor;

uniform float fresnelBias;
uniform float fresnelPow;
uniform float eta;

uniform float materialSpecularShininess;
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color

#if defined (REAL_REFLECTION)
uniform sampler2D dynamicReflection;
uniform sampler2D dynamicRefraction;

#if defined (SCREEN_SPACE_WATER)
varying mediump float eyeDist;

uniform mediump vec2 rcpScreenSize;
uniform mediump vec2 screenOffset;

uniform float reflectionDistortion;
uniform float refractionDistortion;
#else
uniform float aproxDepth;
uniform float aproxReflectionScale;
#endif

#endif

#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif


float FresnelShlick(float NdotL, float fresnelBias, float fresnelPow)
{	  
    return fresnelBias + (1.0 - fresnelBias) * pow(1.0 - NdotL, fresnelPow);
}


void main()
{

#if defined(VERTEX_LIT)
    lowp vec3 reflectionColor = textureCube(cubemap, reflectionDirectionInWorldSpace).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor = vec4(reflectionColor * reflectionTintColor, 1.0);
	
#elif defined(PIXEL_LIT)
  
    //compute normal
    vec3 normal1 = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    vec3 normal2 = 2.0 * texture2D (normalmap, varTexCoord1).rgb - 1.0;
    vec3 normal = normalize (normal1 + normal2);

#if defined (DEBUG_UNITY_Z_NORMAL)    
	normal = vec3(0.0,0.0,1.0);
#endif
	
    //compute shininess and fresnel
    lowp vec3 cameraToPointInTangentSpaceNorm = normalize(cameraToPointInTangentSpace);    
    float lambertFactor = max (dot (-cameraToPointInTangentSpaceNorm, normal), 0.0);    
    lowp vec3 halfVec = normalize(normalize(varLightVec)-cameraToPointInTangentSpaceNorm);
    float shininess = pow (max (dot (halfVec, normal), 0.0), materialSpecularShininess);
    float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);	

    //compute reflection
#if defined (REAL_REFLECTION)
    #if defined(SCREEN_SPACE_WATER)
        mediump vec2 waveOffset = normal.xy/max(1.0, eyeDist);
        lowp vec2 screenPos = (gl_FragCoord.xy-screenOffset)*rcpScreenSize;
        lowp vec3 reflectionColor = texture2D(dynamicReflection, screenPos+waveOffset*reflectionDistortion).rgb;
        screenPos.y=1.0-screenPos.y;
        lowp vec3 refractionColor = texture2D(dynamicRefraction, screenPos+waveOffset*refractionDistortion).rgb;
    #else	
        /**/
        //reflected vector is mixed with camera vector 
        //we need to swap z as reflected texture is already in reflected camera space
        mediump vec3 reflectionVectorInTangentSpace = mix(-cameraToPointInTangentSpace, reflect(cameraToPointInTangentSpaceNorm, normal)*vec3(1.0,1.0,-1.0), aproxReflectionScale); 		
        mediump vec4 reflectionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * reflectionVectorInTangentSpace), 0.0));	
        lowp vec3 reflectionColor = texture2D(dynamicReflection, reflectionVectorInNDCSpace.xy/reflectionVectorInNDCSpace.w*0.5 + vec2(0.5, 0.5)).rgb;
		
        mediump vec3 refractionVectorInTangentSpace = refract(cameraToPointInTangentSpaceNorm, normal, eta)*aproxDepth-cameraToPointInTangentSpace;
        mediump vec4 refractionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * refractionVectorInTangentSpace), 0.0));	
        lowp vec3 refractionColor = texture2D(dynamicRefraction, refractionVectorInNDCSpace.xy/refractionVectorInNDCSpace.w*vec2(0.5, -0.5)+vec2(0.5, 0.5)).rgb;
    #endif	    
    //gl_FragColor = vec4(materialLightSpecularColor, 1.0);		
    gl_FragColor = vec4(mix(refractionColor*refractionTintColor, reflectionColor*reflectionTintColor, fresnel) + shininess * materialLightSpecularColor, 1.0);		
#else
    mediump vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpaceNorm, normal);
    mediump vec3 reflectionVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * reflectionVectorInTangentSpace);    
    lowp vec3 reflectionColor = textureCube(cubemap, reflectionVectorInWorldSpace).rgb;
    
    #if defined (FRESNEL_TO_ALPHA)	
        gl_FragColor = vec4(reflectionTintColor * reflectionColor + vec3(shininess * materialLightSpecularColor), fresnel);
    #else	
        mediump vec3 refractedVectorInTangentSpace = refract(cameraToPointInTangentSpace, normal, eta);
        mediump vec3 refractedVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * refractedVectorInTangentSpace);
        lowp vec3 refractionColor = textureCube(cubemap, refractedVectorInWorldSpace).rgb; 
           
        gl_FragColor = vec4(mix(refractionColor*refractionTintColor, reflectionColor*reflectionTintColor, fresnel) + shininess * materialLightSpecularColor, 1.0);	    

    #endif
#endif
#endif
    
#if defined(VERTEX_FOG)
    gl_FragColor.rgb = mix(fogColor, gl_FragColor.rgb, varFogFactor);
#endif
}