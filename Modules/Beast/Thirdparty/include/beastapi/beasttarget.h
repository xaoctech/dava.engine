/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The target definitions
 */ 
#ifndef BEASTTARGET_H
#define BEASTTARGET_H

#include "beastapitypes.h"
#include "beasttargetentity.h"
#include "beastjob.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Different types of Atlas packing strategies
 */
typedef enum {
    /**
	 * Texel
	 */
    ILB_AP_TEXEL = 0,
    /**
	 * Block
	 */
    ILB_AP_BLOCK
} ILBAtlasPacking;

/**
 * World space filter components
 */
typedef enum {
    /**
	 * Filter everything
	 */
    ILB_WSFC_DEFAULT = 0,
    /**
	 * Filter only direct component
	 */
    ILB_WSFC_DIRECT,
    /**
	 * Filter only indirect component
	 */
    ILB_WSFC_INDIRECT
} ILBWorldSpaceFilterComponent;

/**
 * Camera modes in Live Ernst Session
 */
typedef enum {
    /**
	 * Return frame buffers containing a full shaded light mapped version of the camera view
	 * I.e. Lighting will be applied according to the target texture/vertex light map settings
	 */
    ILB_LEC_CAMERA_LIGHT_MAP_LIGHTING,

    /**
	 * Return frame buffers containing a full shaded per pixel version of the camera view
	 * I.e Lighting will be calculated per pixel, disregarding the texture/vertex light map settings
	 */
    ILB_LEC_CAMERA_PER_PIXEL_LIGHTING
} ILBCameraRenderModeType;

/** 
 * Adds a texture baking target to a job
 * @param job the job to add the target to
 * @param name the name of the target
 * @param width the width in pixels of the texture target
 * @param height the height in pixels of the texture target
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateTextureTarget(ILBJobHandle job,
                                                  ILBConstString name,
                                                  int32 width,
                                                  int32 height,
                                                  ILBTargetHandle* target);

/** 
 * Adds an atlased texture baking target to a job
 * @param job the job to add the target to
 * @param name the name of the target
 * @param maxWidth the maximum width in pixels of each generated texture
 * @param maxHeight the maximum height in pixels of each generated texture
 * @param maxTextures the maximum number of generated textures. 0 means don't care.
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateAtlasedTextureTarget(ILBJobHandle job,
                                                         ILBConstString name,
                                                         int32 maxWidth,
                                                         int32 maxHeight,
                                                         int32 maxTextures,
                                                         ILBTargetHandle* target);

/** 
 * Sets the alignment on an atlased texture target
 * @param target the target
 * @param alignment the alignment
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetAtlasAlignment(ILBTargetHandle target,
                                                int32 alignment);

/** 
 * Sets the padding on an atlased texture target
 * @param target the target
 * @param padding the padding
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetAtlasPadding(ILBTargetHandle target,
                                              int32 padding);

/** 
 * Sets the packing strategy on an atlased texture target
 * @param target the target
 * @param packing the packing strategy, default is Block packing
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetAtlasPacking(ILBTargetHandle target,
                                              ILBAtlasPacking packing);

/**
 * Changes the resolution of a texture target when running an Live Ernst 
 * job.
 * @param target the target to change resolution on
 * @param width the new width
 * @param height the new height
 */
ILB_DLL_FUNCTION ILBStatus ILBSetTargetResolution(ILBTargetHandle target,
                                                  int32 width, int32 height);

