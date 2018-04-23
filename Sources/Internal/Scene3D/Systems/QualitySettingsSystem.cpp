#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Scene.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Render/Highlevel/RenderObject.h"
#include "Logger/Logger.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
const FastName QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION("Vegetation Animation");
const FastName QualitySettingsSystem::QUALITY_OPTION_STENCIL_SHADOW("Stencil Shadows");
const FastName QualitySettingsSystem::QUALITY_OPTION_WATER_DECORATIONS("Water Decorations");
const FastName QualitySettingsSystem::QUALITY_OPTION_DISABLE_EFFECTS("Disable effects");
const FastName QualitySettingsSystem::QUALITY_OPTION_LOD0_EFFECTS("Lod0 effects");

const FastName QualitySettingsSystem::QUALITY_OPTION_DISABLE_FOG("Disable fog");
const FastName QualitySettingsSystem::QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_ATTENUATION("Disable fog attenuation");
const FastName QualitySettingsSystem::QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_SCATTERING("Disable fog scattering");
const FastName QualitySettingsSystem::QUALITY_OPTION_DISABLE_FOG_HALF_SPACE("Disable half-space fog");
const FastName QualitySettingsSystem::QUALITY_OPTION_DEFERRED_DRAW_FORWARD("Deferred Rendering Includes Forward Objects");
const FastName QualitySettingsSystem::QUALITY_OPTION_HALF_RESOLUTION_3D("Half Resolution Rendering");

QualitySettingsSystem::QualitySettingsSystem()
{
    const Map<RenderFlow, FastName> renderFlowNames = {
        { RenderFlow::HDRDeferred, FastName("HDR Deferred") },
        { RenderFlow::HDRForward, FastName("HDR Forward") },
        { RenderFlow::LDRForward, FastName("LDR Forward") },
        { RenderFlow::TileBasedHDRDeferred, FastName("TB HDR Deferred") },
        { RenderFlow::TileBasedHDRForward, FastName("TB HDR Forward") },
    };

    qualityGroups[QualityGroup::RenderFlowType].groupName = FastName("Render Flow");
    for (uint32 i = uint32(RenderFlow::FirstValid); i < uint32(RenderFlow::Count); ++i)
    {
        RenderFlow flow = RenderFlow(i);
        qualityGroups[QualityGroup::RenderFlowType].values.emplace_back(renderFlowNames.at(flow), flow);
        if (Renderer::GetCurrentRenderFlow() == flow)
        {
            qualityGroups[QualityGroup::RenderFlowType].currentValue = renderFlowNames.at(flow);
        }
    }

    static_assert(MAX_SHADOW_CASCADES == 4, "Max shadow cascades should be 4");

    qualityGroups[QualityGroup::Shadow].groupName = FastName("Shadows");
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("Off"), ShadowQuality(0, 0));
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("Poor"), ShadowQuality(1, 0));
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("Low"), ShadowQuality(1, 4));
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("Medium"), ShadowQuality(2, 4));
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("High"), ShadowQuality(3, 8));
    qualityGroups[QualityGroup::Shadow].values.emplace_back(FastName("Ultra"), ShadowQuality(4, 8));
    qualityGroups[QualityGroup::Shadow].currentValue = qualityGroups[QualityGroup::Shadow].values.rbegin()->first;

    qualityGroups[QualityGroup::Anisotropy].groupName = FastName("Anisotropy");
    qualityGroups[QualityGroup::Anisotropy].values.emplace_back(FastName("Disabled"), 1u);
    qualityGroups[QualityGroup::Anisotropy].values.emplace_back(FastName("2x"), 2u);
    qualityGroups[QualityGroup::Anisotropy].values.emplace_back(FastName("4x"), 8u);
    qualityGroups[QualityGroup::Anisotropy].values.emplace_back(FastName("8x"), 8u);
    qualityGroups[QualityGroup::Anisotropy].values.emplace_back(FastName("16x"), 16u);
    qualityGroups[QualityGroup::Anisotropy].currentValue = qualityGroups[QualityGroup::Anisotropy].values.rbegin()->first;

    qualityGroups[QualityGroup::Antialiasing].groupName = FastName("Antialiasing");
    qualityGroups[QualityGroup::Antialiasing].values.emplace_back(FastName("Disabled"), rhi::AntialiasingType::NONE);
    qualityGroups[QualityGroup::Antialiasing].values.emplace_back(FastName("MSAA 2X"), rhi::AntialiasingType::MSAA_2X);
    qualityGroups[QualityGroup::Antialiasing].values.emplace_back(FastName("MSAA 4X"), rhi::AntialiasingType::MSAA_4X);
    qualityGroups[QualityGroup::Antialiasing].values.emplace_back(FastName("TAA"), rhi::AntialiasingType::TEMPORAL_REPROJECTION);
    qualityGroups[QualityGroup::Antialiasing].currentValue = qualityGroups[QualityGroup::Antialiasing].values.begin()->first;

    qualityGroups[QualityGroup::Textures].groupName = FastName("Textures");
    qualityGroups[QualityGroup::Textures].values.emplace_back(FastName("Default"), TexturesQuality(0));
    qualityGroups[QualityGroup::Textures].values.emplace_back(FastName("Medium"), TexturesQuality(1));
    qualityGroups[QualityGroup::Textures].values.emplace_back(FastName("Low"), TexturesQuality(2));
    qualityGroups[QualityGroup::Textures].values.emplace_back(FastName("Poor"), TexturesQuality(3));
    qualityGroups[QualityGroup::Textures].currentValue = qualityGroups[QualityGroup::Textures].values.begin()->first;

    qualityGroups[QualityGroup::Scattering].groupName = FastName("Atmospheric Scattering");
    qualityGroups[QualityGroup::Scattering].values.emplace_back(FastName("Off"), ScatteringQuality(0));
    qualityGroups[QualityGroup::Scattering].values.emplace_back(FastName("Low"), ScatteringQuality(8));
    qualityGroups[QualityGroup::Scattering].values.emplace_back(FastName("High"), ScatteringQuality(16));
    qualityGroups[QualityGroup::Scattering].values.emplace_back(FastName("Ultra"), ScatteringQuality(32));
    qualityGroups[QualityGroup::Scattering].currentValue = FastName("High");

    qualityGroups[QualityGroup::Terrain].groupName = FastName("Landscape");
    qualityGroups[QualityGroup::Terrain].values.emplace_back(FastName("Low"), LandscapeQuality::Low);
    qualityGroups[QualityGroup::Terrain].values.emplace_back(FastName("Medium"), LandscapeQuality::Medium);
    qualityGroups[QualityGroup::Terrain].values.emplace_back(FastName("Full"), LandscapeQuality::Full);
    qualityGroups[QualityGroup::Terrain].currentValue = FastName("Full");

    Load("~res:/quality.yaml");

    EnableOption(QUALITY_OPTION_DEFERRED_DRAW_FORWARD, true);
    EnableOption(QUALITY_OPTION_HALF_RESOLUTION_3D, false);
}

