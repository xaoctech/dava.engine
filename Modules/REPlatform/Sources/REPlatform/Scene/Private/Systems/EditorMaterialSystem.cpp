#include "REPlatform/Scene/Systems/EditorMaterialSystem.h"

#include "REPlatform/Commands/CloneLastBatchCommand.h"
#include "REPlatform/Commands/ConvertToShadowCommand.h"
#include "REPlatform/Commands/CopyLastLODCommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/DeleteRenderBatchCommand.h"
#include "REPlatform/Commands/InspDynamicModifyCommand.h"
#include "REPlatform/Commands/InspMemberModifyCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Commands/MaterialTreeModifyCommands.h"
#include "REPlatform/Commands/MaterialAssignCommand.h"

#include <Entity/ComponentUtils.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialManager.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Systems/LandscapeSystem.h>
#include <Scene3D/Systems/VTDecalSystem.h>

namespace DAVA
{
EditorMaterialSystem::MaterialOwner::MaterialOwner(ReflectionBase* base)
{
    Reflection ref = Reflection::Create(ReflectedObject(base));
    for (const Reflection::Field& f : ref.GetFields())
    {
        if (f.ref.GetValueType() == Type::Instance<NMaterial*>() && f.key.CanGet<FastName>())
        {
            object = f.ref.GetDirectObject();
            fieldName = f.key.Get<FastName>();
        }
    }
}

EditorMaterialSystem::MaterialOwner::MaterialOwner(const Reflection::Field& field)
{
    DVASSERT(field.ref.GetValueType() == Type::Instance<NMaterial*>() && field.key.CanGet<FastName>());

    object = field.ref.GetDirectObject();
    fieldName = field.key.Get<FastName>();
}

EditorMaterialSystem::EditorMaterialSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    NMaterialManager::Instance().SetGlobalFlag(NMaterialFlagName::FLAG_VIEWMODE, CommonInternalSettings::RESOLVE_RESULT);
}

EditorMaterialSystem::~EditorMaterialSystem()
{
    materialMapping.clear();

    for (auto op : materialTopParents)
    {
        SafeRelease(op);
    }
}

Vector<Entity*> EditorMaterialSystem::GetEntities(NMaterial* material) const
{
    Vector<Entity*> entities;
    for (const MaterialMapping& m : materialMapping)
    {
        if (m.material == material)
        {
            entities.emplace_back(m.entity);
        }
    }

    return entities;
}

Vector<ReflectedObject> EditorMaterialSystem::GetMaterialOwners(NMaterial* material) const
{
    Vector<ReflectedObject> owners;
    for (const MaterialMapping& m : materialMapping)
    {
        if ((m.material == material) && (m.owner.object.GetVoidPtr() != nullptr))
        {
            owners.emplace_back(m.owner.object);
        }
    }

    return owners;
}

const Set<NMaterial*>& EditorMaterialSystem::GetTopParents() const
{
    return materialTopParents;
}

int EditorMaterialSystem::GetLightViewMode() const
{
    return curViewMode;
}

bool EditorMaterialSystem::GetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode) const
{
    return (curViewMode & viewMode) != 0;
}

void EditorMaterialSystem::SetLightViewMode(int fullViewMode)
{
    if (curViewMode != fullViewMode)
    {
        curViewMode = fullViewMode;
        ApplyViewMode();
    }
}

void EditorMaterialSystem::SetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode, bool set)
{
    int newMode = curViewMode;

    if (set)
    {
        newMode |= viewMode;
    }
    else
    {
        newMode &= ~viewMode;
    }

    SetLightViewMode(newMode);
}

void EditorMaterialSystem::SetLightmapCanvasVisible(bool enable)
{
    if (enable != showLightmapCanvas)
    {
        showLightmapCanvas = enable;
        ApplyViewMode();
    }
}

bool EditorMaterialSystem::IsLightmapCanvasVisible() const
{
    return showLightmapCanvas;
}

void EditorMaterialSystem::RegisterEntity(Entity* entity)
{
    AddMaterials(entity, Reflection::Create(ReflectedObject(entity)), Any());
}