/** 
 * Enables packing spatially close objects into the same texture
 * @param target the target
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBEnableAtlasSpatial(ILBTargetHandle target);

/**
 * Enables automatic rescaling of instances in the atlas based on frequency
 * information from the baked lightmaps.
 * @param target the atlas target
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBEnableAtlasRescale(ILBTargetHandle target);

/**
 * Sets the frequency cutoff for the atlas rescale mode. Lightmaps with frequency
 * content lower than this value will be downscaled relative this value. Lightmaps
 * with higher frequency content will not be scaled.
 * @param target the atlas target
 * @param threshold the cutoff frequency (default is 1.0f)
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetAtlasRescaleFrequencyThreshold(ILBTargetHandle target, float threshold);

/** 
 * Adds a vertex baking target
 * @param job the job to add the target to
 * @param name the name of the target
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateVertexTarget(ILBJobHandle job,
                                                 ILBConstString name,
                                                 ILBTargetHandle* target);

/** 
 * Adds a camera render target to a job
 * @param job the job to add the target to
 * @param name the name of the target
 * @param camera handle to the camera to render from
 * @param width the width in pixels of the image
 * @param height the height in pixels of the image
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateCameraTarget(ILBJobHandle job,
                                                 ILBConstString name,
                                                 ILBCameraHandle camera,
                                                 int32 width,
                                                 int32 height,
                                                 ILBTargetHandle* target);

/**
 * Dictates which camera render mode used. For classic final jobs type should always be
 * ILB_LEC_CAMERA_PER_PIXEL_LIGHTING. For live ernst jobs ILB_LEC_CAMERA_PER_PIXEL_LIGHTING
 * means final frame rendering per pixel while ILB_LEC_CAMERA_LIGHT_MAPPING_LIGHTING means 
 * light mapped lighting. This requires that all objects to be baked have been setup as well.
 * @param target the target whose camera render mode will be set.
 * @param renderMode the camera type used in live ernst (per pixel or light mapped).
 */
ILB_DLL_FUNCTION ILBStatus ILBCameraTargetSetRenderMode(ILBTargetHandle target,
                                                        ILBCameraRenderModeType renderMode);

