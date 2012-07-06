#ifndef __DAVAENGINE_PARTCLEEMITTER_NODE_H__
#define __DAVAENGINE_PARTCLEEMITTER_NODE_H__

#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter3D.h"

namespace DAVA
{

class ParticleEmitterNode : public SceneNode
{
public:
	ParticleEmitterNode();
	virtual ~ParticleEmitterNode();

	void SetEmitter(ParticleEmitter3D * emitter);

	virtual void Update(float32 timeElapsed);
	virtual void Draw();

private:
	ParticleEmitter3D * emitter;
};

};

#endif //__DAVAENGINE_PARTCLEEMITTER_NODE_H__
