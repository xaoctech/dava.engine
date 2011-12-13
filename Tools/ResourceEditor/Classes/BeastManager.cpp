#ifdef __DAVAENGINE_BEAST__

#include "BeastManager.h"
#include "BeastDebug.h"

bool BeastManager::isInited = false;

BeastManager::BeastManager()
{
	if(!isInited)
	{
		StaticInit();
		isInited = true;
	}

	BEAST_VERIFY(ILBCreateManager("temp_beast", ILB_CS_LOCAL, &handle));
	BEAST_VERIFY(ILBSetBeastPath(handle, "../../Libs/beast/bin"));
}

BeastManager::~BeastManager()
{
	BEAST_VERIFY(ILBDestroyManager(handle));
}

void BeastManager::StaticInit()
{
	BEAST_VERIFY(ILBSetLogTarget(ILB_LT_ERROR, ILB_LS_STDERR, 0));
	BEAST_VERIFY(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_STDOUT, 0));
}

void BeastManager::BeginScene()
{
	BEAST_VERIFY(ILBBeginScene(handle, "sceneName", &scene));
}

void BeastManager::EndScene()
{
	BEAST_VERIFY(ILBEndScene(scene));
}

#endif //__DAVAENGINE_BEAST__
