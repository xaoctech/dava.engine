#ifndef __DAVAENGINE_PARTICLE_EMITTER_3D_H__
#define __DAVAENGINE_PARTICLE_EMITTER_3D_H__

#include "Particles/ParticleEmitter.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class ParticleEmitter3D : public ParticleEmitter
{
public:
	void Draw(Matrix4 * transform);

protected:
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__