void EditorMaterialSystem::RemoveEntity(Entity* entity)
{
    int32 size = int32(materialMapping.size());
    for (int32 i = size - 1; i >= 0; --i)
    {
        MaterialMapping& m = materialMapping[i];
        if (m.entity == entity)
        {
            RemoveMaterialTopParent(m.material);

            m = materialMapping.back();
            materialMapping.pop_back();
        }
    }
}

void EditorMaterialSystem::PrepareForRemove()
{
    for (NMaterial* m : materialTopParents)
        SafeRelease(m);

    materialTopParents.clear();
    materialMapping.clear();
}

void EditorMaterialSystem::RegisterComponent(Entity* entity, Component* component)
{
    AddMaterials(entity, Reflection::Create(ReflectedObject(component)), Any());
}

void EditorMaterialSystem::UnregisterComponent(Entity* entity, Component* component)
{
    RemoveMaterials(Reflection::Create(ReflectedObject(component)), Any());
}

void EditorMaterialSystem::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    auto changeSlotVisitor = [&](const SetFieldValueCommand* cmd)
    {
        const Reflection::Field& field = cmd->GetField();
        ReflectedObject object = field.ref.GetDirectObject();
        Any key = field.key;
        if (object.GetReflectedType() == ReflectedTypeDB::Get<Landscape>()
            && key.Cast<FastName>(FastName("")) == FastName("layersCount"))
        {
            Landscape* landscape = object.GetPtr<Landscape>();
            const Vector<Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
            for (Entity* landEntity : landscapes)
            {
                Landscape* landObject = GetLandscape(landEntity);
                if (landObject == landscape)
                {
                    RemoveMaterials(Reflection::Create(field.ref.GetDirectObject()), Any());
                }
            }
        }
    };
    const RECommandNotificationObject& commandInfo = holder.GetMasterCommandInfo();
    commandInfo.ForEach<SetFieldValueCommand>(changeSlotVisitor);
}

void EditorMaterialSystem::AddMaterials(Entity* entity, const Reflection& ref, const Any& key)
{
    if (ref.GetValueType() == Type::Instance<NMaterial*>() && (key.CanGet<FastName>() || key.CanGet<uint64>()))
    {
        FastName name;
        if (key.CanGet<uint64>())
        {
            uintptr_t p = reinterpret_cast<uintptr_t>(ref.GetDirectObject().GetVoidPtr());
            std::string s = std::to_string(p);
            s += key.Get<uint64>();
            name = FastName(s);
        }
        else
            name = key.Get<FastName>();
        AddMaterial(MaterialOwner(ref.GetDirectObject(), name), ref.GetValue().Get<NMaterial*>(), entity);
    }

    Vector<Reflection::Field> fields = ref.GetFields();
    for (const Reflection::Field& f : fields)
    {
        if (f.ref.GetMeta<M::HiddenField>() == nullptr)
        {
            AddMaterials(entity, f.ref, f.key);
        }
    }
}

void EditorMaterialSystem::RemoveMaterials(const Reflection& ref, const Any& key)
{
    if (ref.GetValueType() == Type::Instance<NMaterial*>() && (key.CanGet<FastName>() || key.CanGet<uint64>()))
    {
        FastName name;
        if (key.CanGet<uint64>())
        {
            uintptr_t p = reinterpret_cast<uintptr_t>(ref.GetDirectObject().GetVoidPtr());
            std::string s = std::to_string(p);
            s += key.Get<uint64>();
            name = FastName(s);
        }
        else
            name = key.Get<FastName>();
        RemoveMaterial(MaterialOwner(ref.GetDirectObject(), name), ref.GetValue().Get<NMaterial*>());
    }

    Vector<Reflection::Field> fields = ref.GetFields();
    for (const Reflection::Field& f : fields)
    {
        if (f.ref.GetMeta<M::HiddenField>() == nullptr)
        {
            RemoveMaterials(f.ref, f.key);
        }
    }
}

void EditorMaterialSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    commandNotification.ForEach<DeleteLODCommand>([&](const DeleteLODCommand* cmd) {
        const Vector<DeleteRenderBatchCommand*> batchCommands = cmd->GetRenderBatchCommands();

        const uint32 count = (const uint32)batchCommands.size();
        for (uint32 i = 0; i < count; ++i)
        {
            RenderBatch* batch = batchCommands[i]->GetRenderBatch();
            MaterialOwner owner(batch);
            if (commandNotification.IsRedo())
            {
                RemoveMaterial(owner, batch->GetMaterial());
            }
            else
            {
                AddMaterial(owner, batch->GetMaterial(), cmd->GetEntity());
            }
        }
    });

    commandNotification.ForEach<CreatePlaneLODCommand>([&](const CreatePlaneLODCommand* cmd) {
        RenderBatch* batch = cmd->GetRenderBatch();

        MaterialOwner owner(batch);
        if (commandNotification.IsRedo())
        {
            AddMaterial(owner, batch->GetMaterial(), cmd->GetEntity());
        }
        else
        {
            RemoveMaterial(owner, batch->GetMaterial());
        }
    });

    commandNotification.ForEach<DeleteRenderBatchCommand>([&](const DeleteRenderBatchCommand* cmd) {
        RenderBatch* batch = cmd->GetRenderBatch();

        MaterialOwner owner(batch);
        if (commandNotification.IsRedo())
        {
            RemoveMaterial(owner, batch->GetMaterial());
        }
        else
        {
            AddMaterial(owner, batch->GetMaterial(), cmd->GetEntity());
        }
    });

    commandNotification.ForEach<ConvertToShadowCommand>([&](const ConvertToShadowCommand* cmd) {
        RenderBatch* oldBatch = cmd->oldBatch;
        RenderBatch* newBatch = cmd->newBatch;

        if (!commandNotification.IsRedo())
        {
            std::swap(oldBatch, newBatch);
        }

        MaterialOwner oldOwner(oldBatch);
        MaterialOwner newOwner(newBatch);

        RemoveMaterial(oldOwner, newBatch->GetMaterial());
        AddMaterial(newOwner, newBatch->GetMaterial(), cmd->GetEntity());
    });

    commandNotification.ForEach<CopyLastLODToLod0Command>([&](const CopyLastLODToLod0Command* cmd) {
        uint32 batchCount = static_cast<uint32>(cmd->newBatches.size());
        for (uint32 i = 0; i < batchCount; ++i)
        {
            RenderBatch* batch = cmd->newBatches[i];
            NMaterial* material = batch->GetMaterial();

            MaterialOwner owner(batch);
            if (commandNotification.IsRedo())
            {
                AddMaterial(owner, material, cmd->GetEntity());
            }
            else
            {
                RemoveMaterial(owner, material);
            }
        }
    });

    commandNotification.ForEach<InspDynamicModifyCommand>([&](const InspDynamicModifyCommand* cmd) {
        //GFX_COMPLETE: review landscape materials
        //Looks like we can just remove this code

        //const Vector<Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
        //for (Entity* landEntity : landscapes)
        //{
        //    Landscape* landObject = GetLandscape(landEntity);
        //    if (landObject == cmd->object)
        //    {
        //        for (uint32 i = 0; i < landObject->GetPageMaterialCount(); ++i)
        //            RemoveMaterial(landObject->GetPageMaterial(i), MaterialMapping(landEntity, landObject));
        //        AddMaterials(landEntity);
        //    }
        //}
    });

    commandNotification.ForEach<InspDynamicModifyCommand>([&](const InspDynamicModifyCommand* cmd) {
        for (Landscape* landscape : GetScene()->landscapeSystem->GetLandscapeObjects())
        {
            for (uint32 i = 0; i < landscape->GetLayersCount(); ++i)
            {
                for (uint32 j = 0; j < landscape->GetPageMaterialCount(i); ++j)
                {
                    if (landscape->GetPageMaterials(i, j) == cmd->ddata.object)
                    {
                        landscape->InvalidateAllPages();
                        break;
                    }
                }
            }
        }
        GetScene()->vtDecalSystem->NotifyMaterialChanged(static_cast<NMaterial*>(cmd->ddata.object));
    });

    commandNotification.ForEach<SetFieldValueCommand>([&](const SetFieldValueCommand* cmd) {
        ReflectedObject object = cmd->GetField().ref.GetDirectObject();
        Any key = cmd->GetField().key;

        if (object.GetReflectedType() == ReflectedTypeDB::Get<Landscape>()
            && key.Cast<FastName>(FastName("")) == FastName("layersCount"))
        {
            Landscape* landscape = object.GetPtr<Landscape>();
            const Vector<Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
            for (Entity* landEntity : landscapes)
            {
                Landscape* landObject = GetLandscape(landEntity);
                if (landObject == landscape)
                {
                    AddMaterials(landEntity, Reflection::Create(object), Any());
                }
            }
        }
    });

    commandNotification.ForEach<MaterialSwitchParentCommand>([&](const MaterialSwitchParentCommand* cmd) {
        NMaterial* material = cmd->GetMaterial();
        DVASSERT(material);

        RemoveMaterialTopParent(material);
        CheckAndAddMaterialParent(material);
    });

    commandNotification.ForEach<MaterialAddCommand>([&](const MaterialAddCommand* cmd) {
        NMaterial* material = cmd->GetMaterial();
        DVASSERT(material != nullptr);

        if (commandNotification.IsRedo() == true)
        {
            CheckAndAddMaterialParent(material);
        }
        else
        {
            RemoveMaterialTopParent(material);
        }
    });

    commandNotification.ForEach<MaterialRemoveCommand>([&](const MaterialRemoveCommand* cmd) {
        NMaterial* material = cmd->GetMaterial();
        DVASSERT(material != nullptr);

        if (commandNotification.IsRedo() == true)
        {
            RemoveMaterialTopParent(material);
        }
        else
        {
            CheckAndAddMaterialParent(material);
        }
    });

    commandNotification.ForEach<MaterialAssignCommand>([&](const MaterialAssignCommand* cmd) {
        Entity* entity = cmd->GetEntity();
        NMaterial* oldMaterial = cmd->GetOldMaterial();
        NMaterial* newMaterial = cmd->GetNewMaterial();

        if (!commandNotification.IsRedo() == true)
        {
            std::swap(oldMaterial, newMaterial);
        }

        MaterialOwner owner(cmd->GetMaterialField());

        DVASSERT(std::find_if(materialMapping.begin(), materialMapping.end(), [&owner](const MaterialMapping& m) {
                     return (owner == m.owner);
                 }) != materialMapping.end(),
                 "Assign to unregistered material field");

        RemoveMaterial(owner, oldMaterial);
        AddMaterial(owner, newMaterial, entity);
    });
}

