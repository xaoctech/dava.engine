#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MANAGER__
#define __BEAST_MANAGER__

#pragma comment(lib,"beast32.lib")

#include "DAVAEngine.h"
#include "BeastTypes.h"

class BeastManager
{
public:
	BeastManager();
	~BeastManager();

	void BeginScene();
	void EndScene();

private:
	static void StaticInit();
	static bool isInited;

	ILBManagerHandle handle;
	ILBSceneHandle scene;
};

#endif //__BEAST_MANAGER__

#endif //__DAVAENGINE_BEAST__