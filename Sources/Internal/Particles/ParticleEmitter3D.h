#ifndef __DAVAENGINE_PARTICLE_EMITTER_3D_H__
#define __DAVAENGINE_PARTICLE_EMITTER_3D_H__

#include "Particles/ParticleEmitter.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class ParticleEmitter3D : public ParticleEmitter
{
public:
	void Draw(const Vector3 & up, const Vector3 & left, const Vector3 & direction);

protected:
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__