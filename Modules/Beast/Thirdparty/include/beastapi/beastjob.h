/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* The beast job function definitions
*/ 
#ifndef BEASTJOB_H
#define BEASTJOB_H
#include "beastapitypes.h"

/**
* Status codes for Beast API calls
*/
typedef enum {
    /**
	* This was a triumph! I'm making a note here; Huge Success!
	*/
    ILB_JS_SUCCESS = 0,

    /**
	* Job was aborted by external means.
	*/
    ILB_JS_CANCELLED,

    /**
	* Beast does not have a valid license.
	*/
    ILB_JS_INVALID_LICENSE,

    /**
	* Error parsing the command line
	*/
    ILB_JS_CMDLINE_ERROR,

    /**
	* Error parsing the config files
	*/
    ILB_JS_CONFIG_ERROR,

    /**
	* Beast crashed, sorry.
	*/
    ILB_JS_CRASH,

    /**
	* Other Error
	*/
    ILB_JS_OTHER_ERROR = 0x10000001

} ILBJobStatus;

/**
* The different states for live job
*/
typedef enum {
    /**
	* Enable rendering
	*/
    ILB_ERS_RUN = 0,

    /**
	* Pause a render without. The renderworker will still be assigned to the job, but idle.
	*/
    ILB_ERS_PAUSE

} ILBErnstRunningState;

/**
 * Sets how to handle the Beast window when rendering
 */
typedef enum {
    /**
	 * Don't display the render window / progress view
	 */
    ILB_SR_NO_DISPLAY = 0,

    /**
	* Show the render window / progress view and close it when the rendering is done.
	*/
    ILB_SR_CLOSE_WHEN_DONE,

    /**
	* Show the render window and keep it open until the user closes it or the job is destroyed.
	* NOTE: Interpreted as ILB_SR_CLOSE_WHEN_DONE when distribution is used.
	*/
    ILB_SR_KEEP_OPEN
} ILBShowResults;

/**
 * Sets how beast should render distributed
 */
typedef enum {
    /**
	 * Force a local render
	 */
    ILB_RD_FORCE_LOCAL = 0,

    /**
	 * Render distributed if possible, otherwise
	 * fallback on local rendering
	 */
    ILB_RD_AUTODETECT,

    /**
	 * Force a distributed render, fails if distribution is not
	 * available
	 */
    ILB_RD_FORCE_DISTRIBUTED
} ILBDistributionType;

/**
 * Different types of updates which can come from Ernst
 */
typedef enum {
    /**
	 * Specifies a new light source created in Ernst
	 */
    ILB_UT_NEW_LIGHTSOURCE = 0,

    /**
	 * Specifies an update to a previously existing light source
	 */
    ILB_UT_UPDATE_LIGHTSOURCE,

    /**
	 * Specifies that a light sources has been deleted in Ernst
	 */
    ILB_UT_DELETE_LIGHTSOURCE,

    /**
	 * Global information about the scene
	 */
    ILB_UT_SCENE_INFO,

    /**
	 * Specifies an update to a previously existing camera
	 */
    ILB_UT_UPDATE_CAMERA,

    /**
	 * Specifies an update to a previously existing target entity
	 */
    ILB_UT_UPDATE_TARGET,

    /**
	 * Specifies a generated light map from a Live Ernst job
	 */
    ILB_UT_UPDATE_TEXTURE,

    /**
	 * Specifies vertex lighting from a Live Ernst job
	 */
    ILB_UT_UPDATE_VERTEX,
    /**
	 * Specifies an update camera frame buffer from a Live Ernst job
	 */
    ILB_UT_UPDATE_CAMERA_FRAME_BUFFER,

    /**
	 * Message that the lighting setup of a scene has changed
	 * and that the specified target does no longer contain
	 * up-to-date data. Used in Live Ernst jobs.
	 */
    ILB_UT_INVALIDATE_FRAMEBUFFER

} ILBUpdateType;

/**
 * Different baking modes for a Live Ernst session
 */
typedef enum {
    /**
	 * Only bake what the camera sees
	 */
    ILB_BM_CAMERA_FRUSTUM,

    /**
	 * Bake all targets in the scene, prioritized by the
	 * areas with the most variance
	 */
    ILB_BM_ENTIRE_LEVEL
} ILBErnstBakingModeType;

