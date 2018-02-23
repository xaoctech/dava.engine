#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/DataNodes/Settings/RESettings.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class RECommandNotificationObject;
class NMaterial;
class Entity;
class RenderBatch;
class EditorMaterialSystem : public SceneSystem, public EditorSceneSystem
{
public:
    EditorMaterialSystem(Scene* scene);
    ~EditorMaterialSystem();

    const Set<NMaterial*>& GetTopParents() const;

    Vector<Entity*> GetEntities(NMaterial*) const;
    Vector<ReflectedObject> GetMaterialOwners(NMaterial*) const;

    void SetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode, bool set);
    bool GetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode) const;

    void SetLightViewMode(int fullViewMode);
    int GetLightViewMode() const;

    void SetLightmapCanvasVisible(bool enable);
    bool IsLightmapCanvasVisible() const;

protected:
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    struct MaterialOwner
    {
        MaterialOwner() = default;
        MaterialOwner(const ReflectedObject& _object, const FastName& _name)
            : object(_object)
            , fieldName(_name)
        {
        }

        MaterialOwner(ReflectionBase* base);
        MaterialOwner(const Reflection::Field& field);

        bool operator==(const MaterialOwner& other) const
        {
            return (object.GetVoidPtr() == other.object.GetVoidPtr()) && (fieldName == other.fieldName);
        }

        bool IsEmpty() const
        {
            return (object.GetVoidPtr() == nullptr) && fieldName.empty();
        }

        ReflectedObject object;
        FastName fieldName;
    };

    struct MaterialMapping
    {
        MaterialMapping(const MaterialOwner& _owner, NMaterial* _material, Entity* _entity)
            : owner(_owner)
            , material(_material)
            , entity(_entity)
        {
        }

        MaterialOwner owner;
        NMaterial* material = nullptr;
        Entity* entity = nullptr;
    };

    void RegisterEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void AccumulateDependentCommands(REDependentCommandsHolder& holder) override;

    void AddMaterials(Entity* entity, const Reflection& ref, const Any& name);
    void RemoveMaterials(const Reflection& ref, const Any& key);

    void AddMaterial(const MaterialOwner&, NMaterial*, Entity*);
    void RemoveMaterial(const MaterialOwner& owner, NMaterial*);

    void CheckAndAddMaterialParent(NMaterial* material);
    void RemoveMaterialTopParent(NMaterial* material);

    void ApplyViewMode();
    void ApplyViewMode(NMaterial* material);

private:
    Vector<MaterialMapping> materialMapping;
    Set<NMaterial*> materialTopParents;
    uint32 curViewMode = CommonInternalSettings::RESOLVE_RESULT;
    bool showLightmapCanvas = false;
};
} // namespace DAVA