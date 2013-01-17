#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"
#include "Render/Highlevel/Camera.h"

namespace DAVA
{

ParticleEmitter3D::ParticleEmitter3D()
{
	is3D = true;
}

void ParticleEmitter3D::Draw(Camera * camera)
{
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		ParticleLayer3D * layer = (ParticleLayer3D*)(*it);
		if(!layer->isDisabled)
		{
			layer->Draw(camera);
		}
	}
}



}


