/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* Render Pass specification
*/ 

#ifndef BEASTRENDERPASS_H
#define BEASTRENDERPASS_H

#include "beastapitypes.h"

/**
* Illumination Modes
*/
typedef enum {
    /**
	* Only direct illumination (no indirect illumination)
	*/
    ILB_IM_DIRECT_ONLY = 0,

    /**
	* Only indirect illumination (no direct illumination)
	*/
    ILB_IM_INDIRECT_ONLY,

    /**
	* Both direct and indirect illumination
	*/
    ILB_IM_FULL,

    /**
	* Stores both direct+indirect and indirect separately
	*/
    ILB_IM_FULL_AND_INDIRECT,

} ILBIlluminationMode;

/**
* Basis type
*/
typedef enum {
    ILB_BT_NO_BASIS = 0,
    ILB_BT_SH,
} ILBBasisType;

/**
* RNM Basis
*/
typedef enum {
    /**
	* Half-Life 2 compatible basis
	*/
    ILB_RB_HL2 = 0,

    /**
	* Unreal Engine 3 compatible basis
	*/
    ILB_RB_UE3,

    /**
	* Unreal Engine 3 basis in untouched order
	*/
    ILB_RB_UE3_FLIPPED,

    /**
	* Allows the user to enter the basis vectors manually
	*/
    ILB_RB_CUSTOM,
} ILBRNMBasis;

/**
* Allow Negative
*/
typedef enum {
    /**
	* Allows negative RNM values.
	*/
    ILB_AN_ALLOW = 0,

    /**
	* Clamps negative RNM values to 0.
	*/
    ILB_AN_DISALLOW,

    /**
	* As #ILB_AN_DISALLOW, and also culls lights below the horizon of each triangle.
	*/
    ILB_AN_DISALLOW_CULL_HORIZON,
} ILBRNMAllowNegative;

/**
* Self Occlusion Mode
*/
typedef enum {
    /**
	* Self Occluded rays will continue beyond the originating object
	*/
    ILB_SO_DISABLED = 0,

    /**
	* Self Occluded rays will be set to the environment
	*/
    ILB_SO_SET_ENVIRONMENT,

    /**
	* Objects can self occlude
	*/
    ILB_SO_ENABLED,
} ILBAOSelfOcclusion;

/**
* Light Pass Type
*/
typedef enum {
    /**
	* Stores the incoming light in the light map.
    */
    ILB_LP_LIGHTMAP = 0,

    /**
	* Stores the shadow mask. The individual light mask intensity will be 
	* proportional to the light source intensity.
    */
    ILB_LP_SHADOWMAP,

    /**
	* Stores the full shading in the light map.
    */
    ILB_LP_FULLSHADING,
} ILBLightPassType;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** 
	* Creates a Full Shading render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateFullShadingPass(ILBJobHandle job,
                                                    ILBConstString name,
                                                    ILBRenderPassHandle* pass);

