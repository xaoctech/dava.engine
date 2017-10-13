#include "Classes/SceneTree/Private/SceneTreeItemTraits.h"
#include "Classes/Commands2/ParticleEditorCommands.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/Utils.h>

#include <Debug/DVAssert.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <QBrush>
#include <QColor>

QString BaseSceneTreeTraits::GetTooltip(const Selectable& object) const
{
    return GetName(object);
}

QIcon EntityTraits::GetIcon(const Selectable& object) const
{
    using namespace DAVA::TArc;

    DVASSERT(object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = object.Cast<DAVA::Entity>();

    if (nullptr != entity->GetComponent(DAVA::Component::STATIC_OCCLUSION_COMPONENT))
    {
        return SharedIcon(":/QtIcons/so.png");
    }
    if (nullptr != DAVA::GetEffectComponent(entity))
    {
        return SharedIcon(":/QtIcons/effect.png");
    }
    else if (nullptr != DAVA::GetLandscape(entity))
    {
        return SharedIcon(":/QtIcons/heightmapeditor.png");
    }
    else if (nullptr != GetLodComponent(entity))
    {
        return SharedIcon(":/QtIcons/lod_object.png");
    }
    else if (nullptr != GetSwitchComponent(entity))
    {
        return SharedIcon(":/QtIcons/switch.png");
    }
    else if (nullptr != DAVA::GetVegetation(entity))
    {
        return SharedIcon(":/QtIcons/grass.png");
    }
    else if (nullptr != DAVA::GetSkeletonComponent(entity))
    {
        return SharedIcon(":/QtIcons/skinned_object.png");
    }
    else if (nullptr != DAVA::GetRenderObject(entity))
    {
        return SharedIcon(":/QtIcons/render_object.png");
    }
    else if (nullptr != entity->GetComponent(DAVA::Component::USER_COMPONENT))
    {
        return SharedIcon(":/QtIcons/user_object.png");
    }
    else if (nullptr != DAVA::GetCamera(entity))
    {
        return SharedIcon(":/QtIcons/camera.png");
    }
    else if (nullptr != DAVA::GetLight(entity))
    {
        return SharedIcon(":/QtIcons/light.png");
    }
    else if (nullptr != DAVA::GetWindComponent(entity))
    {
        return SharedIcon(":/QtIcons/wind.png");
    }
    else if (nullptr != DAVA::GetPathComponent(entity))
    {
        return SharedIcon(":/QtIcons/path.png");
    }
    else if (0 != entity->GetComponentCount(DAVA::Component::TEXT_COMPONENT))
    {
        return SharedIcon(":/QtIcons/text_component.png");
    }

    return DAVA::TArc::SharedIcon(":/QtIcons/node.png");
}

QString EntityTraits::GetName(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = object.Cast<DAVA::Entity>();

    DAVA::FastName name = entity->GetName();
    return QString(name.c_str());
}

DAVA::int32 EntityTraits::GetChildrenCount(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = object.Cast<DAVA::Entity>();
    DAVA::int32 entityChildrenCount = entity->GetChildrenCount();
    DAVA::int32 particleEffectsCount = 0;
    DAVA::ParticleEffectComponent* effectComponent = DAVA::GetParticleEffectComponent(entity);
    if (effectComponent != nullptr)
    {
        particleEffectsCount = effectComponent->GetEmittersCount();
    }

    return entityChildrenCount + particleEffectsCount;
}

void EntityTraits::BuildUnfetchedList(const Selectable& object, const DAVA::Function<bool(const Selectable&)>& isFetchedFn, DAVA::Vector<DAVA::int32>& unfetchedIndexes) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = object.Cast<DAVA::Entity>();

    DAVA::int32 childCount = entity->GetChildrenCount();
    for (DAVA::int32 childIndex = 0; childIndex < childCount; ++childIndex)
    {
        DAVA::Entity* childEntity = entity->GetChild(childIndex);
        if (isFetchedFn(Selectable(DAVA::Any(childEntity))) == false)
        {
            unfetchedIndexes.push_back(childIndex);
        }
    }

    DAVA::ParticleEffectComponent* effectComponent = DAVA::GetParticleEffectComponent(entity);
    if (effectComponent != nullptr)
    {
        for (DAVA::uint32 emitterIndex = 0; emitterIndex < effectComponent->GetEmittersCount(); ++emitterIndex)
        {
            DAVA::ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(emitterIndex);
            if (isFetchedFn(Selectable(DAVA::Any(emitterInstance))) == false)
            {
                unfetchedIndexes.push_back(childCount + emitterIndex);
            }
        }
    }
}

