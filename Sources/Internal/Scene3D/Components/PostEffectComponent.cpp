#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/PostEffectComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/PostEffectSystem.h"
#include "Particles/ParticlePropertyLine.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PostEffectComponent)
{
    ReflectionRegistrator<PostEffectComponent>::Begin()
    .ConstructorByPointer()
    .Field("useEv", &PostEffectComponent::GetUseEv, &PostEffectComponent::SetUseEv)[M::DisplayName("Use EV")]
    .Field("baseEV", &PostEffectComponent::GetBaseEV, &PostEffectComponent::SetBaseEV)[M::DisplayName("EV")]
    .Field("dynamicRange", &PostEffectComponent::GetDynamicRange, &PostEffectComponent::SetDynamicRange)[M::DisplayName("Dynamic Range (+/- EV)")]
    .Field("adaptationRange", &PostEffectComponent::GetAdaptationRange, &PostEffectComponent::SetAdaptationRange)[M::DisplayName("Adaptation Range (+/- EV)")]

    .Field("aperture", &PostEffectComponent::GetAperture, &PostEffectComponent::SetAperture)[M::DisplayName("Aperture (f-stops)")]
    .Field("shutterTimeInv", &PostEffectComponent::GetShutterTimeInv, &PostEffectComponent::SetShutterTimeInv)[M::DisplayName("Shutter Time (1/s)")]
    .Field("ISO", &PostEffectComponent::GetISO, &PostEffectComponent::SetISO)[M::DisplayName("ISO")]
    .Field("adaptationSpeed", &PostEffectComponent::GetAdaptationSpeed, &PostEffectComponent::SetAdaptationSpeed)[M::DisplayName("Adaptation Speed")]

    .Field("toneMapping", &PostEffectComponent::GetToneMapping, &PostEffectComponent::SetToneMapping)[M::DisplayName("Tone mapping")]
    .Field("colorGrading", &PostEffectComponent::GetColorGrading, &PostEffectComponent::SetColorGrading)[M::DisplayName("Color Grading")]
    .Field("colorGradingTable", &PostEffectComponent::GetColorGradingTable, &PostEffectComponent::SetColorGradingTable)[M::DisplayName("Color Grading Table")]

    .Field("heatmap", &PostEffectComponent::GetHeapMapEnabled, &PostEffectComponent::SetHeapMapEnabled)[M::DisplayName("Display Heat Map")]
    .Field("heatmapTable", &PostEffectComponent::GetHeatmapTable, &PostEffectComponent::SetHeatmapTable)[M::DisplayName("Heatmap Table")]

    .Field("lightMeterTable", &PostEffectComponent::GetLightMeterTable, &PostEffectComponent::SetLightMeterTable)[M::DisplayName("Exposure Meter Mask")]
    .Field("resetLuminanceHistory", &PostEffectComponent::GetResetHistory, &PostEffectComponent::SetResetHistory)[M::DisplayName("Reset Luminance History")]

    .End();
}

PostEffectComponent::PostEffectComponent()
    : material(new NMaterial())
{
    material->SetRuntime(true);
}

Component* PostEffectComponent::Clone(Entity* toEntity)
{
    PostEffectComponent* component = new PostEffectComponent();
    component->SetEntity(toEntity);

    component->useEv = useEv;
    component->aperture = aperture;
    component->shutterTimeInv = shutterTimeInv;
    component->ISO = ISO;
    component->dynamicRange = dynamicRange;
    component->adaptationRange = adaptationRange;
    component->adaptationSpeed = adaptationSpeed;
    component->baseEV = baseEV;
    component->bloomColor = bloomColor;
    component->bloomEVC = bloomEVC;
    component->enableColorGrading = enableColorGrading;
    component->enableHeatMap = enableHeatMap;
    component->enableToneMapping = enableToneMapping;

    return component;
}

void PostEffectComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    NMaterial* curNode = material.get();
    if (curNode != nullptr)
    {
        dataNodes.insert(curNode);
        curNode = curNode->GetParent();
    }
}

void PostEffectComponent::SetColorGradingTexture(Texture* texture)
{
    static const FastName COLOR_GRADING = FastName("COLOR_GRADING");
    if (material->HasLocalTexture(COLOR_GRADING))
    {
        material->SetTexture(COLOR_GRADING, texture);
    }
    else
    {
        material->AddTexture(COLOR_GRADING, texture);
    }
}

void PostEffectComponent::SetHeatmapTexture(Texture* texture)
{
    static const FastName HEAT_MAP = FastName("HEAT_MAP");
    if (material->HasLocalTexture(HEAT_MAP))
    {
        material->SetTexture(HEAT_MAP, texture);
    }
    else
    {
        material->AddTexture(HEAT_MAP, texture);
    }
}

void PostEffectComponent::SetLightMeterTexture(Texture* texture)
{
    static const FastName LIGHT_METER_TEXTURE = FastName("LIGHT_METER_TEXTURE");
    if (material->HasLocalTexture(LIGHT_METER_TEXTURE))
    {
        material->SetTexture(LIGHT_METER_TEXTURE, texture);
    }
    else
    {
        material->AddTexture(LIGHT_METER_TEXTURE, texture);
    }
}

void PostEffectComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetBool("pe.useEv", useEv);
        archive->SetFloat("pe.aperture", aperture);
        archive->SetFloat("pe.shutterTimeInv", shutterTimeInv);
        archive->SetFloat("pe.ISO", ISO);
        archive->SetVector2("pe.dynamicRange", dynamicRange);
        archive->SetVector2("pe.adaptationRange", adaptationRange);
        archive->SetVector2("pe.adaptationSpeed", adaptationSpeed);
        archive->SetFloat("pe.baseEV", baseEV);
        archive->SetColor("pe.bloomColor", bloomColor);
        archive->SetFloat("pe.bloomEVC", bloomEVC);
        archive->SetString("pe.colorgradingtable", colorGradingTable.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetString("pe.heatmaptable", heatmapTable.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetString("pe.lightmetertable", lightMeterTable.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetBool("pe.toneMapping", enableToneMapping);
        archive->SetBool("pe.colorGrading", enableColorGrading);
        archive->SetBool("pe.heatmap", enableHeatMap);
    }
}

void PostEffectComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive != nullptr)
    {
        useEv = archive->GetBool("pe.useEv", true);
        aperture = archive->GetFloat("pe.aperture");
        shutterTimeInv = archive->GetFloat("pe.shutterTimeInv");
        ISO = archive->GetFloat("pe.ISO");
        dynamicRange = archive->GetVector2("pe.dynamicRange");
        adaptationRange = archive->GetVector2("pe.adaptationRange");
        adaptationSpeed = archive->GetVector2("pe.adaptationSpeed");
        baseEV = archive->GetFloat("pe.baseEV");
        bloomColor = archive->GetColor("pe.bloomColor");
        bloomEVC = archive->GetFloat("pe.bloomEVC");

        String cgTablePath = archive->GetString("pe.colorgradingtable");
        if (!cgTablePath.empty())
        {
            colorGradingTable = serializationContext->GetScenePath() + cgTablePath;
        }

        String lightMeterPath = archive->GetString("pe.lightmetertable");
        if (!lightMeterPath.empty())
        {
            lightMeterTable = serializationContext->GetScenePath() + lightMeterPath;
        }

        String heatmapPath = archive->GetString("pe.heatmaptable");
        if (!heatmapPath.empty())
        {
            heatmapTable = serializationContext->GetScenePath() + heatmapPath;
        }

        enableColorGrading = archive->GetBool("pe.colorGrading", enableColorGrading);
        enableToneMapping = archive->GetBool("pe.toneMapping", enableToneMapping);
        enableHeatMap = archive->GetBool("pe.heatmap", enableHeatMap);
    }
    Component::Deserialize(archive, serializationContext);
}

void PostEffectComponent::SaveToYaml(const FilePath& presetPath, YamlNode* parentNode)
{
    YamlNode* posteffectNode = new YamlNode(YamlNode::TYPE_MAP);
    parentNode->AddNodeToMap("posteffect", posteffectNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(posteffectNode, "useEv", useEv);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::float32>(posteffectNode, "aperture", aperture);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::float32>(posteffectNode, "shutterTimeInv", shutterTimeInv);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::float32>(posteffectNode, "ISO", ISO);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::Vector2>(posteffectNode, "dynamicRange", dynamicRange);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::Vector2>(posteffectNode, "adaptationRange", adaptationRange);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::Vector2>(posteffectNode, "adaptationSpeed", adaptationSpeed);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::float32>(posteffectNode, "baseEV", baseEV);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::Vector4>(posteffectNode, "bloomColor", Vector4(bloomColor.r, bloomColor.g, bloomColor.b, bloomColor.a));
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(posteffectNode, "bloomEVC", bloomEVC);

    if (!colorGradingTable.IsEmpty())
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::String>(posteffectNode, "colorgradingtable", colorGradingTable.GetFilename());

    if (!heatmapTable.IsEmpty())
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::String>(posteffectNode, "heatmaptable", heatmapTable.GetFilename());

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(posteffectNode, "toneMapping", enableToneMapping);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(posteffectNode, "colorGrading", enableColorGrading);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(posteffectNode, "heatMap", enableHeatMap);
}

void PostEffectComponent::LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode)
{
    useEv = true;
    const YamlNode* useEvNode = parentNode->Get("useEv");
    if (useEvNode != nullptr)
        useEv = useEvNode->AsBool();

    const YamlNode* apertureNode = parentNode->Get("aperture");
    if (apertureNode != nullptr)
        aperture = apertureNode->AsFloat();

    const YamlNode* shutterTimeInvNode = parentNode->Get("shutterTimeInv");
    if (shutterTimeInvNode != nullptr)
        shutterTimeInv = shutterTimeInvNode->AsFloat();

    const YamlNode* isoNode = parentNode->Get("ISO");
    if (isoNode != nullptr)
        ISO = isoNode->AsFloat();

    const YamlNode* adaptationRangeNode = parentNode->Get("adaptationRange");
    if (adaptationRangeNode != nullptr)
        adaptationRange = adaptationRangeNode->AsVector2();

    const YamlNode* adaptationSpeedNode = parentNode->Get("adaptationSpeed");
    if (adaptationSpeedNode != nullptr)
        adaptationSpeed = adaptationSpeedNode->AsVector2();

    const YamlNode* baseEVNode = parentNode->Get("baseEV");
    if (baseEVNode != nullptr)
        baseEV = baseEVNode->AsFloat();

    const YamlNode* bloomColorNode = parentNode->Get("bloomColor");
    if (bloomColorNode != nullptr)
        bloomColor = Color(bloomColorNode->AsVector4());

    const YamlNode* bloomEVCNode = parentNode->Get("bloomEVC");
    if (bloomEVCNode != nullptr)
        bloomEVC = bloomEVCNode->AsFloat();

    const YamlNode* enableColorGradingNode = parentNode->Get("colorGrading");
    if (enableColorGradingNode != nullptr)
        enableColorGrading = enableColorGradingNode->AsBool();

    const YamlNode* colorGradingTableNode = parentNode->Get("colorgradingtable");
    if (colorGradingTableNode != nullptr)
        colorGradingTable = presetPath.GetDirectory() + colorGradingTableNode->AsString();

    const YamlNode* enableToneMappingNode = parentNode->Get("toneMapping");
    if (enableToneMappingNode != nullptr)
        enableToneMapping = enableToneMappingNode->AsBool();

    const YamlNode* enableHeatMapnode = parentNode->Get("heatMap");
    if (enableHeatMapnode != nullptr)
        enableHeatMap = enableHeatMapnode->AsBool();

    const YamlNode* heatmapTableNode = parentNode->Get("heatmaptable");
    if (heatmapTableNode != nullptr)
        heatmapTable = presetPath.GetDirectory() + heatmapTableNode->AsString();
}

