#include "Render/Highlevel/Light.h"
#include "Render/RenderHelper.h"
#include "Particles/ParticlePropertyLine.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Utils/StringFormat.h"

#include "Engine/Engine.h"
#include "Base/GlobalEnum.h"

ENUM_DECLARE(DAVA::Light::eFlags)
{
    ENUM_ADD_DESCR(DAVA::Light::DYNAMIC_LIGHT, "Dynamic");
    ENUM_ADD_DESCR(DAVA::Light::CASTS_SHADOW, "Casts Shadow");
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Light)
{
    ReflectionRegistrator<Light>::Begin()
    .End();
}

Light::Light()
{
    bbox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 1.0f); // GFX_COMPLETE - temporary
    RenderObject::type = TYPE_LIGHT;
}

void Light::SetAmbientColor(const Color& _color)
{
    ambientColorDeprecated = _color;
}

void Light::SetColor(const Color& _color)
{
    baseColor = _color;
    lastSetColorTemperature = 0.0f;
}

void Light::SetIntensity(float32 _intensity)
{
    intensityDeprecated = _intensity;
}

RenderObject* Light::Clone(RenderObject* dstNode)
{
    if (dstNode == nullptr)
    {
        DVASSERT(IsPointerToExactClass<Light>(this), "Can clone only LightNode");
        dstNode = new Light();
    }

    Light* lightNode = static_cast<Light*>(dstNode);

    lightNode->type = type;
    lightNode->baseColor = baseColor;
    lightNode->ambientColorDeprecated = ambientColorDeprecated;
    lightNode->intensityDeprecated = intensityDeprecated;
    lightNode->radius = radius;
    lightNode->flags = flags;

    return dstNode;
}

void Light::SetPositionDirectionFromMatrix(const Matrix4& worldTransform)
{
    position = Vector3(0.0f, 0.0f, 0.0f) * worldTransform;
    direction = MultiplyVectorMat3x3(Vector3(0.0, -1.0f, 0.0f), worldTransform);
    direction.Normalize();
}

void Light::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::SaveObject(archive);

    archive->SetInt32("type", type);
    archive->SetFloat("ambColor.r", ambientColorDeprecated.r);
    archive->SetFloat("ambColor.g", ambientColorDeprecated.g);
    archive->SetFloat("ambColor.b", ambientColorDeprecated.b);
    archive->SetFloat("ambColor.a", ambientColorDeprecated.a);
    archive->SetFloat("color.r", baseColor.r);
    archive->SetFloat("color.g", baseColor.g);
    archive->SetFloat("color.b", baseColor.b);
    archive->SetFloat("color.a", baseColor.a);
    archive->SetFloat("intensity", intensityDeprecated);
    archive->SetFloat("radius", radius);
    archive->SetFloat("ao.radius", aoRadius);
    archive->SetUInt32("flags", flags);
    archive->SetBool("autoColor", autoColor);

    archive->SetFloat("shadow.cascades.0", shadowCascadesIntervals.x);
    archive->SetFloat("shadow.cascades.1", shadowCascadesIntervals.y);
    archive->SetFloat("shadow.cascades.2", shadowCascadesIntervals.z);
    archive->SetFloat("shadow.cascades.3", shadowCascadesIntervals.w);
    archive->SetFloat("shadow.write.bias", shadowWriteBias.x);
    archive->SetFloat("shadow.write.bias_scale", shadowWriteBias.y);
    archive->SetFloat("shadow.filter.radius", shadowFilterRadius.x);
    archive->SetFloat("shadow.filter.elongation", shadowFilterRadius.y);

    archive->SetString("env.map", environmentMap.GetRelativePathname(serializationContext->GetScenePath()));
}

void Light::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::LoadObject(archive);

    type = eType(archive->GetInt32("type"));

    ambientColorDeprecated.r = archive->GetFloat("ambColor.r", ambientColorDeprecated.r);
    ambientColorDeprecated.g = archive->GetFloat("ambColor.g", ambientColorDeprecated.g);
    ambientColorDeprecated.b = archive->GetFloat("ambColor.b", ambientColorDeprecated.b);
    ambientColorDeprecated.a = archive->GetFloat("ambColor.a", ambientColorDeprecated.a);

    baseColor.r = archive->GetFloat("color.r", baseColor.r);
    baseColor.g = archive->GetFloat("color.g", baseColor.g);
    baseColor.b = archive->GetFloat("color.b", baseColor.b);
    baseColor.a = archive->GetFloat("color.a", baseColor.a);
    autoColor = archive->GetBool("autoColor", autoColor);

    intensityDeprecated = archive->GetFloat("intensity", intensityDeprecated);

    flags = archive->GetUInt32("flags", flags);

    String envMap = archive->GetString("env.map");
    if (!envMap.empty())
    {
        environmentMap = serializationContext->GetScenePath() + envMap;
    }

    shadowCascadesIntervals.x = archive->GetFloat("shadow.cascades.0", shadowCascadesIntervals.x);
    shadowCascadesIntervals.y = archive->GetFloat("shadow.cascades.1", shadowCascadesIntervals.y);
    shadowCascadesIntervals.z = archive->GetFloat("shadow.cascades.2", shadowCascadesIntervals.z);
    shadowCascadesIntervals.w = archive->GetFloat("shadow.cascades.3", shadowCascadesIntervals.w);
    shadowFilterRadius.x = archive->GetFloat("shadow.filter.radius", shadowFilterRadius.x);
    shadowFilterRadius.y = archive->GetFloat("shadow.filter.elongation", shadowFilterRadius.y);
    shadowWriteBias.x = archive->GetFloat("shadow.write.bias", shadowWriteBias.x);
    shadowWriteBias.y = archive->GetFloat("shadow.write.bias_scale", shadowWriteBias.y);
    radius = archive->GetFloat("radius", 10.0f);
    aoRadius = archive->GetFloat("ao.radius", aoRadius);
}

