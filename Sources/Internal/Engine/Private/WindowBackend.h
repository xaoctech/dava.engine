#pragma once

#if defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/WindowBackendQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Private/Win32/Window/WindowBackendWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Private/Win10/Window/WindowBackendWin10.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Private/Mac/Window/WindowBackendMac.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"
#elif defined(__DAVAENGINE_LINUX__)
#include "Engine/Private/Linux/Window/WindowBackendLinux.h"
#else
#error "WindowBackend is not implemented"
#endif
