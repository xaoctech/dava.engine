/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* Windows Image Viewer
*/ 
#ifndef WINDOWSIMAGEVIEWER_H
#define WINDOWSIMAGEVIEWER_H

#include "imageviewerbase.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bex
{
class WindowsImageViewer : public ImageViewerBase
{
public:
    WindowsImageViewer();
    ~WindowsImageViewer();

    virtual void writeData(const unsigned char* data, int width, int height);

    virtual void update();
    virtual bool isExitRequested() const;
    virtual void waitUntilClosed();

protected:
    void calculateWindowSize(int clientWidth, int clientHeight, int* windowWidth, int* windowHeight);
    void setClientSize(int width, int height);

    void createBitmap();
    void releaseBitmap();

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    int m_width;
    int m_height;

    DWORD m_style;
    HWND m_windowHandle;
    HDC m_deviceContext;

    HDC m_memoryDeviceContext;
    HBITMAP m_memoryBitmap;
    HANDLE m_oldMemoryObject;

    bool m_visible;
    bool m_bitmapInitialized;
    bool m_exitRequested;

    static const char* s_windowClassName;
};
}


#endif //IMAGEVIEWER_H