void EntityTraits::FetchMore(const Selectable& object, const DAVA::Vector<DAVA::int32>& unfetchedIndexes, const DAVA::Function<void(DAVA::int32, const Selectable&)>& fetchCallback) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = object.Cast<DAVA::Entity>();

    DAVA::int32 entityChildrenCount = entity->GetChildrenCount();
    DAVA::ParticleEffectComponent* effectComponent = DAVA::GetParticleEffectComponent(entity);

    for (DAVA::int32 index : unfetchedIndexes)
    {
        if (index < entity->GetChildrenCount())
        {
            DAVA::Entity* child = entity->GetChild(index);
            fetchCallback(index, Selectable(DAVA::Any(child)));
        }
        else
        {
            DVASSERT(effectComponent != nullptr);
            DAVA::int32 emitterIndex = index - entityChildrenCount;
            DVASSERT(emitterIndex >= 0);
            DAVA::uint32 unsignedEmitterIndex = static_cast<DAVA::uint32>(emitterIndex);
            DVASSERT(unsignedEmitterIndex < effectComponent->GetEmittersCount());
            DAVA::ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(unsignedEmitterIndex);
            fetchCallback(index, Selectable(DAVA::Any(emitterInstance)));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QIcon ParticleEmitterInstanceTraits::GetIcon(const Selectable& object) const
{
    return DAVA::TArc::SharedIcon(":/QtIcons/emitter_particle.png");
}

QString ParticleEmitterInstanceTraits::GetName(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleEmitterInstance>());
    DAVA::ParticleEmitterInstance* emitter = object.Cast<DAVA::ParticleEmitterInstance>();
    return QString(emitter->GetEmitter()->name.c_str());
}

DAVA::int32 ParticleEmitterInstanceTraits::GetChildrenCount(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleEmitterInstance>());
    DAVA::ParticleEmitterInstance* emitter = object.Cast<DAVA::ParticleEmitterInstance>();

    return static_cast<DAVA::int32>(emitter->GetEmitter()->layers.size());
}

void ParticleEmitterInstanceTraits::BuildUnfetchedList(const Selectable& object, const DAVA::Function<bool(const Selectable&)>& isFetchedFn, DAVA::Vector<DAVA::int32>& unfetchedIndexes) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleEmitterInstance>());
    DAVA::ParticleEmitterInstance* emitter = object.Cast<DAVA::ParticleEmitterInstance>();

    DAVA::Vector<DAVA::ParticleLayer*>& layers = emitter->GetEmitter()->layers;
    for (size_t i = 0; i < layers.size(); ++i)
    {
        DAVA::ParticleLayer* layer = layers[i];
        if (isFetchedFn(Selectable(DAVA::Any(layer))) == false)
        {
            unfetchedIndexes.push_back(static_cast<DAVA::int32>(i));
        }
    }
}

void ParticleEmitterInstanceTraits::FetchMore(const Selectable& object, const DAVA::Vector<DAVA::int32>& unfetchedIndexes, const DAVA::Function<void(DAVA::int32, const Selectable&)>& fetchCallback) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleEmitterInstance>());
    DAVA::ParticleEmitterInstance* emitter = object.Cast<DAVA::ParticleEmitterInstance>();

    DAVA::Vector<DAVA::ParticleLayer*>& layers = emitter->GetEmitter()->layers;
    for (DAVA::int32 index : unfetchedIndexes)
    {
        DAVA::ParticleLayer* layer = layers[index];
        fetchCallback(index, Selectable(DAVA::Any(layer)));
    }
}

