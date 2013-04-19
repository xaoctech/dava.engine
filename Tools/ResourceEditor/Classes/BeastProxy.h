#ifndef __BEAST_PROXY__
#define __BEAST_PROXY__

#include "DAVAEngine.h"

class BeastManager;
struct LightmapAtlasingData;
class BeastProxy : public DAVA::Singleton<BeastProxy>
{
public:
	virtual BeastManager * CreateManager();
	virtual void SafeDeleteManager(BeastManager ** manager) {};
	virtual void Update(BeastManager * manager) {};
	virtual bool IsJobDone(BeastManager * manager) {return false;}

	virtual void Run(BeastManager * manager, DAVA::Scene * scene) {};
	virtual void SetLightmapsDirectory(BeastManager * manager, const DAVA::FilePath & path) {};
	virtual void SetMode(BeastManager * manager, DAVA::int32 mode) {};

	virtual void UpdateAtlas(BeastManager * manager, DAVA::Vector<LightmapAtlasingData> * atlasData) {};
};

#endif //__BEAST_PROXY__
