#ifdef __DAVAENGINE_BEAST__

#include "BeastProxyImpl.h"
#include "BeastManager.h"
#include "BeastTexture.h"

BeastManager * BeastProxyImpl::CreateManager()
{
	return new BeastManager();
}

void BeastProxyImpl::SafeDeleteManager(BeastManager ** manager)
{
	delete(*manager);
	*manager = 0;
}

void BeastProxyImpl::ParseScene(BeastManager * manager, DAVA::Scene * scene)
{
	manager->ParseScene(scene);
}

#endif //__DAVAENGINE_BEAST__
