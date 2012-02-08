#ifndef __BEAST_PROXY__
#define __BEAST_PROXY__

#include "DAVAEngine.h"

class BeastManager;
class BeastProxy : public DAVA::Singleton<BeastProxy>
{
public:
	virtual BeastManager * CreateManager();
	virtual void SafeDeleteManager(BeastManager ** manager) {};

	virtual void ParseScene(BeastManager * manager, DAVA::Scene * scene) {};
	virtual void GenerateLightmaps(BeastManager * manager) {};
	virtual void SetLightmapsDirectory(BeastManager * manager, const DAVA::String & path) {};
	virtual void SetMode(BeastManager * manager, DAVA::int32 mode) {};
};

#endif //__BEAST_PROXY__
