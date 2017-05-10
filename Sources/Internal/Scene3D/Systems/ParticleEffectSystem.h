#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/BaseProcessSystem.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

namespace DAVA
{
class Component;
class ParticleEffectSystem : public SceneSystem
{
    friend class ParticleEffectComponent;
    friend class UIParticles;

public:
    struct MaterialData
    {
        Texture* texture = nullptr;
        bool enableFog = false;
        bool enableFrameBlend = false;
        Texture* flowmap = nullptr;
        bool enableFlow = false;

        eBlending blending = BLENDING_ALPHABLEND;

        MaterialData(Texture* texture_, bool enableFog_, bool enableFrameBlend_, Texture* flowmap_, bool enableFlow_, eBlending blending_)
            : texture(texture_)
            , enableFog(enableFog_)
            , enableFrameBlend(enableFrameBlend_)
            , flowmap(flowmap_)
            , enableFlow(enableFlow_)
            , blending(blending_)
        {
        }

        bool operator ==(const MaterialData& rhs)
        {
            return texture == rhs.texture
                && enableFog == rhs.enableFog
                && enableFrameBlend == rhs.enableFrameBlend
                && flowmap == rhs.flowmap
                && enableFlow == rhs.enableFlow
                && blending == rhs.blending;
        }
    };

    ParticleEffectSystem(Scene* scene, bool is2DMode = false);

    ~ParticleEffectSystem();
    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;

    void AddEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;

    void RemoveEntity(Entity* entity) override;
    void RemoveComponent(Entity* entity, Component* component) override;

    void SetGlobalMaterial(NMaterial* material);
    void SetGlobalExtertnalValue(const String& name, float32 value);
    float32 GetGlobalExternalValue(const String& name);
    Map<String, float32> GetGlobalExternals();

    inline void SetAllowLodDegrade(bool allowDegrade);
    inline bool GetAllowLodDegrade() const;

    inline const  Vector<std::pair<MaterialData, NMaterial*>>& GetMaterialInstances() const;

    void PrebuildMaterials(ParticleEffectComponent* component);

protected:
    void RunEffect(ParticleEffectComponent* effect);
    void AddToActive(ParticleEffectComponent* effect);
    void RemoveFromActive(ParticleEffectComponent* effect);

    void UpdateActiveLod(ParticleEffectComponent* effect);
    void UpdateEffect(ParticleEffectComponent* effect, float32 time, float32 shortEffectTime);
    Particle* GenerateNewParticle(ParticleEffectComponent* effect, ParticleGroup& group, float32 currLoopTime, const Matrix4& worldTransform);

    void PrepareEmitterParameters(Particle* particle, ParticleGroup& group, const Matrix4& worldTransform);
    void AddParticleToBBox(const Vector3& position, float radius, AABBox3& bbox);

    void RunEmitter(ParticleEffectComponent* effect, ParticleEmitter* emitter, const Vector3& spawnPosition, int32 positionSource = 0);

private:
    Map<String, float32> globalExternalValues;

    Vector<ParticleEffectComponent*> activeComponents;

private: //materials stuff

    NMaterial* particleBaseMaterial;
    Vector<std::pair<MaterialData, NMaterial*>> prebultMaterialsVector;
    NMaterial* GetMaterial(MaterialData&& materialData);

    bool allowLodDegrade;

    bool is2DMode;

};

inline const Vector<std::pair<ParticleEffectSystem::MaterialData, NMaterial*>>& ParticleEffectSystem::GetMaterialInstances() const
{
    return prebultMaterialsVector;
}

inline void ParticleEffectSystem::SetAllowLodDegrade(bool allowDegrade)
{
    allowLodDegrade = allowDegrade;
}
inline bool ParticleEffectSystem::GetAllowLodDegrade() const
{
    return allowLodDegrade;
}
};