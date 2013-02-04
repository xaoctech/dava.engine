#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleEmitter3D.h"

namespace DAVA
{

ParticleEmitterComponent::ParticleEmitterComponent()
:	particleEmitter(0)
{

}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	Scene::GetActiveScene()->GetRenderSystem()->UnregisterFromUpdate(particleEmitter);
	SafeRelease(particleEmitter);
}

Component * ParticleEmitterComponent::Clone(SceneNode * toEntity)
{
	ParticleEmitterComponent * newComponent = new ParticleEmitterComponent();
	newComponent->SetEntity(toEntity);

	ParticleEmitter * newEmitter = new ParticleEmitter3D();
	newEmitter->LoadFromYaml(particleEmitter->GetConfigPath());
	newComponent->SetParticleEmitter(newEmitter);
	SafeRelease(newEmitter);
	return newComponent;
}

void ParticleEmitterComponent::SetParticleEmitter(ParticleEmitter * _particleEmitter)
{
	Scene::GetActiveScene()->GetRenderSystem()->UnregisterFromUpdate(particleEmitter);

	SafeRelease(particleEmitter);
	particleEmitter = SafeRetain(_particleEmitter);

	Scene::GetActiveScene()->GetRenderSystem()->RegisterForUpdate(particleEmitter);
}

ParticleEmitter * ParticleEmitterComponent::GetParticleEmitter()
{
	return particleEmitter;
}

void ParticleEmitterComponent::LoadFromYaml(const String& yamlPath)
{
	particleEmitter->Cleanup();
	particleEmitter->LoadFromYaml(yamlPath);
}

void ParticleEmitterComponent::SaveToYaml(const String& yamlPath)
{
	particleEmitter->SaveToYaml(yamlPath);
}

String ParticleEmitterComponent::GetYamlPath()
{
	if (particleEmitter)
	{
		return particleEmitter->GetConfigPath();
	}

	return String();
}

}