/**
 * Controls the type of caustics to include in physical rendering (both Ernst and Beast)
 */
typedef enum {
    /**
	 * No caustics at all
	 */
    ILB_CM_NONE,

    /**
	 * Caustics with one diffuse interaction before all specular interactions are allowed
	 * \verbatim (CD{0,1}[SGs]*D*<Ts>*[LO]) \endverbatim
	 */
    ILB_CM_SOME,

    /**
	 * All caustic light paths allowed
	 */
    ILB_CM_ALL
} ILBCausticsModeType;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** 
 * Creates a Beast job.
 * @param beastManager the beast manager to create the job for
 * @param uniqueName a unique name for the job
 * @param scene the scene to render
 * @param jobXML the config XML file to use, or an empty string to use defaults for all settings
 * @param job pointer to where the job handle should be stored
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateJob(ILBManagerHandle beastManager,
                                        ILBConstString uniqueName,
                                        ILBSceneHandle scene,
                                        ILBConstString jobXML,
                                        ILBJobHandle* job);

/** 
 * Creates an Ernst job. Ernst jobs works like bake jobs but does 
 * not use render passes. All texture and vertex targets added to 
 * an Ernst job will be lit in the Ernst session.
 * @param beastManager the beast manager to create the job for
 * @param uniqueName a unique name for the job
 * @param scene the scene to render
 * @param jobXML the config XML file to use, or an empty string to use defaults for all settings
 * @param job pointer to where the job handle should be stored
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateErnstJob(ILBManagerHandle beastManager,
                                             ILBConstString uniqueName,
                                             ILBSceneHandle scene,
                                             ILBConstString jobXML,
                                             ILBJobHandle* job);

/** 
 * Creates a Live Ernst job. The scene will be locked and cannot
 * be used for other jobs until the Live Ernst job is done.
 * @param beastManager the beast manager to create the job for
 * @param uniqueName a unique name for the job
 * @param scene the scene to render
 * @param job pointer to where the job handle should be stored
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateLiveErnstJob(ILBManagerHandle beastManager,
                                                 ILBConstString uniqueName,
                                                 ILBSceneHandle scene,
                                                 ILBJobHandle* job);

/** 
 * Creates an UV job. 
 * @param beastManager the beast manager to create the job for
 * @param uniqueName a unique name for the job
 * @param job pointer to where the job handle should be stored
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateUVJob(ILBManagerHandle beastManager,
                                          ILBConstString uniqueName,
                                          ILBJobHandle* job);

/** 
 * Checks if a job has a new update and retrieves it
 * @param job the job to get updates from
 * @param hasUpdate set to true if new update is available
 * @param updateHandle if hasUpdate is true this will contain a handle to the update
 */
ILB_DLL_FUNCTION ILBStatus ILBGetJobUpdate(ILBJobHandle job,
                                           ILBBool* hasUpdate,
                                           ILBJobUpdateHandle* updateHandle);

/** 
 * Gets the type of update
 * @param update handle to the update
 * @param updateType returns the update type
 */
ILB_DLL_FUNCTION ILBStatus ILBGetJobUpdateType(ILBJobUpdateHandle update,
                                               ILBUpdateType* updateType);

/** 
 * Destroys a job update
 * @param update the job update to destroy
 */
ILB_DLL_FUNCTION ILBStatus ILBDestroyUpdate(ILBJobUpdateHandle update);

/** 
 * Get an updated light source
 * @param update the job update
 * @param light the updated light source
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateLightSource(ILBJobUpdateHandle update,
                                                   ILBLightHandle* light);

/** 
 * Get an updated scene info node
 * @param update the job update
 * @param sceneInfo the updated scene info node
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateSceneInfo(ILBJobUpdateHandle update,
                                                 ILBSceneInfoHandle* sceneInfo);

/** 
 * Get an updated camera
 * @param update the job update
 * @param camera the updated camera
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateCamera(ILBJobUpdateHandle update,
                                              ILBCameraHandle* camera);

/** 
 * Get an updated target entity
 * @param update the job update
 * @param targetEntity the updated target entity
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateTargetEntity(ILBJobUpdateHandle update,
                                                    ILBTargetEntityHandle* targetEntity);

/** 
 * Get an updated frame buffer
 * @param update the job update
 * @param fb the updated frame buffer
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateFramebuffer(ILBJobUpdateHandle update,
                                                   ILBFramebufferHandle* fb);

/** 
 * Get the target entity of an invalidated frame buffer update
 * @param update the job update
 * @param te the target entity of the invalidated frame buffer
 */
