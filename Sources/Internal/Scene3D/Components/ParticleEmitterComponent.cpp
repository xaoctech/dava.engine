#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA
{

ParticleEmitterComponent::ParticleEmitterComponent()
:	particleEmitter(0)
{

}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	SafeRelease(particleEmitter);
}

Component * ParticleEmitterComponent::Clone()
{
	ParticleEmitterComponent * newComponent = new ParticleEmitterComponent();
	newComponent->SetParticleEmitter(particleEmitter->Clone());
	return newComponent;
}

void ParticleEmitterComponent::SetParticleEmitter(ParticleEmitter * _particleEmitter)
{
	RenderSystem::Instance()->UnregisterFromUpdate(particleEmitter);

	SafeRelease(particleEmitter);
	particleEmitter = SafeRetain(_particleEmitter);

	RenderSystem::Instance()->RegisterForUpdate(particleEmitter);
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
