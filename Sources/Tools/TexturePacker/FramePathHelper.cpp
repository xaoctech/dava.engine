#include "FramePathHelper.h"
#include "Utils/StringFormat.h"

namespace DAVA {

static const char* DEFAULT_FRAME_EXTENSION = ".png";

String FramePathHelper::GetFramePathRelative(const String& nameWithoutExt, int32 frameIndex)
{
	return Format("%s_%d%s", nameWithoutExt.c_str(), frameIndex, DEFAULT_FRAME_EXTENSION);
}

FilePath FramePathHelper::GetFramePathAbsolute(const FilePath& directory, const String& nameWithoutExt,
									 int32 frameIndex)
{
	return directory + GetFramePathRelative(nameWithoutExt, frameIndex);
}

};