ILB_DLL_FUNCTION ILBStatus ILBGetUpdateInvalidateFramebuffer(ILBJobUpdateHandle update,
                                                             ILBTargetEntityHandle* te);

/**
 * Destroys a job
 * @param job the job to destroy
 */
ILB_DLL_FUNCTION ILBStatus ILBDestroyJob(ILBJobHandle job);

/**
* Sets the output directory for the job. If this function is not called output files will
* end up in the cache hierarchy.
* @param job the job to set directory for
* @param path the path to the output directory
*/
ILB_DLL_FUNCTION ILBStatus ILBSetJobOutputPath(ILBJobHandle job, ILBConstString path);

/**
* Sets the number of processing threads on the local submitting machine. Only available for Ernst jobs.
* If this function is not called, Ernst will use one thread for every available core.
* @param job the job to set directory for
* @param numLocalProcessingThreads the number of local threads used
*/
ILB_DLL_FUNCTION ILBStatus ILBSetJobLocalProcessingThreads(ILBJobHandle job, int numLocalProcessingThreads);

/** 
 * Starts a job
 * @param job the job to start
 * @param showResults Specifies the behaviour of the render window
 * @param distribution Sets how to distribute the rendering
 */
ILB_DLL_FUNCTION ILBStatus ILBStartJob(ILBJobHandle job, ILBShowResults showResults, ILBDistributionType distribution);

/** 
* Waits until a job is done or until there is progress updates
* @param job The job to wait for
* @param timeout The maximum time to wait in milliseconds
*/
ILB_DLL_FUNCTION ILBStatus ILBWaitJobDone(ILBJobHandle job, int32 timeout);

/** 
* Checks if the job is running.
* @param job The job to check
* @param result Is set to true if job is running, false otherwise
*/
ILB_DLL_FUNCTION ILBStatus ILBIsJobRunning(ILBJobHandle job, ILBBool* result);

/** 
* Checks if the job is completed. Note that a running job can be completed if
* the user has selected to keep the render window open. A job that is not running
* might not have finished if it was aborted or had errors.
* @param job the job to check
* @param result set to true if the job is completed, false otherwise
*/
ILB_DLL_FUNCTION ILBStatus ILBIsJobCompleted(ILBJobHandle job, ILBBool* result);

/**
* Returns the result of the job as a JobStatus
* @param job the job to get the result for
* @param status pointer to the JobStatus
*/
ILB_DLL_FUNCTION ILBStatus ILBGetJobResult(ILBJobHandle job, ILBJobStatus* status);

/**
 * Cancels a running job
 * @param job the job to cancel
 */
ILB_DLL_FUNCTION ILBStatus ILBCancelJob(ILBJobHandle job);

/**
 * Gets the current status of a job.
 * @param job the job to get progress for
 * @param jobName pointer to a string object that receives the name of job being executed
 * Set to 0 to ignore this parameter. 
 * @param progress to the completion percentage of the current activity
 */
ILB_DLL_FUNCTION ILBStatus ILBGetJobProgress(ILBJobHandle job, ILBStringHandle* jobName, int32* progress);

/**
 * Checks if the progress of a Job has been updated since the last time ILBGetJobProgress was called.
 * @param job The job to check if it has progress
 * @param newActivity set to true if a new activity has started
 * @param newProgress set to true if the progress has been updated
 */
ILB_DLL_FUNCTION ILBStatus ILBJobHasNewProgress(ILBJobHandle job, ILBBool* newActivity, ILBBool* newProgress);

/**
 * Convenience function to execute a Beast job.
 * Blocks until the job is done or fails
 * @deprecated This function will go away in the future
 * @param bm the beast manager to use
 * @param job the job to execute
 * @param showResults sets how the beast window should be handled
 * @param distribution Sets how to distribute the rendering
 * @param status the result of the rendering
 */
ILB_DLL_FUNCTION ILBStatus ILBExecuteBeast(ILBManagerHandle bm,
                                           ILBJobHandle job,
                                           ILBShowResults showResults,
                                           ILBDistributionType distribution,
                                           ILBJobStatus* status);

