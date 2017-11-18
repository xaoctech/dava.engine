/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* Image Viewer
*/ 
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#if defined(WIN32)

#include "windowsimageviewer.h"

namespace bex
{
typedef WindowsImageViewer ImageViewer;
}
#endif // WIN32

#endif // IMAGEVIEWER_H
