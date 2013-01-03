#ifndef __DAVAENGINE_PARTICLE_LAYER_LONG_H__
#define __DAVAENGINE_PARTICLE_LAYER_LONG_H__

#include "Particles/ParticleLayer3D.h"

namespace DAVA
{

class Camera;
class ParticleLayerLong : public ParticleLayer3D
{
public:
	ParticleLayerLong();

	virtual void Draw(Camera * camera);
    
    // Whether this layer is Long Layer?
    virtual bool IsLong() {return true;};
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_LONG_H__
