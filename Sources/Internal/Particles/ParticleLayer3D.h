#ifndef __DAVAENGINE_PARTICLE_LAYER_3D_H__
#define __DAVAENGINE_PARTICLE_LAYER_3D_H__

#include "Base/BaseTypes.h"
#include "Particles/ParticleLayer.h"
#include "Math/Matrix4.h"


namespace DAVA
{

class RenderDataObject;
class Material;
class Camera;
class ParticleLayer3D : public ParticleLayer
{
public:
	ParticleLayer3D();
	virtual ~ParticleLayer3D();

	virtual void LoadFromYaml(const String & configPath, YamlNode * node);
	virtual ParticleLayer * Clone(ParticleLayer * dstLayer = 0);

	virtual void Draw(Camera * camera);

	Material * GetMaterial();

protected:
	RenderDataObject * renderData;
	Vector<float32> verts;
	Vector<float32> textures;
	Vector<uint32> colors;

	Material * material;
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_3D_H__
