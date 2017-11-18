/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The Beast uv layer functions
 */ 
#ifndef BEASTUVLAYER_H
#define BEASTUVLAYER_H

#include "beastapitypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** 
* Gets the number of uv values in the layer
* @param uvLayer the uv layer to get the size of
* @param size pointer to where the size shall be written
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBGetUVLayerSize(ILBUVLayerHandle uvLayer, int32* size);

/** 
* Reads a number of uv values from the uv layer
* @param uvLayer the uv layer to read values from
* @param start the index to the first element to read
* @param count the number of elements to read
* @param uvs pointer to an array where the values shall be written, must be large enough to hold all values
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBReadUVLayerValues(ILBUVLayerHandle uvLayer, int32 start, int32 count, ILBVec2* uvs);

/** 
* Gets the number of uv index values in the layer. 
* There is one index value for each face vertex of the mesh the layer was created for.
* @param uvLayer the uv layer to get the index size of
* @param size pointer to where the size shall be written
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBGetUVLayerIndexSize(ILBUVLayerHandle uvLayer, int32* size);

/** 
* Reads uv index values from the uv layer
* The index values are use to assign uvs to the mesh the uv layer was created for. 
* There is one index value for each face vertex of the mesh the layer was created for.
* For each face vertex in the mesh the uv index points into the uv array, specifying which uv to assign to that face vertex.
* @param uvLayer the uv layer to read values from
* @param start the index to the first element to read
* @param count the number of elements to read
* @param uvIndex pointer to an array where the values shall be written, must be large enough to hold all values
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBReadUVLayerIndexValues(ILBUVLayerHandle uvLayer, int32 start, int32 count, int32* uvIndex);

/**
* Destroys and frees all memory related to the uv layer.
* The uv layer handle will be invalid afterwards.\n
* @param uvLayer the uv layer to erase
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBDestroyUVLayer(ILBUVLayerHandle uvLayer);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTUVLAYER_H
