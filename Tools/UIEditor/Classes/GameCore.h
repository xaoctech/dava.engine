#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"

namespace DAVA {
	
class GameCore : public DAVA::ApplicationCore
{
public:	
	GameCore();
	virtual ~GameCore();
	
	virtual void OnAppStarted();
	virtual void OnAppFinished();
	
	virtual void OnSuspend();
	virtual void OnResume();
	virtual void OnBackground();
	
	virtual void BeginFrame();
	virtual void Update(DAVA::float32 update);
	virtual void Draw();
	
private:
	Cursor * cursor;
};

}

#endif // __GAMECORE_H__