#ifndef __CUBEMAP_UTILS_H__
#define __CUBEMAP_UTILS_H__

#include "Base/BaseTypes.h"

#define CUBEMAPEDITOR_FACE_PX 0
#define CUBEMAPEDITOR_FACE_NX 1
#define CUBEMAPEDITOR_FACE_PY 2
#define CUBEMAPEDITOR_FACE_NY 3
#define CUBEMAPEDITOR_FACE_PZ 4
#define CUBEMAPEDITOR_FACE_NZ 5

class CubemapUtils
{
public:
	
	static void GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::String>& faceNames);
	
	static int GetMaxFaces();
	static int MapUIToFrameworkFace(int uiFace);
	static int MapFrameworkToUIFace(int frameworkFace);
	static const DAVA::String& GetFaceNameSuffix(int faceId);
	static const DAVA::String& GetDefaultFaceExtension();
};

#endif /* defined(__CUBEMAP_UTILS_H__) */