void QualitySettingsSystem::Load(const FilePath& path)
{
    if (GetEngineContext()->fileSystem->Exists(path))
    {
        ScopedPtr<YamlParser> parser(YamlParser::Create(path));
        YamlNode* rootNode = parser->GetRootNode();

        if (NULL != rootNode)
        {
            materialGroups.clear();
            soundQualities.clear();
            landscapeQualities.clear();

            // materials
            const YamlNode* materialGroupsNode = rootNode->Get("materials");
            if (NULL != materialGroupsNode)
            {
                for (uint32 i = 0; i < materialGroupsNode->GetCount(); ++i)
                {
                    const YamlNode* groupNode = materialGroupsNode->Get(i);
                    const YamlNode* name = groupNode->Get("group");
                    const YamlNode* values = groupNode->Get("quality");
                    const YamlNode* deflt = groupNode->Get("default");

                    FastName defQualityName;
                    if (NULL != deflt && deflt->GetType() == YamlNode::TYPE_STRING)
                    {
                        defQualityName = FastName(deflt->AsString().c_str());
                    }

                    if (NULL != name && NULL != values &&
                        name->GetType() == YamlNode::TYPE_STRING &&
                        values->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        const Vector<YamlNode*>& v = values->AsVector();

                        MAGrQ maGr;
                        maGr.curQuality = 0;
                        maGr.qualities.reserve(v.size());

                        for (size_t j = 0; j < v.size(); ++j)
                        {
                            if (v[j]->GetType() == YamlNode::TYPE_STRING)
                            {
                                MaterialQuality mq;
                                mq.qualityName = FastName(v[j]->AsString().c_str());

                                maGr.qualities.push_back(mq);

                                if (mq.qualityName == defQualityName)
                                {
                                    maGr.curQuality = uint32(j);
                                }
                            }
                        }

                        String nodeName = name->AsString();
                        FastName materialKey = FastName(nodeName);
                        materialGroups[materialKey] = std::move(maGr);
                    }
                }
            }

            // sound
            const YamlNode* soundsNode = rootNode->Get("sounds");
            if (NULL != soundsNode)
            {
                const YamlNode* defltSfx = soundsNode->Get("default");
                const YamlNode* qualitiesNode = soundsNode->Get("qualities");

                if (NULL != qualitiesNode)
                {
                    FastName defSfxQualityName;
                    if (NULL != defltSfx && defltSfx->GetType() == YamlNode::TYPE_STRING)
                    {
                        defSfxQualityName = FastName(defltSfx->AsString().c_str());
                    }

                    soundQualities.reserve(qualitiesNode->GetCount());
                    for (uint32 i = 0; i < qualitiesNode->GetCount(); ++i)
                    {
                        const YamlNode* qualityNode = qualitiesNode->Get(i);
                        const YamlNode* name = qualityNode->Get("quality");
                        const YamlNode* confgNode = qualityNode->Get("configPath");

                        if (NULL != name && name->GetType() == YamlNode::TYPE_STRING &&
                            NULL != confgNode && confgNode->GetType() == YamlNode::TYPE_STRING)
                        {
                            SFXQ sfxq;

                            sfxq.name = FastName(name->AsString().c_str());
                            sfxq.configPath = FilePath(confgNode->AsString());

                            soundQualities.push_back(sfxq);

                            if (sfxq.name == defSfxQualityName)
                            {
                                curSoundQuality = i;
                            }
                        }
                    }
                }
            }

            // particles
            const YamlNode* particlesNode = rootNode->Get("particles");
            if (nullptr != particlesNode)
            {
                particlesQualitySettings.LoadFromYaml(particlesNode);
            }
        }
    }
}

