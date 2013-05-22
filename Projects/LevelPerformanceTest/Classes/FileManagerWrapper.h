#ifndef SurfaceTester_FileManagerWrapper_h
#define SurfaceTester_FileManagerWrapper_h

#include "DAVAEngine.h"

class FileManagerWrapper
{
	FileManagerWrapper();
	FileManagerWrapper(const FileManagerWrapper&);
	FileManagerWrapper& operator=(const FileManagerWrapper&);
public:
	static const DAVA::Vector<DAVA::String> GetFileListByExtension(const DAVA::String& path, const DAVA::String& ext, DAVA::int32 maxLevel = 0);
};

#endif