QVariant ParticleEmitterInstanceTraits::GetValue(const Selectable& object, int itemRole) const
{
    if (itemRole == Qt::BackgroundRole)
    {
        DVASSERT(object.CanBeCastedTo<DAVA::ParticleEmitterInstance>());
        DAVA::ParticleEmitterInstance* emitter = object.Cast<DAVA::ParticleEmitterInstance>();
        if (emitter->GetEmitter()->shortEffect == true)
        {
            return QVariant(QBrush(QColor(255, 0, 0, 20)));
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QIcon ParticleLayerTraits::GetIcon(const Selectable& object) const
{
    return DAVA::TArc::SharedIcon(":/QtIcons/layer_particle.png");
}

QString ParticleLayerTraits::GetName(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
    DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
    return QString(layer->layerName.c_str());
}

DAVA::int32 ParticleLayerTraits::GetChildrenCount(const Selectable& object) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
    DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
    DAVA::int32 forcesCount = static_cast<DAVA::int32>(layer->forces.size());

    if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
    {
        DVASSERT(layer->innerEmitter != nullptr);
        ++forcesCount;
    }

    return forcesCount;
}

void ParticleLayerTraits::BuildUnfetchedList(const Selectable& object, const DAVA::Function<bool(const Selectable&)>& isFetchedFn, DAVA::Vector<DAVA::int32>& unfetchedIndexes) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
    DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
    DAVA::int32 forcesCount = static_cast<DAVA::int32>(layer->forces.size());
    for (DAVA::int32 i = 0; i < forcesCount; ++i)
    {
        DAVA::ParticleForce* force = layer->forces[i];
        if (isFetchedFn(Selectable(DAVA::Any(force))) == false)
        {
            unfetchedIndexes.push_back(i);
        }
    }

    if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
    {
        DVASSERT(layer->innerEmitter != nullptr);
        if (isFetchedFn(Selectable(DAVA::Any(layer->innerEmitter))) == false)
        {
            unfetchedIndexes.push_back(forcesCount);
        }
    }
}

void ParticleLayerTraits::FetchMore(const Selectable& object, const DAVA::Vector<DAVA::int32>& unfetchedIndexes, const DAVA::Function<void(DAVA::int32, const Selectable&)>& fetchCallback) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
    DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
    DAVA::int32 forcesCount = static_cast<DAVA::int32>(layer->forces.size());
    for (DAVA::int32 index : unfetchedIndexes)
    {
        if (index < forcesCount)
        {
            DAVA::ParticleForce* force = layer->forces[index];
            fetchCallback(index, Selectable(DAVA::Any(force)));
        }
        else
        {
            DVASSERT(index == forcesCount);
            DVASSERT(layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES);
            DVASSERT(layer->innerEmitter != nullptr);
            fetchCallback(forcesCount, Selectable(DAVA::Any(layer->innerEmitter)));
        }
    }
}

Qt::ItemFlags ParticleLayerTraits::GetItemFlags(const Selectable& object, Qt::ItemFlags defaultFlags) const
{
    DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
    return defaultFlags | Qt::ItemIsUserCheckable;
}

QVariant ParticleLayerTraits::GetValue(const Selectable& object, int itemRole) const
{
    if (itemRole == Qt::CheckStateRole)
    {
        DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
        DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
        return QVariant(layer->isDisabled == true ? Qt::Unchecked : Qt::Checked);
    }

    return QVariant();
}