void EditorMaterialSystem::AddMaterial(const MaterialOwner& owner, NMaterial* material, Entity* entity)
{
    DVASSERT(!owner.IsEmpty());

    if (nullptr != material && material->IsRuntime())
        return;

    auto found = std::find_if(materialMapping.begin(), materialMapping.end(), [&owner](const MaterialMapping& m) {
        return (owner == m.owner);
    });

    if (found == materialMapping.end())
    {
        materialMapping.emplace_back(owner, material, entity);
        CheckAndAddMaterialParent(material);
    }
}

void EditorMaterialSystem::RemoveMaterial(const MaterialOwner& owner, NMaterial* material)
{
    DVASSERT(!owner.IsEmpty());

    Vector<MaterialMapping>::iterator found = std::find_if(materialMapping.begin(), materialMapping.end(), [&owner, &material](const MaterialMapping& m) {
        return (owner == m.owner) && (material == m.material);
    });

    if (found != materialMapping.end())
    {
        *found = materialMapping.back();
        materialMapping.pop_back();
    }
}

void EditorMaterialSystem::CheckAndAddMaterialParent(NMaterial* material)
{
    if (material == nullptr)
        return;

    NMaterial* globalMaterial = GetScene()->GetGlobalMaterial();
    NMaterial* parent = material;
    while (parent->GetParent() != nullptr && parent->GetParent() != globalMaterial)
        parent = parent->GetParent();

    if (materialTopParents.count(parent) == 0)
    {
        materialTopParents.insert(parent);
        SafeRetain(parent);

        ApplyViewMode(parent);
    }
}

void EditorMaterialSystem::RemoveMaterialTopParent(NMaterial* material)
{
    if (materialTopParents.count(material) != 0)
    {
        materialTopParents.erase(material);
        SafeRelease(material);
    }
}

void EditorMaterialSystem::ApplyViewMode()
{
    NMaterialManager::Instance().SetGlobalFlag(NMaterialFlagName::FLAG_VIEWMODE, curViewMode);
}

void EditorMaterialSystem::ApplyViewMode(NMaterial* material)
{
}

} // namespace DAVA