/** 
	* Creates an RNM render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param mode Selects Direct Illumination Only, Indirect Illumination 
	* only or both.
	* @param samples Number of samples for non-adaptive RNM. Set to 0 samples to turn 
	* on adaptivity (recommended).
	* @param basis The RNM basis to use
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateRNMPass(ILBJobHandle job,
                                            ILBConstString name,
                                            ILBIlluminationMode mode,
                                            int32 samples,
                                            ILBRNMBasis basis,
                                            ILBRenderPassHandle* pass);

/** 
	* Creates a Light render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param type the lighting mode
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateLightPass(ILBJobHandle job,
                                              ILBConstString name,
                                              ILBLightPassType type,
                                              ILBRenderPassHandle* pass);

/**
	* No support in physical rendering.
	* Makes a light pass use signed distance field shadow maps. Each resulting
	* baked pixel will store the distance to the closest shadow transition. \n
	* Distances are remapped to the range [0.0, 1.0], where:
	* -	Values below 0.5 indicate pixels that lie in unlit areas. As the distance from the pixel
	* 	to the shadow transition increases, the value decreases. The minimum value of 0.0f indicates
	* 	that the pixel is at a distance of at least \c maxWorldDistance from the transition.
	* -	0.5 indicates a pixel that lies exactly on a shadow transition.
	* -	Values above 0.5 indicate pixels that lie in lit areas. As the distance from the pixel
	*	to the shadow transition increases, the value increases. The maximum value of 1.0f indicates
	* 	that the pixel is at a distance of at least \c maxWorldDistance from the transition. \n
	* We strongly recommend that you disable soft shadows on any lights that generate
	* signed distance fields, as sharp shadow transitions generate the best results. Also make
	* sure to specify a high maximum super sampling rate to ensure high quality signed distance
	* fields. \n
	* Only enable signed distance fields for a light pass, and only when the light pass is configured
	* to bake shadow maps (#ILB_LP_SHADOWMAP). Any other use will produce undefined results.
	* @param pass the light pass
	* @param pixelFilterSize Sets the maximum search range in pixels in the image.
	* This is mainly a performance optimization hint, having this set too large will cause
	* excessive overlap rendering which will degrade performance.
	* Default value is 20.
	* @param maxWorldDistance the maximum world distance to be stored. Default 
	* value is 1.0f.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBEnableSignedDistanceField(ILBRenderPassHandle pass,
                                                        int32 pixelFilterSize, float maxWorldDistance);

/** 
	* Sets the lambertian scale on a light pass. \n
	* This configures the amount of lambertian reflectance (N * L) to be weighted into the 
	* resulting direct lighting. A scale of 1.0f will render the light that is reflected by the
	* surface, just like the ordinary illumination pass. A scale of 0.0f renders the incoming 
	* light on the surface, disregarding the orientation from the surface to the light source.
	* This can be useful if you want to do surface shading at a later stage. Please note that
	* this makes sense to bake only for single light sources and only for direct lighting.
	* @param pass the pass
	* @param scale the scale of influcence from lambertian reflectance (N*L)
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetLambertianScale(ILBRenderPassHandle pass,
                                                 float scale);

/** 
	* Creates a Light Pass Entry
	* @param pass the light pass to create the entry on
	* @param entry the created entry
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateLightPassEntry(ILBRenderPassHandle pass,
                                                   ILBLightPassEntryHandle* entry);

/** 
	* Add a light to a light pass
	* @param entry the light pass entry to add the light to
	* @param light the light to add
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBAddLightToPass(ILBLightPassEntryHandle entry,
                                             ILBLightHandle light);

/** 
	* Add an affected target entity to a light pass
	* @param entry the light pass entry to add the target entity to
	* @param target the target to add
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBAddTargetToPass(ILBLightPassEntryHandle entry,
                                              ILBTargetEntityHandle target);

/** 
	* Add a light to be fully baked to a FullAndIndirectIllumination Pass.
	* @param pass the illumination pass to add the light to
	* @param light the light to add
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBAddFullyBakedLight(ILBRenderPassHandle pass,
                                                 ILBLightHandle light);

/** 
	* Creates a Normal render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateNormalPass(ILBJobHandle job,
                                               ILBConstString name,
                                               ILBRenderPassHandle* pass);

/** 
	* Creates an Ambient Occlusion render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param maxDistance the maximum distance to check for occlusion. 0 for infinite.
	* @param coneAngle the cone angle. Default is 180
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateAmbientOcclusionPass(ILBJobHandle job,
                                                         ILBConstString name,
                                                         float maxDistance,
                                                         float coneAngle,
                                                         ILBRenderPassHandle* pass);

/** 
	* No support in physical rendering.
	* Enables adaptivity on an AO pass
	* @param pass the pass to enable adaptivity on, must be an AO pass
	* @param accuracy adaptive accuracy, default is 1
	* @param smooth smooth value, default is 1
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAOAdaptive(ILBRenderPassHandle pass,
                                            float accuracy,
                                            float smooth);

/** 
	* No support in physical rendering.
	* Sets the number of rays to use in an AO pass
	* @param pass the affected pass, must be an AO pass
	* @param minRay the minimum number of rays to sample for each point, default is 64
	* @param maxRay the maximum number of rays to sample for each point, default is 300
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAONumRays(ILBRenderPassHandle pass,
                                           int32 minRay,
                                           int32 maxRay);

/** 
	* Sets the contrast and scale of an AO pass
	* @param pass the affected pass, must be an AO pass
	* @param contrast the desired contrast of the AO pass. Default = 1.0f
	* @param scale scale of occlusion values. Default = 1.0f
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAOContrast(ILBRenderPassHandle pass,
                                            float contrast,
                                            float scale);

/** 
	* Enables Uniform Sampling on an AO pass. When Uniform Sampling is enabled
	* the sampling is not cos()-weighted.
	* @param pass the affected pass, must be an AO pass
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAOUniformSampling(ILBRenderPassHandle pass);

/** 
	* Sets how the AO pass should react to self occlusion.
	* @param pass the affected pass, must be an AO pass
	* @param selfOcclusion the self occlusion mode. Default is ILB_SO_ENABLED.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAOSelfOcclusion(ILBRenderPassHandle pass,
                                                 ILBAOSelfOcclusion selfOcclusion);

/** 
	* No support in physical rendering.
	* Calculates the "bent normal" (most visible direction).
	* If this is used, sampling cannot be adaptive.
	* The put will contain normals in RGB and occlusion in A
	* @param pass the affected pass, must be an AO pass
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBEnableAOBentNormals(ILBRenderPassHandle pass);

/** 
	* Creates an Illumination render pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param mode Selects Direct Illumination Only, Indirect Illumination 
	* only or both.
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateIlluminationPass(ILBJobHandle job,
                                                     ILBConstString name,
                                                     ILBIlluminationMode mode,
                                                     ILBRenderPassHandle* pass);

/** 
	* Creates an Illumination render pass with sh output
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param mode Selects Direct Illumination Only, Indirect Illumination 
	* only or both.
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateIlluminationPassSH(ILBJobHandle job,
                                                       ILBConstString name,
                                                       ILBIlluminationMode mode,
                                                       ILBRenderPassHandle* pass);

/** 
	* Creates a LUA pass
	* @param job the job to add the pass to
	* @param name the name of the pass
	* @param scriptFile the file name of the script
	* @param pass the handle to store the generated target in
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBCreateLuaPass(ILBJobHandle job,
                                            ILBConstString name,
                                            ILBConstString scriptFile,
                                            ILBRenderPassHandle* pass);

/** 
	* Enable lambertian clamp on an RNM pass.
	* @param pass the pass
	* @param val the lambertian clamp value
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetLambertianClamp(ILBRenderPassHandle pass,
                                                 float val);

/** 
	* Determines how the RNM pass handles light contributions that come from below the plane of
	* the surface. In an ordinary illumination pass, such a light would yield a light contribution
	* of 0. However, in the RNM pass the light can still affect one or two of the RNM basis
	* components. This function determines whether negative RNM components will be taken into account
	* as-is or clamped to 0.
	* @param pass the pass
	* @param allow the allow value (default is ILB_AN_DISALLOW_CULL_HORIZON)
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetAllowNegative(ILBRenderPassHandle pass,
                                               ILBRNMAllowNegative allow);

/**
	* Enables inclusion of a normal component in the RNM pass.
	* @param pass the pass
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBIncludeNormalComponent(ILBRenderPassHandle pass);

/**
	* Scales the RNM values to the amplitude of the normal component.
	* @param pass the pass
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBRNMMatchNormalIntensity(ILBRenderPassHandle pass);

/**
	* Normalizes the texture values to the 0..1 range. Stores the original range per entity
	* which can be collected with the getNormalization* functions.
	* @param pass the pass
	* @param perChannel if enabled normalization will be done individually for each channel
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBNormalizeTextures(ILBRenderPassHandle pass,
                                                bool perChannel);

/**
	* Scales the resolution on all texture targets for the pass.
	* @param pass the pass
	* @param scale the resolution scale factor.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetResolutionScale(ILBRenderPassHandle pass,
                                                 float scale);

/**
	* Scales the nondirectional influence of materials. Only supported by the RNM pass.
	* @param pass the pass
	* @param scale the nondirectional scale factor.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetNondirectionalScale(ILBRenderPassHandle pass,
                                                     float scale);

/**
	* Adds a custom basis vector to be used with ILB_RB_CUSTOM. Only supported by the RNM pass.
	* @param pass the pass
	* @param basisVector the basis vector to be added.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBAddCustomBasisVector(ILBRenderPassHandle pass,
                                                   const ILBVec3* basisVector);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //BEASTRENDERPASS_H
