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
	void Draw(Camera * camera);

protected:
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__