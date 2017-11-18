/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The api for specifying point clouds in beast
 */ 
#ifndef BEASTPOINTCLOUD_H
#define BEASTPOINTCLOUD_H

#include "beastapitypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** 
	 * Begins creation of a Point Cloud
	 * @param scene the scene the point cloud should be a part of
	 * @param name the name of the point cloud, must be unique within the scene.
	 * @param pointCloud a pointer to a point cloud handle to store the result in.
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBCreatePointCloud(ILBSceneHandle scene,
                                               ILBConstString name,
                                               ILBPointCloudHandle* pointCloud);

/** 
	 * Finalizes a point cloud.\n
	 * After this call it's impossible to add more points and it's
	 * possible to use the point cloud in a baking.
	 * @param pointCloud the point cloud to finalize
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBEndPointCloud(ILBPointCloudHandle pointCloud);

/** 
	 * Adds a chunk of point data to a point cloud. This can be called multiple times 
	 * to keep temporary buffer bounded.
	 * @param pointCloud the point cloud to add vertices to.
	 * @param pointData a pointer to an array of points
	 * @param normalData a pointer to an array of normals for the points
	 * @param pointCount the number of points and normals specified with this
	 * call.
	 * Behavior is undefined if vertexData or normalsData contains less than
	 * vertexCount vertices/normals
	 * @return The result of the operation.
	 */
ILB_DLL_FUNCTION ILBStatus ILBAddPointCloudData(ILBPointCloudHandle pointCloud,
                                                const ILBVec3* pointData,
                                                const ILBVec3* normalData,
                                                int32 pointCount);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTPOINTCLOUD_H
