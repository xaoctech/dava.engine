#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MANAGER__
#define __BEAST_MANAGER__

#pragma comment(lib,"beast32.lib")

#include "DAVAEngine.h"
#include "beastapi/beastmanager.h"

class BeastManager
{
public:
	BeastManager();
	~BeastManager();

private:
	static void Init();
	static bool isInited;

	ILBManagerHandle handle;
};

#endif //__BEAST_MANAGER__

#endif //__DAVAENGINE_BEAST__