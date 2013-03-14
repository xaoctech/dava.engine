#ifndef __PARTICLE_EFFECT_PROPERTY_CONTROL_H__
#define __PARTICLE_EFFECT_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class ParticleEffectPropertyControl : public NodesPropertyControl
{
public:
	ParticleEffectPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~ParticleEffectPropertyControl();

	virtual void ReadFrom(Entity * sceneNode);

protected:

	void OnStart(BaseObject * object, void * userData, void * callerData);
	void OnStop(BaseObject * object, void * userData, void * callerData);
	void OnRestart(BaseObject * object, void * userData, void * callerData);


};

#endif //__PARTICLE_EFFECT_PROPERTY_CONTROL_H__
