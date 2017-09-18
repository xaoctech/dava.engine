/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The beast camera function definitions
 */ 
#ifndef BEASTCAMERA_H
#define BEASTCAMERA_H
#include "beastapitypes.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
	 * Environment camera types
	 */
typedef enum {
    /**
		 * Cubemap environment camera.
		 */
    ILB_ECT_CUBEMAP = 0,

    /**
		 * Ball environment camera.
		 */
    ILB_ECT_BALL,

    /**
		 * Latlong environment camera.
		 */
    ILB_ECT_LATLONG
} ILBEnvironmentCameraType;

/** 
	 * Add a camera to the scene.
	 * The camera looks in the negative z direction, positive x in camera
	 * space maps to right in screen space.
	 * positive y maps to up i screen space.
	 * @param scene the scene the camera should be a part of
	 * @param name the name of the camera, must be unique within the scene.
	 * @param transform the object space to world space transform for this camera
	 * @param camera the handle to store the generated camera in
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBCreatePerspectiveCamera(ILBSceneHandle scene,
                                                      ILBConstString name,
                                                      const ILBMatrix4x4* transform,
                                                      ILBCameraHandle* camera);

/**
	 * Add an environment camera to the scene.
	 * @param scene the scene the camera should be a part of
	 * @param name the name of the camera, must be unique within the scene.
	 * @param transform the object space to world space transform for this camera
	 * @param type the type of environment camera, i.e Ball, Cubemap or Latlong
	 * @param camera the handle to store the generated camera in
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBCreateEnvironmentCamera(ILBSceneHandle scene,
                                                      ILBConstString name,
                                                      const ILBMatrix4x4* transform,
                                                      ILBEnvironmentCameraType type,
                                                      ILBCameraHandle* camera);

/** 
	* Sets the transform of a camera. 
	* @param camera the camera to set the transform for
	* @param transform the new camera transform
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetCameraTransform(ILBCameraHandle camera,
                                                 const ILBMatrix4x4* transform);

/** 
	* Sets the motion transforms of a motion blurred camera. 
	* @param camera the camera to set the transform for
	* @param transforms an array of transforms describing the camera motion
	* @param count the number of transforms in the array
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetCameraMotionTransforms(ILBCameraHandle camera,
                                                        const ILBMatrix4x4* transforms,
                                                        int32 count);

/** 
	 * Sets the fov of the camera.
	 * Only works on perspective cameras.
	 * The vertical fov will be generated from the horizontal fov and the image 
	 * dimensions
	 * @param camera the camera to set fov for
	 * @param horizontalFovRadians the field of view in the X direction in radians.\n
	 * Note that it refers to the complete field of vision, not the angle to the
	 * forward direction.
	 * Negative horizontalFovRadians is currently not supported
	 * @param pixelAspectRatio the aspect ratio of a pixel, expressed as the x / y
	 */
ILB_DLL_FUNCTION ILBStatus ILBSetFov(ILBCameraHandle camera,
                                     float horizontalFovRadians,
                                     float pixelAspectRatio);

/** 
	 * Sets the vertical fov of the camera.
	 * Only works on perspective cameras in scenes locked for use in a
	 * Live Ernst Job.
	 * @param camera the camera to set vertical fov for
	 * @param verticalFovRadians the field of view in the Y direction in radians.\n
	 * Note that it refers to the complete field of vision, not the angle to the
	 * forward direction.
	 * Negative horizontalFovRadians is not supported
	 */
ILB_DLL_FUNCTION ILBStatus ILBSetVerticalFov(ILBCameraHandle camera,
                                             float verticalFovRadians);

/** 
	 * Sets the display name for the camera.
	 * This name does not have to be unique within the scene and can be used by tools
	 * to give human understandable names to objects that has names that are generated
	 * and for some reason isn't suitable to read
	 * @param camera the camera to set the name for
	 * @param displayName the display name to set
	 */
ILB_DLL_FUNCTION ILBStatus ILBSetCameraDisplayName(ILBCameraHandle camera,
                                                   ILBConstString displayName);

/** 
	 * Sets depth of field parameters for the camera.
	 * @param camera the camera to set depth of field on
	 * @param fstop the aperture fstop
	 * @param focusDistance distance from camera to focal plane
	 * @param blades The number of shutter blades used to construct a non-circular dof
	 * a value less than 3 will produce a circular dof
	 */
ILB_DLL_FUNCTION ILBStatus ILBSetCameraDof(ILBCameraHandle camera,
                                           float fstop,
                                           float focusDistance,
                                           int32 blades);

/** 
	* Gets the transform of a camera. This is used for cameras returned from
	* an Ernst session.
	* @param camera the camera to get the transform for
	* @param transform the returned camera transform
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBGetCameraTransform(ILBCameraHandle camera,
                                                 ILBMatrix4x4* transform);

/** 
	* Gets the name of a camera. This is used for cameras returned from
	* an Ernst session.
	* @param camera the camera to get the name for
	* @param name the returned camera name
	*/
ILB_DLL_FUNCTION ILBStatus ILBGetCameraName(ILBCameraHandle camera,
                                            ILBStringHandle* name);

/** 
	 * Gets the fov of the camera.
	 * Only works on perspective cameras.
	 * The vertical fov will be generated from the horizontal fov and the image 
	 * dimensions
	 * @param camera the camera to get fov for
	 * @param horizontalFovRadians the field of view in the X direction in radians.\n
	 * Note that it refers to the complete field of vision, not the angle to the
	 * forward direction.
	 * @param pixelAspectRatio the aspect ratio of a pixel, expressed as the x / y
	 */
ILB_DLL_FUNCTION ILBStatus ILBGetCameraFov(ILBCameraHandle camera,
                                           float* horizontalFovRadians,
                                           float* pixelAspectRatio);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTCAMERA_H
