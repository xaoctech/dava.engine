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

	void Draw(Camera * camera);
	virtual void RenderUpdate(Camera *camera, float32 timeElapsed);

	virtual RenderObject * Clone(RenderObject *newObject);

protected:
	// Virtual methods which are different for 2D and 3D emitters.
	virtual void PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex);
	virtual void LoadParticleLayerFromYaml(YamlNode* yamlNode, bool isLiong);
	
	// 3D-specific methods.
	void PrepareEmitterParametersShockwave(Particle * particle, float32 velocity,
										   int32 emitIndex, const Vector3& tempPosition,
										   const Matrix3& rotationMatrix);
	void PrepareEmitterParametersGeneric(Particle * particle, float32 velocity,
										 int32 emitIndex, const Vector3& tempPosition,
										 const Matrix3& rotationMatrix);
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__