/** 
 * Adds a point cloud target to a job
 * @param job the job to add the target to
 * @param name the name of the target
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreatePointCloudTarget(ILBJobHandle job,
                                                     ILBConstString name,
                                                     ILBTargetHandle* target);

/**
 * Adds an UV unwrap target to a job
 * @param job the job to add the target to. Must be a UV job created with ILBCreateUVJob.
 * @param name the name of the target
 * @param texelsPerUnit the number of texels per unit to be used with the resulting UV layout. This affects spacing between UV charts.
 * @param target the handle to store the generated target in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateUVUnwrapTarget(ILBJobHandle job,
                                                   ILBConstString name,
                                                   float texelsPerUnit,
                                                   ILBTargetHandle* target);

/**
 * Sets the threshold used when segmenting the mesh into charts for unwrapping.
 * Controls the number of charts created by setting the angular threshold (allowed normal deviation) 
 * for triangles in a chart. A higher threshold results in larger normal deviations and creates larger, 
 * but more curved charts; a lower threshold creates smaller, flat charts.
 * @param target the UV unwrap target.
 * @param threshold the segmentation threshold, range 0.0 - 1.0, default is 0.3.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetUVUnwrapSegmentationThreshold(ILBTargetHandle target, float threshold);

/**
 * Sets a weight for the importance of having flat charts.
 * It affects the importance of flatness relative to straightness and compactness.
 * If set to 0.0, flatness is ignored (this will in most cases give a bad UV layout).
 * @param target the UV unwrap target.
 * @param weight the weight for flatness, range 0.0 - 1.0, default is 1.0.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetUVUnwrapFlatness(ILBTargetHandle target, float weight);

/**
 * Sets a weight for the importance of having straight chart boundaries.
 * It affects the importance of straightness relative to flatness and compactness.
 * If set to 0.0, straightness is ignored.
 * @param target the UV unwrap target.
 * @param weight the weight for straightness, range 0.0 - 1.0, default is 0.5.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetUVUnwrapStraightness(ILBTargetHandle target, float weight);

/**
 * Sets a weight for the importance of having compact (round) charts.
 * It affects the importance of compactness relative to flatness and straightness.
 * Compactness is expensive to compute and will increase the computation time if enabled.
 * If set to 0.0, compactness is ignored.
 * @param target the UV unwrap target.
 * @param weight the weight for compactness, range 0.0 - 1.0, default is 0.0.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetUVUnwrapCompactness(ILBTargetHandle target, float weight);

/** 
 * Adds an instance to bake to a texture or vertex bake target
 * @param target the target to add the instance to
 * @param bakeInstance the instance to bake
 * @param targetEntity the targetEntity for this instance. Can be 0 if you don't care
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddBakeInstance(ILBTargetHandle target,
                                              ILBInstanceHandle bakeInstance,
                                              ILBTargetEntityHandle* targetEntity);

/** 
 * Adds a point cloud to bake
 * @param target the target to add the point cloud to (only works for point cloud targets)
 * @param pointCloud the point cloud to bake
 * @param targetEntity the targetEntity for this instance. Can be 0 if you don't care
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddBakePointCloud(ILBTargetHandle target,
                                                ILBPointCloudHandle pointCloud,
                                                ILBTargetEntityHandle* targetEntity);

/** 
* Adds a mesh to be unwrapped to a UV unwrap target
* @param target the target to add the mesh to (only works UV unwrap targets)
* @param mesh the mesh to be unwrapped
* @param targetEntity the target entity for this mesh. Needed to get the resulting uv layer back.
* @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddMeshToTarget(ILBTargetHandle target,
                                              ILBMeshHandle mesh,
                                              ILBTargetEntityHandle* targetEntity);

/** 
 * Gets the number of framebuffers associated with this target
 * Is only valid on targets rendering images and if the
 * target is done
 * @param target the target to get the count for
 * @param count a pointer to the variable to receive the framebuffer count
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetFramebufferCount(ILBTargetHandle target,
                                                  int32* count);

/** 
 * Gets a framebuffer from a target
 * Is only valid on targets rendering images and the
 * target is done
 * @param target the target to get framebuffer from
 * @param pass the pass to get vertex data for
 * @param index of the framebuffer to get
 * @param framebuffer pointer to the handle that should receive the framebuffer
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetFramebuffer(ILBTargetHandle target,
                                             ILBRenderPassHandle pass,
                                             int32 index,
                                             ILBFramebufferHandle* framebuffer);

/** 
 * Gets a framebuffer with vertex data from a target
 * Is only valid on targets rendering vertex data and the
 * target is done
 * @param target the target to get vertex buffer from
 * @param pass the pass to get vertex data for
 * @param targetEntity the target entity to get vertex data for
 * @param framebuffer pointer to the handle that should receive the framebuffer
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetVertexbuffer(ILBTargetHandle target,
                                              ILBRenderPassHandle pass,
                                              ILBTargetEntityHandle targetEntity,
                                              ILBFramebufferHandle* framebuffer);

/** 
* Gets a uv layer from a target
* Only valid on UV unwrap targets and when the target is done
* @param target the target to get a UV layer from
* @param targetEntity the target entity to get a UV layer for
* @param uvLayer pointer to the handle that holds the new UV layer
* @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUVLayerFromTarget(ILBTargetHandle target,
                                                   ILBTargetEntityHandle targetEntity,
                                                   ILBUVLayerHandle* uvLayer);

/** 
* Gets the resulting resolution from packing the UV layout of the unwrapped mesh
* Only valid on UV unwrap targets and when the target is done
* @param target the target to get resolution from
* @param targetEntity the target entity to get resolution from
* @param width the resolution width
* @param height the resolution height
* @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUVLayerResolutionFromTarget(ILBTargetHandle target,
                                                             ILBTargetEntityHandle targetEntity,
                                                             int32* width,
                                                             int32* height);

/** 
* Add a Render Pass to a target
* @param target the target to add the pass to
* @param pass the pass to add to the target
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBAddPassToTarget(ILBTargetHandle target,
                                              ILBRenderPassHandle pass);

/** 
* Enable world space filter for a target
* @param target the target to enable the filter on
* @param component the component to apply the filter to
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBEnableWorldSpaceFilter(ILBTargetHandle target, ILBWorldSpaceFilterComponent component);

/**
* Set the world space filter size for a target
*
* The filter size is relative to the mean sample distance of the target
* The default filter size is 1
* Start with a default filter size and tweak it to get the desired smoothness
*
* @param target the target to set the size for
* @param component the component to set the size for
* @param size the filter size
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBSetWorldSpaceFilterSize(ILBTargetHandle target, ILBWorldSpaceFilterComponent component, float size);

/**
* Set the maximum allowed normal deviation for world space filtering
*
* The filtered sample will exclude samples with a normal deviating more than the specified limit
* The normal deviation is specified in degrees
* The default limit is 25 degrees
*
* @param target the target to set the normal deviation for
* @param component the component to set the normal deviation for
* @param normalDeviation the maximum allowed normal deviation
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBSetWorldSpaceFilterNormalDeviation(ILBTargetHandle target, ILBWorldSpaceFilterComponent component, float normalDeviation);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif // BEASTTARGET_H
