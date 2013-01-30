#ifndef __DAVAENGINE_PARTCLEEMITTER_NODE_H__
#define __DAVAENGINE_PARTCLEEMITTER_NODE_H__

#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleEmitter3D.h"

namespace DAVA
{

class Material;
class ParticleEmitterNode : public SceneNode
{
public:
	ParticleEmitterNode();
	virtual ~ParticleEmitterNode();

	void LoadFromYaml(const String& yamlPath);
    void SaveToYaml(const String& _yamlPath);

	String GetYamlPath();

	ParticleEmitter * GetEmitter();

	virtual void Update(float32 timeElapsed);
	virtual void Draw();

	virtual SceneNode* Clone(SceneNode *dstNode = NULL);
	virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);

	virtual void GetDataNodes(Set<DataNode*> & dataNodes);

private:
	ParticleEmitter3D * emitter;
	String yamlPath;
};

};

#endif //__DAVAENGINE_PARTCLEEMITTER_NODE_H__
