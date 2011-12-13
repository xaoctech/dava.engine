#ifdef __DAVAENGINE_BEAST__

#include "BeastManager.h"

bool BeastManager::isInited = false;

BeastManager::BeastManager()
{
	if(!isInited)
	{
		Init();
		isInited = true;
	}

	ILBCreateManager("./temp_beast", ILB_CS_LOCAL, &handle);
}

BeastManager::~BeastManager()
{

}

void BeastManager::Init()
{
	ILBSetLogTarget(ILB_LT_ERROR, ILB_LS_STDERR, 0);
	ILBSetLogTarget(ILB_LT_INFO, ILB_LS_STDOUT, 0);
}

#endif //__DAVAENGINE_BEAST__
