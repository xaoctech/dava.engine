#include "FileManagerWrapper.h"

const Vector<String> FileManagerWrapper::GetFileListByExtension(const String& path, const String& ext, int32 maxLevel)
{
    FileList fileList(path);

    Vector<String> list;
    for(int32 i = 0; i < fileList.GetCount(); ++i)
    {
        if(fileList.IsDirectory(i))
		{
			if (maxLevel > 0)
			{
				String subDirName = fileList.GetFilename(i) + "/";
				if (subDirName == "./" || subDirName == "../")
				{
					continue;
				}

				Vector<String> subDirList = GetFileListByExtension(path + subDirName, ext, maxLevel - 1);
				for (Vector<String>::iterator it = subDirList.begin(); it != subDirList.end(); ++it)
				{
					list.push_back(subDirName + *it);
				}
			}
			else
			{
				continue;
			}
		}

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