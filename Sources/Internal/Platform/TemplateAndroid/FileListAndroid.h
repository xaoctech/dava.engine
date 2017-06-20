#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"

namespace DAVA
{
class JniFileList
{
public:
    struct JniFileListEntry
    {
        String name;
        uint32 size;
        bool isDirectory;
    };
    JniFileList();
    Vector<JniFileListEntry> GetFileList(const String& path);

private:
    JNI::JavaClass jniFileList;
    Function<jobjectArray(jstring)> getFileList;
};
}

#endif //#if defined(__DAVAENGINE_ANDROID__)
