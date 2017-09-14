/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The api for specifying scenes in beast
 */ 
#ifndef BEASTSCENE_H
#define BEASTSCENE_H

#include "beastapitypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
	* The scene up vector.
	*/
typedef enum {
    ILB_UP_POS_X = 0, /*!< Positive direction of the X axis. */
    ILB_UP_NEG_X, /*!< Negative direction of the X axis. */
    ILB_UP_POS_Y, /*!< Positive direction of the Y axis. */
    ILB_UP_NEG_Y, /*!< Negative direction of the Y axis. */
    ILB_UP_POS_Z, /*!< Positive direction of the Z axis. */
    ILB_UP_NEG_Z /*!< Negative direction of the Z axis. */
} ILBSceneUpVector;

/** 
	 * Begins creation of a scene that will use "classic" fixed-function materials.
	 * @param beastManager the beast manager this scene will be associated with
	 * @param uniqueName a unique name for the scene.
	 * @param target a pointer to a Beast scene object that will receive the created object
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBBeginScene(ILBManagerHandle beastManager,
                                         ILBConstString uniqueName,
                                         ILBSceneHandle* target);

/** 
	 * Begins creation of a scene that will use physical materials set up with OSL shaders.
	 * @param beastManager the beast manager this scene will be associated with
	 * @param uniqueName a unique name for the scene.
	 * @param target a pointer to a Beast scene object that will receive the created object
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBBeginPhysicalScene(ILBManagerHandle beastManager,
                                                 ILBConstString uniqueName,
                                                 ILBSceneHandle* target);

/** 
	 * Releases the scene data.\n
	 * All handles created in this scene will be invalid after this call.
	 * @param scene the scene to to release.
	 */
ILB_DLL_FUNCTION ILBStatus ILBReleaseScene(ILBSceneHandle scene);

/** 
	 * Finalizes this scene
	 * Any future call to modify this scene or any of its objects will fail
	 * @param scene the scene to finalize
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBEndScene(ILBSceneHandle scene);

/** 
	* Sets how many meters a world unit in represents for the scene.
	* This parameter is optional. It is used by eRnsT optimize the user
	* experience in the external editor.
	* @param scene the current scene
	* @param meterPerWorldUnit the relation between a meter and a world unit in the scene.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetMeterPerWorldUnit(ILBSceneHandle scene, float meterPerWorldUnit);

/**
	* Sets the up vector of the scene. 
	* This parameter is optional. It is used by eRnsT editor. 
	* @param scene the current scene
	* @param upVector the direction of the up vector.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBSetSceneUpVector(ILBSceneHandle scene, ILBSceneUpVector upVector);

/** 
	* Gets how many meters a world unit in represents for the scene.
	* This parameter is returned from an Ernst session in a SceneInfo node.
	* @param scene the current scene
	* @param meterPerWorldUnit the relation between a meter and a world unit in the scene.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBGetMeterPerWorldUnit(ILBSceneInfoHandle scene, float* meterPerWorldUnit);

/**
	* Gets the up vector of the scene. 
	* This parameter is returned from an Ernst session in a SceneInfo node.
	* @param scene the current scene
	* @param upVector the direction of the up vector.
	* @return The result of the operation.
	*/
ILB_DLL_FUNCTION ILBStatus ILBGetSceneUpVector(ILBSceneInfoHandle scene, ILBSceneUpVector* upVector);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTSCENE_H
