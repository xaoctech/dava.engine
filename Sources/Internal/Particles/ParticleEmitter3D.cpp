#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"

namespace DAVA
{

void ParticleEmitter3D::Draw(const Vector3 & up, const Vector3 & left)
{
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		ParticleLayer3D * layer = (ParticleLayer3D*)(*it);
		if(!layer->isDisabled)
		{
			layer->Draw(up, left);
		}
	}
}

}