bool ParticleLayerTraits::SetValue(const Selectable& object, const QVariant& value, int itemRole, DAVA::Scene* scene) const
{
    if (itemRole == Qt::CheckStateRole)
    {
        DVASSERT(object.CanBeCastedTo<DAVA::ParticleLayer>());
        DAVA::ParticleLayer* layer = object.Cast<DAVA::ParticleLayer>();
        Qt::CheckState state = value.value<Qt::CheckState>();
        DVASSERT(state != Qt::PartiallyChecked);
        bool isChecked = state == Qt::Checked ? true : false;
        static_cast<SceneEditor2*>(scene)->Exec(std::unique_ptr<DAVA::Command>(new CommandUpdateParticleLayerEnabled(layer, value.toBool())));
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QIcon ParticleForceTraits::GetIcon(const Selectable& object) const
{
    return DAVA::TArc::SharedIcon(":/QtIcons/force.png");
}

QString ParticleForceTraits::GetName(const Selectable& object) const
{
    return QStringLiteral("force");
}

DAVA::int32 ParticleForceTraits::GetChildrenCount(const Selectable& object) const
{
    return 0;
}

void ParticleForceTraits::BuildUnfetchedList(const Selectable& object, const DAVA::Function<bool(const Selectable&)>& isFetchedFn, DAVA::Vector<DAVA::int32>& unfetchedIndexes) const
{
}

void ParticleForceTraits::FetchMore(const Selectable& object, const DAVA::Vector<DAVA::int32>& unfetchedIndexes, const DAVA::Function<void(DAVA::int32, const Selectable&)>& fetchCallback) const
{
}

////////////////////////////////////////////////////////////////////////////////////////

SceneTreeTraitsManager::SceneTreeTraitsManager()
{
    AddTraitsNode<DAVA::Entity, EntityTraits>();
    AddTraitsNode<SceneEditor2, EntityTraits>();
    AddTraitsNode<DAVA::ParticleEmitterInstance, ParticleEmitterInstanceTraits>();
    AddTraitsNode<DAVA::ParticleLayer, ParticleLayerTraits>();
    AddTraitsNode<DAVA::ParticleForce, ParticleForceTraits>();
    std::sort(traits.begin(), traits.end(), [](const TraitsNode& left, const TraitsNode& right) {
        return left.type < right.type;
    });
}

QIcon SceneTreeTraitsManager::GetIcon(const Selectable& object) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return QIcon();
    }

    return traits->GetIcon(object);
}

QString SceneTreeTraitsManager::GetName(const Selectable& object) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return QString();
    }

    return traits->GetName(object);
}

QString SceneTreeTraitsManager::GetTooltip(const Selectable& object) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return QString();
    }

    return traits->GetTooltip(object);
}

DAVA::int32 SceneTreeTraitsManager::GetChildrenCount(const Selectable& object) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return false;
    }

    return traits->GetChildrenCount(object);
}

void SceneTreeTraitsManager::BuildUnfetchedList(const Selectable& object, const DAVA::Function<bool(const Selectable&)>& isFetchedFn, DAVA::Vector<DAVA::int32>& unfetchedIndexes) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return;
    }

    return traits->BuildUnfetchedList(object, isFetchedFn, unfetchedIndexes);
}

void SceneTreeTraitsManager::FetchMore(const Selectable& object, const DAVA::Vector<DAVA::int32>& unfetchedIndexes, const DAVA::Function<void(DAVA::int32, const Selectable&)>& fetchCallback) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return;
    }

    return traits->FetchMore(object, unfetchedIndexes, fetchCallback);
}

Qt::ItemFlags SceneTreeTraitsManager::GetItemFlags(const Selectable& object, Qt::ItemFlags defaultFlags) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return defaultFlags;
    }

    return traits->GetItemFlags(object, defaultFlags);
}

QVariant SceneTreeTraitsManager::GetValue(const Selectable& object, int itemRole) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return QVariant();
    }

    return traits->GetValue(object, itemRole);
}

bool SceneTreeTraitsManager::SetValue(const Selectable& object, const QVariant& value, int itemRole, DAVA::Scene* scene) const
{
    const BaseSceneTreeTraits* traits = GetTraits(object);
    if (traits == nullptr)
    {
        return false;
    }

    return traits->SetValue(object, value, itemRole, scene);
}

template <typename TObject, typename TTraits>
void SceneTreeTraitsManager::AddTraitsNode()
{
    TraitsNode node;
    node.type = DAVA::ReflectedTypeDB::Get<TObject>();
    node.traits = DAVA::Any(TTraits());
    traits.push_back(node);
}

const BaseSceneTreeTraits* SceneTreeTraitsManager::GetTraits(const Selectable& object) const
{
    const DAVA::ReflectedType* objType = object.GetObjectType();
    auto iter = std::lower_bound(traits.begin(), traits.end(), objType, [](const TraitsNode& node, const DAVA::ReflectedType* type) {
        return node.type < type;
    });

    if (iter == traits.end())
    {
        DVASSERT(false);
        return nullptr;
    }

    DVASSERT(iter->type == objType);
    return reinterpret_cast<const BaseSceneTreeTraits*>(iter->traits.GetData());
}
