#ifndef __DAVAENGINE_PARTICLE_EMITTER_3D_H__
#define __DAVAENGINE_PARTICLE_EMITTER_3D_H__

#include "Particles/ParticleEmitter.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class Camera;
class ParticleEmitter3D : public ParticleEmitter
{
public:
	ParticleEmitter3D();

	virtual void AddLayer(ParticleLayer * layer);
	virtual void AddLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove);

	virtual bool Is3DFlagCorrect();
	
	virtual void Draw();
	void Draw(Camera * camera);

protected:
	// Virtual methods which are different for 2D and 3D emitters.
	virtual void PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex);
	virtual void LoadParticleLayerFromYaml(YamlNode* yamlNode, bool isLiong);
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__