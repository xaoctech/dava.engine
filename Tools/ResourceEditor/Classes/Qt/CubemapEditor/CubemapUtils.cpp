#include "CUbemapEditor/CubemapUtils.h"
#include "Render/Texture.h"

#define CUBEMAPEDITOR_MAXFACES 6

static int UI_TO_FRAMEWORK_FACE[] = {
	DAVA::Texture::CUBE_FACE_POSITIVE_X,
	DAVA::Texture::CUBE_FACE_NEGATIVE_X,
	DAVA::Texture::CUBE_FACE_POSITIVE_Y,
	DAVA::Texture::CUBE_FACE_NEGATIVE_Y,
	DAVA::Texture::CUBE_FACE_POSITIVE_Z,
	DAVA::Texture::CUBE_FACE_NEGATIVE_Z
};

static int FRAMEWORK_TO_UI_FACE[] = {
	CUBEMAPEDITOR_FACE_PX,
	CUBEMAPEDITOR_FACE_NX,
	CUBEMAPEDITOR_FACE_PY,
	CUBEMAPEDITOR_FACE_NY,
	CUBEMAPEDITOR_FACE_PZ,
	CUBEMAPEDITOR_FACE_NZ
};


static DAVA::String FACE_NAME_SUFFIX[] = {
	DAVA::String("_px"),
	DAVA::String("_nx"),
	DAVA::String("_py"),
	DAVA::String("_ny"),
	DAVA::String("_pz"),
	DAVA::String("_nz")
};

const DAVA::String FACE_FILE_TYPE = "png";

void CubemapUtils::GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::String>& faceNames)
{
	faceNames.clear();
	
	DAVA::FilePath filePath(baseName);
	
	DAVA::String fileNameWithoutExtension = filePath.GetFilename();
	DAVA::String extension = filePath.GetExtension();
	fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");

	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		DAVA::FilePath faceFilePath = baseName;
		faceFilePath.ReplaceFilename(fileNameWithoutExtension +
									 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) + "." +
									 CubemapUtils::GetDefaultFaceExtension());

		faceNames.push_back(faceFilePath.GetAbsolutePathname());
	}
}

int CubemapUtils::GetMaxFaces()
{
	return CUBEMAPEDITOR_MAXFACES;
}

int CubemapUtils::MapUIToFrameworkFace(int uiFace)
{
	return UI_TO_FRAMEWORK_FACE[uiFace];
}

int CubemapUtils::MapFrameworkToUIFace(int frameworkFace)
{
	return FRAMEWORK_TO_UI_FACE[frameworkFace];
}

const DAVA::String& CubemapUtils::GetFaceNameSuffix(int faceId)
{
	return FACE_NAME_SUFFIX[faceId];
}

const DAVA::String& CubemapUtils::GetDefaultFaceExtension()
{
	return FACE_FILE_TYPE;
}


