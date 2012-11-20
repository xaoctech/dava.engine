#include "Scene3D/ParticleEmitterNode.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Material.h"
#include "Particles/ParticleEmitter3D.h"

namespace DAVA
{

REGISTER_CLASS(ParticleEmitterNode);

ParticleEmitterNode::ParticleEmitterNode()
:	emitter(0)
{
	SetName("Particle emitter");
}

ParticleEmitterNode::~ParticleEmitterNode()
{
	SafeRelease(emitter);
}

void ParticleEmitterNode::Update(float32 timeElapsed)
{
	SceneNode::Update(timeElapsed);
	if(emitter)
	{
		Vector3 position = Vector3(worldTransform._30, worldTransform._31, worldTransform._32);
		emitter->SetPosition(position);
		emitter->Update(timeElapsed);
	}
}

void ParticleEmitterNode::Draw()
{
	if(emitter)
	{
		eBlendMode sblend = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dblend = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);

		ParticleEmitter3D * emitter3D = static_cast<ParticleEmitter3D*>(emitter);
		emitter3D->Draw(scene->GetCurrentCamera());

		RenderManager::Instance()->SetBlendMode(sblend, dblend);
	}
}

void ParticleEmitterNode::LoadFromYaml(String _yamlPath)
{
	yamlPath = _yamlPath;
	SafeRelease(emitter);
	emitter = new ParticleEmitter3D();
	emitter->LoadFromYaml(yamlPath);
}

String ParticleEmitterNode::GetYamlPath()
{
	return yamlPath;
}

ParticleEmitter * ParticleEmitterNode::GetEmitter()
{
	return emitter;
}

SceneNode* ParticleEmitterNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new ParticleEmitterNode();
	}

	SceneNode::Clone(dstNode);
	ParticleEmitterNode *nd = (ParticleEmitterNode *)dstNode;

	nd->yamlPath = yamlPath;
	nd->LoadFromYaml(yamlPath);

	return dstNode;
}

void ParticleEmitterNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	SceneNode::Save(archive, sceneFile);

	archive->SetString("yamlPath", sceneFile->AbsoluteToRelative(yamlPath));
}

void ParticleEmitterNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	SceneNode::Load(archive, sceneFile);
	
	yamlPath = archive->GetString("yamlPath");
	yamlPath = sceneFile->RelativeToAbsolute(yamlPath);
	LoadFromYaml(yamlPath);
}

};