void PostEffectComponent::SetUseEv(bool value)
{
    useEv = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

bool PostEffectComponent::GetUseEv() const
{
    return useEv;
}

void PostEffectComponent::SetColorGrading(bool value)
{
    enableColorGrading = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

bool PostEffectComponent::GetColorGrading() const
{
    return enableColorGrading;
}

bool PostEffectComponent::GetHeapMapEnabled() const
{
    return enableHeatMap;
}

void PostEffectComponent::SetHeapMapEnabled(bool value)
{
    enableHeatMap = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

void PostEffectComponent::SetToneMapping(bool value)
{
    enableToneMapping = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

bool PostEffectComponent::GetToneMapping() const
{
    return enableToneMapping;
}

void PostEffectComponent::SetAperture(const float32& value)
{
    aperture = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

float32 PostEffectComponent::GetAperture() const
{
    return aperture;
}

void PostEffectComponent::SetShutterTimeInv(const float32& value)
{
    shutterTimeInv = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

float32 PostEffectComponent::GetShutterTimeInv() const
{
    return shutterTimeInv;
}

void PostEffectComponent::SetISO(const float32& value)
{
    ISO = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

float32 PostEffectComponent::GetISO() const
{
    return ISO;
}

void PostEffectComponent::SetDynamicRange(const Vector2& value)
{
    dynamicRange = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

Vector2 PostEffectComponent::GetDynamicRange() const
{
    return dynamicRange;
}

void PostEffectComponent::SetAdaptationRange(const Vector2& value)
{
    adaptationRange = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

Vector2 PostEffectComponent::GetAdaptationRange() const
{
    return adaptationRange;
}

void PostEffectComponent::SetAdaptationSpeed(const Vector2& value)
{
    adaptationSpeed = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

Vector2 PostEffectComponent::GetAdaptationSpeed() const
{
    return adaptationSpeed;
}

void PostEffectComponent::SetBaseEV(const float32& value)
{
    baseEV = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

float32 PostEffectComponent::GetBaseEV() const
{
    return baseEV;
}

void PostEffectComponent::SetBloomColor(const Color& value)
{
    bloomColor = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

Color PostEffectComponent::GetBloomColor() const
{
    return bloomColor;
}

void PostEffectComponent::SetBloomEVC(const float32& value)
{
    bloomEVC = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

float32 PostEffectComponent::GetBloomEVC() const
{
    return bloomEVC;
}

void PostEffectComponent::SetColorGradingTable(const FilePath& p)
{
    colorGradingTable = p;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

const FilePath& PostEffectComponent::GetColorGradingTable() const
{
    return colorGradingTable;
}

void PostEffectComponent::SetHeatmapTable(const FilePath& value)
{
    heatmapTable = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

const FilePath& PostEffectComponent::GetHeatmapTable() const
{
    return heatmapTable;
}

void PostEffectComponent::SetLightMeterTable(const FilePath& value)
{
    lightMeterTable = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
}

const FilePath& PostEffectComponent::GetLightMeterTable() const
{
    return lightMeterTable;
}

void PostEffectComponent::SetResetHistory(bool reset)
{
    resetHistory = reset;
    if (resetHistory)
    {
        GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_CHANGED);
    }
}

bool PostEffectComponent::GetResetHistory() const
{
    return resetHistory;
}
};
