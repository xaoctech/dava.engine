#include "Platform/TemplateAndroid/FileListAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Logger/Logger.h"

namespace DAVA
{
JniFileList::JniFileList()
    : jniFileList("com/dava/framework/JNIFileList")
{
    getFileList = jniFileList.GetStaticMethod<jobjectArray, jstring>("GetFileList");
}

Vector<JniFileList::JniFileListEntry> JniFileList::GetFileList(const String& path)
{
    Vector<JniFileList::JniFileListEntry> fileList;
    JNIEnv* env = JNI::GetEnv();
    JNI::LocalRef<jstring> jPath = env->NewStringUTF(path.c_str());

    JNI::LocalRef<jobjectArray> jArray = getFileList(jPath);
    if (jArray)
    {
        jsize size = env->GetArrayLength(jArray);
        for (jsize i = 0; i < size; ++i)
        {
            JNI::LocalRef<jobject> item = env->GetObjectArrayElement(jArray, i);

            JNI::LocalRef<jclass> cls = env->GetObjectClass(item);
            jfieldID jNameField = env->GetFieldID(cls, "name", JNI::TypeSignature<jstring>::value());
            jfieldID jSizeField = env->GetFieldID(cls, "size", JNI::TypeSignature<jlong>::value());
            jfieldID jIsDirectoryField = env->GetFieldID(cls, "isDirectory", JNI::TypeSignature<jboolean>::value());

            jlong jSize = env->GetLongField(item, jSizeField);
            jboolean jIsDir = env->GetBooleanField(item, jIsDirectoryField);
            JNI::LocalRef<jstring> jName = static_cast<jstring>(env->GetObjectField(item, jNameField));

            JniFileListEntry entry;
            entry.name = JNI::ToString(jName);
            entry.size = jSize;
            entry.isDirectory = jIsDir;
            fileList.push_back(entry);
        }
    }

    return fileList;
}

} //namespace DAVA

#endif // __DAVAENGINE_ANDROID__
