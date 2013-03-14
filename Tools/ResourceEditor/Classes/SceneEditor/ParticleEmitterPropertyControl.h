#ifndef __PARTCLEEMITTER_PROPERTY_CONTROL_H__
#define __PARTCLEEMITTER_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class ParticleEmitterPropertyControl : public NodesPropertyControl
{
public:
	ParticleEmitterPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~ParticleEmitterPropertyControl();

	virtual void ReadFrom(Entity * sceneNode);
};

#endif //__PARTCLEEMITTER_PROPERTY_CONTROL_H__