/**
 * Sets the amount of direct illumination to use in a Live Ernst job.
 * @param job the Live Ernst job
 * @param illumination the desired amount of direct illumination
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstDirectIllumination(ILBJobHandle job, float illumination);

/**
* Sets the amount of indirect illumination to use in a Live Ernst job.
* @param job the Live Ernst job
* @param illumination the desired amount of indirect illumination
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstIndirectIllumination(ILBJobHandle job, float illumination);

/**
 * Sets the variance threshold in a Live Ernst job.
 * Please refer the Ernst documentation for more information.
 * @param job the Live Ernst job
 * @param threshold the variance threshold
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstVarianceThreshold(ILBJobHandle job, float threshold);

/**
 * Sets the display gamma for the Ernst editor.
 * @param job the Live Ernst job
 * @param displayGamma the display gamma
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstDisplayGamma(ILBJobHandle job, float displayGamma);

/**
 * Sets the GI depth in a Live Ernst job.
 * @param job the Live Ernst job
 * @param minDepth the minimum depth of a traced path
 * @param maxDepth the maximum depth of a traced path
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstGIDepth(ILBJobHandle job, int minDepth, int maxDepth);

/**
 * Sets the GI baking mode in a Live Ernst job.
 * @param job the Live Ernst job
 * @param mode controls whether to bake what the camera sees or the entire 
 * scene
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstGIBakingMode(ILBJobHandle job, ILBErnstBakingModeType mode);

/**
 * Sets the variance limit in a Live Ernst job.
 * Please refer the Ernst documentation for more information.
 * @param job the Live Ernst job
 * @param vl the variance limit
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstGIVarianceLimit(ILBJobHandle job, float vl);

/**
 * Sets the GI diffuse boost in a Live Ernst job.
 * Please refer the Ernst documentation for more information.
 * @param job the Live Ernst job
 * @param boost the diffuse boost
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstGIDiffuseBoost(ILBJobHandle job, float boost);

/**
 * Sets the GI emissive scale in a Live Ernst job.
 * Please refer the Ernst documentation for more information.
 * @param job the Live Ernst job
 * @param scale the emissive scale
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstGIEmissiveScale(ILBJobHandle job, float scale);

/**
 * Sets the running state of a Live Ernst job.
 * Allows pausing and restarting a job without terminating it.
 * @param job the Live Ernst job
 * @param state whether the job should be paused or running
 */
ILB_DLL_FUNCTION ILBStatus ILBSetErnstRunningState(ILBJobHandle job, ILBErnstRunningState state);

/**
 * Sets the render quality to use for a job. 
 * The quality value range from 0.0 to 1.0. Default value is 0.5.
 * A low value will give fast but noisy results, a high value will give less noise but longer render times.
 * This quality value will be used to derive appropriate render sampling settings. If you need full control of the 
 * sampling settings, you can use the function ILBSetJobRenderSamples instead, to set the sampling settings explicitly.
 * @param job the job
 * @param quality the quality value
 */
ILB_DLL_FUNCTION ILBStatus ILBSetJobRenderQuality(ILBJobHandle job, float quality);

/**
 * Explicitly sets the sampling settings to use for a job. 
 * These are automatically derived when using ILBSetJobRenderQuality, so consider using that function instead for a simplified render setup.
 * @param job the job
 * @param minSamples the minimum number of samples to use for each pixel
 * @param maxSamples the maximum number of samples to use for each pixel
 * @param varianceThreshold the threshold for variance where a pixel is considered to be sampled well enough
 */
ILB_DLL_FUNCTION ILBStatus ILBSetJobRenderSamples(ILBJobHandle job, int minSamples, int maxSamples, float varianceThreshold);

/**
 * Sets the minimum and maximum number of light bounces to use for rendering.
 * @param job the job
 * @param minDepth the minimum depth of a traced path
 * @param maxDepth the maximum depth of a traced path
 */
ILB_DLL_FUNCTION ILBStatus ILBSetJobRenderDepth(ILBJobHandle job, int minDepth, int maxDepth);

/**
 * Enables or disables caustics in physical rendering.
 * @param job the job
 * @param mode the caustics mode, explained in the enum definition.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetCausticsMode(ILBJobHandle job, ILBCausticsModeType mode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //BEASTJOB_H
