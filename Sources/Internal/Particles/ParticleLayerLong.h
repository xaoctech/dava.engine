#ifndef __DAVAENGINE_PARTICLE_LAYER_LONG_H__
#define __DAVAENGINE_PARTICLE_LAYER_LONG_H__

#include "Particles/ParticleLayer3D.h"

namespace DAVA
{
class ParticleLayerLong : public ParticleLayer3D
{
public:
	virtual void Draw(const Vector3 & up, const Vector3 & left);
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_LONG_H__
