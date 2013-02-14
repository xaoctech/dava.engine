#include "FileManagerWrapper.h"

const Vector<String> FileManagerWrapper::GetFileListByExtension(const String& path, const String& ext)
{
    FileList fileList(path);

    Vector<String> list;
    for(int32 i = 0; i < fileList.GetCount(); ++i)
    {
        if(fileList.IsDirectory(i))
            continue;

        String curFileName = fileList.GetFilename(i);
        int32 dotIndex = curFileName.rfind('.');
        if(dotIndex != String::npos)
        {
            String curFileExt = curFileName.substr(dotIndex);

            if(curFileExt == ext)
                list.push_back(curFileName);
        }
    }

    return list;
}