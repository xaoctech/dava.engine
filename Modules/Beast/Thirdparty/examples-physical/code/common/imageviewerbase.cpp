/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

#include "windowsimageviewer.h"

#include "utils.h"

/** \file 
* Image Viewer Base
*/

namespace bex
{
void ImageViewerBase::writeFramebuffer(ILBFramebufferHandle framebuffer)
{
    int32 width, height;
    apiCall(ILBGetResolution(framebuffer, &width, &height));

    unsigned char* data = new unsigned char[width * height * 3];

    bex::apiCall(ILBReadRegionLDR(framebuffer, 0, 0, width, height, ILB_CS_RGB, 2.2f, data));
    writeData(data, width, height);

    delete[] data;
}
}
