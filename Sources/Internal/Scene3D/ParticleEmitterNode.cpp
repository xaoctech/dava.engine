#include "Scene3D/ParticleEmitterNode.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Material.h"
#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"

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
	if(emitter)
	{
		const Matrix4 & worldTransform = GetWorldTransform();
		Vector3 position = Vector3(worldTransform._30, worldTransform._31, worldTransform._32);
		emitter->rotationMatrix = Matrix3(worldTransform);;
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
		RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);

//		ParticleEmitter3D * emitter3D = static_cast<ParticleEmitter3D*>(emitter);
//		emitter3D->Draw(scene->GetCurrentCamera());
		emitter->Draw(scene->GetCurrentCamera());

		RenderManager::Instance()->SetBlendMode(sblend, dblend);
	}
}

void ParticleEmitterNode::LoadFromYaml(const String& _yamlPath)
{
	yamlPath = _yamlPath;
	SafeRelease(emitter);
	emitter = new ParticleEmitter3D();
	emitter->LoadFromYaml(yamlPath);
}

ParticleEmitter * ParticleEmitterNode::GetEmitter()
{
	return emitter;
}

Entity* ParticleEmitterNode::Clone(Entity *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitterNode>(this), "Can clone only ParticleEmitterNode");
		dstNode = new ParticleEmitterNode();
	}

	Entity::Clone(dstNode);
	ParticleEmitterNode *nd = (ParticleEmitterNode *)dstNode;

	nd->yamlPath = yamlPath;
	nd->LoadFromYaml(yamlPath);

	return dstNode;
}

void ParticleEmitterNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	Entity::Save(archive, sceneFile);

	archive->SetString("yamlPath", sceneFile->AbsoluteToRelative(yamlPath));
}

void ParticleEmitterNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	Entity::Load(archive, sceneFile);
	
	yamlPath = archive->GetString("yamlPath");
	yamlPath = sceneFile->RelativeToAbsolute(yamlPath);
	LoadFromYaml(yamlPath);
}

void ParticleEmitterNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
	if(emitter)
	{
		int32 layersCount = emitter->GetLayers().size();
		for(int32 i = 0; i < layersCount; ++i)
		{
			ParticleLayer3D * layer = dynamic_cast<ParticleLayer3D*>(emitter->GetLayers()[i]);
			dataNodes.insert(layer->GetMaterial());
		}
	}
	

	Entity::GetDataNodes(dataNodes);
}

};

