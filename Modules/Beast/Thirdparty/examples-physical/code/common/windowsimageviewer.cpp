/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

#include "windowsimageviewer.h"

/** \file 
* Windows Image Viewer
*/

namespace bex
{
WindowsImageViewer::WindowsImageViewer()
    :
    m_exitRequested(false)
    ,
    m_width(16)
    ,
    m_height(9)
    ,
    m_visible(false)
    ,
    m_bitmapInitialized(false)
{
    HINSTANCE instance = GetModuleHandle(NULL);

    WNDCLASS windowClass;
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = wndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = TEXT("BeastSampleWindow");
    RegisterClass(&windowClass);

    m_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    int windowWidth, windowHeight;
    calculateWindowSize(m_width, m_height, &windowWidth, &windowHeight);
    m_windowHandle = CreateWindow(windowClass.lpszClassName, TEXT("Image Viewer"),
                                  m_style, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
                                  NULL, NULL, instance, NULL);

    SetWindowLongPtr(m_windowHandle, GWLP_USERDATA, (LONG_PTR) this);

    m_deviceContext = GetDC(m_windowHandle);

    m_memoryDeviceContext = CreateCompatibleDC(m_deviceContext);
}

WindowsImageViewer::~WindowsImageViewer()
{
    HINSTANCE instance = GetModuleHandle(NULL);
    UnregisterClass(TEXT("BeastSampleWindow"), instance);

    releaseBitmap();
    DeleteDC(m_memoryDeviceContext);
}

void WindowsImageViewer::calculateWindowSize(int clientWidth, int clientHeight, int* windowWidth, int* windowHeight)
{
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = clientWidth;
    rect.bottom = clientHeight;
    AdjustWindowRect(&rect, m_style, FALSE);
    *windowWidth = rect.right - rect.left;
    *windowHeight = rect.bottom - rect.top;
}

void WindowsImageViewer::setClientSize(int width, int height)
{
    releaseBitmap();

    m_width = width;
    m_height = height;

    int windowWidth = 0;
    int windowHeight = 0;
    calculateWindowSize(width, height, &windowWidth, &windowHeight);

    UINT flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER;
    SetWindowPos(m_windowHandle, NULL, 0, 0, windowWidth, windowHeight, flags);

    UpdateWindow(m_windowHandle);

    createBitmap();
}

void WindowsImageViewer::createBitmap()
{
    m_memoryBitmap = CreateCompatibleBitmap(m_deviceContext, m_width, m_height);
    m_oldMemoryObject = SelectObject(m_memoryDeviceContext, m_memoryBitmap);

    m_bitmapInitialized = true;
}

void WindowsImageViewer::releaseBitmap()
{
    if (m_bitmapInitialized)
    {
        SelectObject(m_memoryDeviceContext, m_oldMemoryObject);
        DeleteObject(m_memoryBitmap);

        m_bitmapInitialized = false;
    }
}

void WindowsImageViewer::writeData(const unsigned char* data, int width, int height)
{
    if (m_width != width || m_height != height)
        setClientSize(width, height);

    if (!m_visible)
    {
        ShowWindow(m_windowHandle, SW_SHOW);
        SetForegroundWindow(m_windowHandle);
        m_visible = true;
    }

    BITMAPINFO bitmapInfo;
    memset(&bitmapInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 24;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    unsigned char* convertedData = new unsigned char[width * height * 3];

    for (int y = 0; y < height; ++y)
    {
        int flippedY = height - 1 - y;
        const unsigned char* sourceData = &data[y * width * 3];
        unsigned char* targetData = &convertedData[flippedY * width * 3];

        for (int x = 0; x < width; ++x)
        {
            targetData[2] = sourceData[0];
            targetData[1] = sourceData[1];
            targetData[0] = sourceData[2];

            sourceData += 3;
            targetData += 3;
        }
    }

    SetDIBits(m_memoryDeviceContext, m_memoryBitmap, 0, height, convertedData, &bitmapInfo, DIB_RGB_COLORS);
    InvalidateRect(m_windowHandle, NULL, FALSE);
    UpdateWindow(m_windowHandle);

    delete[] convertedData;
}

void WindowsImageViewer::update()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool WindowsImageViewer::isExitRequested() const
{
    return m_exitRequested;
}

void WindowsImageViewer::waitUntilClosed()
{
    while (!m_exitRequested)
    {
        update();
        Sleep(0);
    }
}

LRESULT CALLBACK WindowsImageViewer::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR userData = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    WindowsImageViewer* imageViewer = NULL;

    if (userData)
    {
        imageViewer = reinterpret_cast<WindowsImageViewer*>(userData);
    }

    switch (message)
    {
    case WM_CLOSE:
        imageViewer->m_exitRequested = true;
        return 1;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            imageViewer->m_exitRequested = true;

        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT paintStruct;
        HDC hdc = BeginPaint(imageViewer->m_windowHandle, &paintStruct);

        BitBlt(imageViewer->m_deviceContext, 0, 0, imageViewer->m_width, imageViewer->m_height, imageViewer->m_memoryDeviceContext, 0, 0, SRCCOPY);

        EndPaint(imageViewer->m_windowHandle, &paintStruct);

        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
}
