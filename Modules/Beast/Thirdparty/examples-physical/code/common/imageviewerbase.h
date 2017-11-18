/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* Image Viewer Base
*/ 
#ifndef IMAGEVIEWERBASE_H
#define IMAGEVIEWERBASE_H

#include <beastapi/beastframebuffer.h>

namespace bex
{
class ImageViewerBase
{
public:
    virtual ~ImageViewerBase()
    {
    }

    virtual void writeData(const unsigned char* data, int width, int height) = 0;
    virtual void writeFramebuffer(ILBFramebufferHandle framebuffer);

    virtual void update() = 0;
    virtual bool isExitRequested() const = 0;
    virtual void waitUntilClosed() = 0;
};
}

#endif // IMAGEVIEWERBASE_H
