#ifndef __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
#define __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class RenderObject;
class ParticleEmitter;

class ParticleEmitterSystem
{
public:
	void AddIfEmitter(RenderObject * maybeEmitter);
	void RemoveIfEmitter(RenderObject * maybeEmitter);
	void Update(float32 timeElapsed);

private:
	//TODO: use HashMap
	Vector<ParticleEmitter*> emitters;
};

}

#endif //__DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
