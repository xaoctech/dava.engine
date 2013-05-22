#ifndef __DAVAENGINE_PARTCLEEMITTER_NODE_H__
#define __DAVAENGINE_PARTCLEEMITTER_NODE_H__

#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleEmitter3D.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class Material;
class ParticleEmitterNode : public Entity
{
public:
	ParticleEmitterNode();
	virtual ~ParticleEmitterNode();

	ParticleEmitter * GetEmitter();

	virtual void Update(float32 timeElapsed);
	virtual void Draw();

	virtual Entity* Clone(Entity *dstNode = NULL);
	virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);

	virtual void GetDataNodes(Set<DataNode*> & dataNodes);

protected:
	void LoadFromYaml(const FilePath & yamlPath);

private:
	ParticleEmitter3D * emitter;
	FilePath yamlPath;
};

};

#endif //__DAVAENGINE_PARTCLEEMITTER_NODE_H__
