#ifndef SurfaceTester_FileManagerWrapper_h
#define SurfaceTester_FileManagerWrapper_h

#include "DAVAEngine.h"

using namespace DAVA;

class FileManagerWrapper
{
	FileManagerWrapper();
	FileManagerWrapper(const FileManagerWrapper&);
	FileManagerWrapper& operator=(const FileManagerWrapper&);
public:
	static const Vector<String> GetFileListByExtension(const String& path, const String& ext, int32 maxLevel = 0);
};

#endif