void Light::SaveToYaml(const FilePath& presetPath, YamlNode* parentNode, const FilePath& realEnvMapPath)
{
    YamlNode* lightNode = new YamlNode(YamlNode::TYPE_MAP);
    parentNode->AddNodeToMap(Format("light%d", type), lightNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(lightNode, "type", static_cast<int32>(type));
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.r", ambientColorDeprecated.r);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.g", ambientColorDeprecated.g);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.b", ambientColorDeprecated.b);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.a", ambientColorDeprecated.a);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.r", baseColor.r);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.g", baseColor.g);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.b", baseColor.b);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.a", baseColor.a);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "intensity", intensityDeprecated);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "radius", radius);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ao.radius", aoRadius);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(lightNode, "flags", flags);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades.0", shadowCascadesIntervals.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades.1", shadowCascadesIntervals.y);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades.2", shadowCascadesIntervals.z);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades.3", shadowCascadesIntervals.w);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.filter.radius", shadowFilterRadius.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.filter.elongation", shadowFilterRadius.y);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.write.bias", shadowWriteBias.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.write.bias_scale", shadowWriteBias.y);

    if (!realEnvMapPath.IsEmpty())
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::String>(lightNode, "environmentMap", realEnvMapPath.GetFilename());
}

void Light::LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode)
{
    type = eType::TYPE_DIRECTIONAL;
    const YamlNode* typeNode = parentNode->Get("type");
    if (typeNode != nullptr)
        type = static_cast<eType>(typeNode->AsInt32());

    const YamlNode* abmColorDeprNodeR = parentNode->Get("ambColor.r");
    if (abmColorDeprNodeR != nullptr)
        ambientColorDeprecated.r = abmColorDeprNodeR->AsFloat();

    const YamlNode* abmColorDeprNodeG = parentNode->Get("ambColor.g");
    if (abmColorDeprNodeG != nullptr)
        ambientColorDeprecated.g = abmColorDeprNodeG->AsFloat();

    const YamlNode* abmColorDeprNodeB = parentNode->Get("ambColor.b");
    if (abmColorDeprNodeB != nullptr)
        ambientColorDeprecated.b = abmColorDeprNodeB->AsFloat();

    const YamlNode* abmColorDeprNodeA = parentNode->Get("ambColor.a");
    if (abmColorDeprNodeA != nullptr)
        ambientColorDeprecated.a = abmColorDeprNodeA->AsFloat();

    const YamlNode* baseColorNodeR = parentNode->Get("color.r");
    if (baseColorNodeR != nullptr)
        baseColor.r = baseColorNodeR->AsFloat();

    const YamlNode* baseColorNodeG = parentNode->Get("color.g");
    if (baseColorNodeG != nullptr)
        baseColor.g = baseColorNodeG->AsFloat();

    const YamlNode* baseColorNodeB = parentNode->Get("color.b");
    if (baseColorNodeB != nullptr)
        baseColor.b = baseColorNodeB->AsFloat();

    const YamlNode* baseColorNodeA = parentNode->Get("color.a");
    if (baseColorNodeA != nullptr)
        baseColor.a = baseColorNodeA->AsFloat();

    const YamlNode* intensityDeprecatedNode = parentNode->Get("intensity");
    if (intensityDeprecatedNode != nullptr)
        intensityDeprecated = intensityDeprecatedNode->AsFloat();

    const YamlNode* flagsNode = parentNode->Get("intensity");
    if (flagsNode != nullptr)
        flags = static_cast<uint32>(flagsNode->AsInt32());

    const YamlNode* shadowCascadesIntervalsNodeX = parentNode->Get("shadow.cascades.0");
    if (shadowCascadesIntervalsNodeX != nullptr)
        shadowCascadesIntervals.x = shadowCascadesIntervalsNodeX->AsFloat();

    const YamlNode* shadowCascadesIntervalsNodeY = parentNode->Get("shadow.cascades.1");
    if (shadowCascadesIntervalsNodeY != nullptr)
        shadowCascadesIntervals.y = shadowCascadesIntervalsNodeY->AsFloat();

    const YamlNode* shadowCascadesIntervalsNodeZ = parentNode->Get("shadow.cascades.2");
    if (shadowCascadesIntervalsNodeZ != nullptr)
        shadowCascadesIntervals.z = shadowCascadesIntervalsNodeZ->AsFloat();

    const YamlNode* shadowCascadesIntervalsNodeW = parentNode->Get("shadow.cascades.3");
    if (shadowCascadesIntervalsNodeW != nullptr)
        shadowCascadesIntervals.w = shadowCascadesIntervalsNodeW->AsFloat();

    const YamlNode* shadowFilterRadiusNode = parentNode->Get("shadow.filter.radius");
    if (shadowFilterRadiusNode != nullptr)
        shadowFilterRadius.x = shadowFilterRadiusNode->AsFloat();

    const YamlNode* shadowFilterElongationNode = parentNode->Get("shadow.filter.elongation");
    if (shadowFilterElongationNode != nullptr)
        shadowFilterRadius.y = shadowFilterElongationNode->AsFloat();

    const YamlNode* shadowWriteBiasNode = parentNode->Get("shadow.write.bias");
    if (shadowWriteBiasNode != nullptr)
        shadowWriteBias.x = shadowWriteBiasNode->AsFloat();

    const YamlNode* shadowWriteBiasScaleNode = parentNode->Get("shadow.write.bias_scale");
    if (shadowWriteBiasScaleNode != nullptr)
        shadowWriteBias.y = shadowWriteBiasScaleNode->AsFloat();

    const YamlNode* radiusNode = parentNode->Get("radius");
    if (radiusNode != nullptr)
        radius = radiusNode->AsFloat();

    const YamlNode* aoRadiusNode = parentNode->Get("ao.radius");
    if (aoRadiusNode != nullptr)
        aoRadius = aoRadiusNode->AsFloat();

    const YamlNode* envMapNode = parentNode->Get("environmentMap");
    if (envMapNode != nullptr)
        environmentMap = presetPath.GetDirectory() + envMapNode->AsString();
}

