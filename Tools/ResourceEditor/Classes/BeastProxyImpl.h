#ifndef __BEAST_PROXY_IMPL__
#define __BEAST_PROXY_IMPL__

#include "BeastProxy.h"

class BeastProxyImpl : public DAVA::Singleton<BeastProxy>
{
public:
	virtual BeastManager * CreateManager();
	virtual void SafeDeleteManager(BeastManager ** manager);

	virtual void ParseScene(BeastManager * manager, DAVA::Scene * scene);
};

#endif //__BEAST_PROXY_IMPL__