size_t QualitySettingsSystem::GetSFXQualityCount() const
{
    return soundQualities.size();
}

FastName QualitySettingsSystem::GetSFXQualityName(size_t index) const
{
    FastName ret;

    if (index < soundQualities.size())
    {
        ret = soundQualities[index].name;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurSFXQuality() const
{
    return GetSFXQualityName(curSoundQuality);
}

void QualitySettingsSystem::SetCurSFXQuality(const FastName& name)
{
    for (size_t i = 0; i < soundQualities.size(); ++i)
    {
        if (soundQualities[i].name == name)
        {
            curSoundQuality = static_cast<int32>(i);
            return;
        }
    }
}

FilePath QualitySettingsSystem::GetSFXQualityConfigPath(const FastName& name) const
{
    FilePath ret;

    for (size_t i = 0; i < soundQualities.size(); ++i)
    {
        if (soundQualities[i].name == name)
        {
            ret = soundQualities[i].configPath;
            break;
        }
    }

    return ret;
}

FilePath QualitySettingsSystem::GetSFXQualityConfigPath(size_t index) const
{
    FilePath ret;

    if (index < soundQualities.size())
    {
        ret = soundQualities[index].configPath;
    }

    return ret;
}

bool QualitySettingsSystem::GetAllowCutUnusedVertexStreams()
{
    return cutUnusedVertexStreams;
}

void QualitySettingsSystem::SetAllowCutUnusedVertexStreams(bool cut)
{
    cutUnusedVertexStreams = cut;
}

size_t QualitySettingsSystem::GetMaterialQualityGroupCount() const
{
    return materialGroups.size();
}

FastName QualitySettingsSystem::GetMaterialQualityGroupName(size_t index) const
{
    if (index < materialGroups.size())
    {
        return std::next(materialGroups.begin(), index)->first;
    }

    return FastName();
}

size_t QualitySettingsSystem::GetMaterialQualityCount(const FastName& group) const
{
    size_t ret = 0;

    auto it = materialGroups.find(group);

    if (it != materialGroups.end())
    {
        ret = it->second.qualities.size();
    }

    return ret;
}

FastName QualitySettingsSystem::GetMaterialQualityName(const FastName& group, size_t index) const
{
    FastName ret;

    auto it = materialGroups.find(group);

    if (it != materialGroups.end())
    {
        auto& materialGroup = it->second;

        if (index < materialGroup.qualities.size())
        {
            ret = materialGroup.qualities[index].qualityName;
        }
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurMaterialQuality(const FastName& group) const
{
    FastName ret;

    auto it = materialGroups.find(group);

    if (it != materialGroups.end())
    {
        ret = GetMaterialQualityName(group, it->second.curQuality);
    }

    return ret;
}

void QualitySettingsSystem::SetCurMaterialQuality(const FastName& group, const FastName& quality)
{
    auto it = materialGroups.find(group);

    if (it != materialGroups.end())
    {
        auto& materialGroup = it->second;
        for (size_t i = 0; i < materialGroup.qualities.size(); ++i)
        {
            if (materialGroup.qualities[i].qualityName == quality)
            {
                materialGroup.curQuality = uint32(i);
                return;
            }
        }
    }

    DVASSERT(0 && "Not such quality");
}

const MaterialQuality* QualitySettingsSystem::GetMaterialQuality(const FastName& group, const FastName& quality) const
{
    const MaterialQuality* ret = NULL;

    auto it = materialGroups.find(group);

    if (it != materialGroups.end())
    {
        auto& materialGroup = it->second;
        for (size_t i = 0; i < materialGroup.qualities.size(); ++i)
        {
            if (materialGroup.qualities[i].qualityName == quality)
            {
                ret = &materialGroup.qualities[i];
                break;
            }
        }
    }

    //DVASSERT(NULL != ret && "No such quality");

    return ret;
}

const ParticlesQualitySettings& QualitySettingsSystem::GetParticlesQualitySettings() const
{
    return particlesQualitySettings;
}

ParticlesQualitySettings& QualitySettingsSystem::GetParticlesQualitySettings()
{
    return particlesQualitySettings;
}

void QualitySettingsSystem::EnableOption(const FastName& option, bool enabled)
{
    qualityOptions[option] = enabled;
}

bool QualitySettingsSystem::IsOptionEnabled(const FastName& option) const
{
    auto it = qualityOptions.find(option);

    if (it != qualityOptions.end())
    {
        return it->second;
    }

    return false;
}

int32 QualitySettingsSystem::GetOptionsCount() const
{
    return static_cast<int32>(qualityOptions.size());
}

FastName QualitySettingsSystem::GetOptionName(int32 index) const
{
    DVASSERT(index >= 0);

    if (static_cast<size_t>(index) < qualityOptions.size())
    {
        return std::next(qualityOptions.begin(), index)->first;
    }

    return FastName();
}

void QualitySettingsSystem::UpdateEntityAfterLoad(Entity* entity)
{
    if (qualityOptions.empty() || (NULL == entity))
        return;

    Vector<Entity*> entitiesWithQualityComponent;
    entity->GetChildEntitiesWithComponent(entitiesWithQualityComponent, Type::Instance<QualitySettingsComponent>());

    for (size_t i = 0, sz = entitiesWithQualityComponent.size(); i < sz; ++i)
    {
        if (!IsQualityVisible(entitiesWithQualityComponent[i]))
        {
            if (keepUnusedQualityEntities)
            {
                UpdateEntityVisibilityRecursively(entitiesWithQualityComponent[i], false);
            }
            else
            {
                Entity* parent = entitiesWithQualityComponent[i]->GetParent();
                parent->RemoveNode(entitiesWithQualityComponent[i]);
            }
        }
    }
}

bool QualitySettingsSystem::IsQualityVisible(const Entity* entity)
{
    QualitySettingsComponent* comp = GetQualitySettingsComponent(entity);
    if (comp)
    {
        if (comp->filterByType)
            return (!comp->modelType.IsValid()) || IsOptionEnabled(comp->GetModelType());
        else
            return (GetCurMaterialQuality(comp->requiredGroup) == comp->requiredQuality);
    }

    return true;
}

void QualitySettingsSystem::UpdateEntityVisibility(Entity* e)
{
    QualitySettingsComponent* comp = GetQualitySettingsComponent(e);
    if (comp)
        UpdateEntityVisibilityRecursively(e, IsQualityVisible(e));
}

void QualitySettingsSystem::UpdateEntityVisibilityRecursively(Entity* e, bool qualityVisible)
{
    RenderObject* ro = GetRenderObject(e);
    if (ro)
    {
        if (qualityVisible)
            ro->AddFlag(RenderObject::VISIBLE_QUALITY);
        else
            ro->RemoveFlag(RenderObject::VISIBLE_QUALITY);
    }

    for (int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
        UpdateEntityVisibilityRecursively(e->GetChild(i), qualityVisible);
}

/*
 * New stuff
 */
const FastName& QualitySettingsSystem::GetQualityGroupName(QualityGroup group) const
{
    DVASSERT(group < QualityGroup::Count);
    return qualityGroups[group].groupName;
}

const FastName& QualitySettingsSystem::GetCurrentQualityForGroup(QualityGroup group) const
{
    DVASSERT(group < QualityGroup::Count);
    return qualityGroups[group].currentValue;
}

Vector<FastName> QualitySettingsSystem::GetAvailableQualitiesForGroup(QualityGroup group) const
{
    DVASSERT(group < QualityGroup::Count);
    Vector<FastName> result;
    for (const auto& i : qualityGroups[group].values)
        result.emplace_back(i.first);
    return result;
}

void QualitySettingsSystem::SetCurrentQualityForGroup(QualityGroup group, const FastName& value)
{
    DVASSERT(group < QualityGroup::Count);
    DVASSERT(HasQualityInGroup(group, value)); // make sure such value actually exist
    qualityGroups[group].currentValue = value;
}

bool QualitySettingsSystem::HasQualityInGroup(QualityGroup group, const FastName& value) const
{
    DVASSERT(group < QualityGroup::Count);

    return std::find_if(qualityGroups[group].values.begin(), qualityGroups[group].values.end(), [&value](const auto& p) {
               return p.first == value;
           }) != qualityGroups[group].values.end();
}
}
