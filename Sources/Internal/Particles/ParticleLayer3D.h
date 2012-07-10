#ifndef __DAVAENGINE_PARTICLE_LAYER_3D_H__
#define __DAVAENGINE_PARTICLE_LAYER_3D_H__

#include "Base/BaseTypes.h"
#include "Particles/ParticleLayer.h"
#include "Math/Matrix4.h"


namespace DAVA
{

class RenderDataObject;
class Material;
class ParticleLayer3D : public ParticleLayer
{
public:
	ParticleLayer3D();
	virtual ~ParticleLayer3D();

	void Draw(const Vector3 & up, const Vector3 & left, const Vector3 & direction);

protected:
	RenderDataObject * renderData;
	Vector<float32> verts;
	Vector<float32> textures;
	Vector<uint32> colors;

	Material * material;
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_3D_H__
