#ifdef __DAVAENGINE_BEAST__

#include "BeastTexture.h"
#include "BeastDebug.h"

BeastTexture::BeastTexture(ILBManagerHandle manager)
:	BeastResource(manager)
{

}

void BeastTexture::InitWithFile(const DAVA::String & filePath)
{
	DAVA::String fullPath = DAVA::FileSystem::Instance()->SystemPathForFrameworkPath(filePath);
	BEAST_VERIFY(ILBReferenceTexture(manager, filePath.c_str(), fullPath.c_str(), &texture));
}


#endif //__DAVAENGINE_BEAST__