void Light::SetIsDynamic(const bool& isDynamic)
{
    if (isDynamic)
    {
        AddFlag(DYNAMIC_LIGHT);
    }
    else
    {
        RemoveFlag(DYNAMIC_LIGHT);
    }
}

bool Light::GetIsDynamic()
{
    return (flags & DYNAMIC_LIGHT) == DYNAMIC_LIGHT;
}

void Light::SetCastsShadow(const bool& castsShadow)
{
    if (castsShadow)
    {
        AddFlag(CASTS_SHADOW);
    }
    else
    {
        RemoveFlag(CASTS_SHADOW);
    }
}

bool Light::GetCastsShadow()
{
    return (flags & CASTS_SHADOW) == CASTS_SHADOW;
}

void Light::AddFlag(uint32 flag)
{
    flags |= flag;
}

void Light::RemoveFlag(uint32 flag)
{
    flags &= ~flag;
}

uint32 Light::GetFlags()
{
    return flags;
}

const Vector4& Light::CalculatePositionDirectionBindVector(Camera* inCamera)
{
    uint32 globalFrameIndex = Engine::Instance()->GetGlobalFrameIndex();
    if (inCamera != lastUsedCamera || lastUpdatedFrame != globalFrameIndex)
    {
        DVASSERT(inCamera);

        lastUsedCamera = inCamera;
        lastUpdatedFrame = globalFrameIndex;
        if (type == TYPE_DIRECTIONAL)
        {
            // Here we prepare direction according to shader direction usage.
            // Shaders use it as ToLightDirection, so we have to invert it here
            resultPositionDirection = -direction;
            resultPositionDirection.w = 0.0f;
            resultPositionDirection.Normalize();
        }
        else
        {
            resultPositionDirection = position;
        }
    }
    return resultPositionDirection;
}

void Light::SetColorTemperature(float k)
{
    /*
     * https://www.shadertoy.com/view/lsSXW1
     * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
     */
    float luminance = Max(1.0f, Max(baseColor.r, Max(baseColor.g, baseColor.b)));
    double temperatureInKelvins = Clamp(static_cast<double>(k), 1000.0, 40000.0) / 100.0;
    double cr = 1.0f;
    double cg = 1.0f;
    double cb = 1.0f;

    if (temperatureInKelvins <= 66.0)
    {
        cg = Clamp(0.39008157876901960784 * log(temperatureInKelvins) - 0.63184144378862745098, 0.0, 1.0);
    }
    else
    {
        double t = temperatureInKelvins - 60.0;
        cr = Clamp(1.29293618606274509804 * pow(t, -0.1332047592), 0.0, 1.0);
        cg = Clamp(1.12989086089529411765 * pow(t, -0.0755148492), 0.0, 1.0);
    }

    if (temperatureInKelvins <= 19.0)
        cb = 0.0;
    else if (temperatureInKelvins < 66.0)
        cb = Clamp(0.54320678911019607843 * log(temperatureInKelvins - 10.0) - 1.19625408914, 0.0, 1.0);

    SetColor(Color(luminance * static_cast<float>(cr), luminance * static_cast<float>(cg), luminance * static_cast<float>(cb), 1.0));
    lastSetColorTemperature = static_cast<float>(100.0 * temperatureInKelvins);
}

float Light::GetColorTemperature() const
{
    return lastSetColorTemperature;
}
};
