#include "Beast/BeastLight.h"
#include "Beast/BeastDebug.h"

#include <Base/BaseMath.h>
#include <FileSystem/KeyedArchive.h>
#include <Math/Color.h>
#include <Render/Highlevel/Light.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>

namespace Beast
{
BeastLight::BeastLight(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
{
}

void BeastLight::InitWithLight(DAVA::Entity* owner, DAVA::Light* _light)
{
    if (light == nullptr)
    {
        ownerNode = owner;
        lightObject = _light;

        DAVA::Color nodeColor = lightObject->GetDiffuseColor();
        linearRGB = ILBLinearRGB(nodeColor.r, nodeColor.g, nodeColor.b);
        matrix = ConvertDavaMatrix(ownerNode->GetWorldTransform());

        switch (lightObject->GetType())
        {
        case DAVA::Light::TYPE_SKY:
            CreateSkyLight();
            break;

        case DAVA::Light::TYPE_DIRECTIONAL:
            CreateDirectionalLight();
            break;

        case DAVA::Light::TYPE_POINT:
            CreatePointLight();
            break;

        case DAVA::Light::TYPE_AMBIENT:
            CreateAmbientLight();
            break;

        case DAVA::Light::TYPE_SPOT:
            CreateSpotLight();
            break;

        default:
            DVASSERT(0, "Invalid light type");
        }

        SetShadowParameters();
        SetIntensity();
    }
}

ILBLightHandle BeastLight::GetILBLight()
{
    return light;
}

void BeastLight::CreateSkyLight()
{
    DAVA::Matrix4 davaMatrix;
    davaMatrix.Identity();
    ILBMatrix4x4 matrix = ConvertDavaMatrix(davaMatrix);

    BEAST_VERIFY(ILBCreateSkyLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &matrix, &GetColor(), &light));
    BEAST_VERIFY(ILBSetCastShadows(light, GetCastShadows()));
}

void BeastLight::CreateAmbientLight()
{
    BEAST_VERIFY(ILBCreateAmbientLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
}

void BeastLight::CreateDirectionalLight()
{
    BEAST_VERIFY(ILBCreateDirectionalLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
}

void BeastLight::CreatePointLight()
{
    BEAST_VERIFY(ILBCreatePointLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
    BEAST_VERIFY(ILBSetLightMaxRangeFalloff(light, GetFalloffCutoff(), GetFalloffExponent()));
}

void BeastLight::CreateSpotLight()
{
    BEAST_VERIFY(ILBCreateSpotLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
    BEAST_VERIFY(ILBSetLightMaxRangeFalloff(light, GetFalloffCutoff(), GetFalloffExponent()));
    BEAST_VERIFY(ILBSetSpotlightCone(light, GetConeAngle() * DAVA::PI / 180.0f, GetConePenumbraAngle() * DAVA::PI / 180.0f, GetConePenumbraExponent()));
}

void BeastLight::SetShadowParameters()
{
    BEAST_VERIFY(ILBSetCastShadows(light, GetCastShadows()));
    BEAST_VERIFY(ILBSetShadowRadius(light, GetShadowRadius()));
    BEAST_VERIFY(ILBSetShadowSamples(light, GetShadowSamples()));
    BEAST_VERIFY(ILBSetShadowAngle(light, GetShadowAngle()));
}

const ILBLinearRGB& BeastLight::GetColor() const
{
    return linearRGB;
}

DAVA::float32 BeastLight::GetFloat(const DAVA::String& key, DAVA::float32 defaultValue)
{
    DAVA::float32 result = defaultValue;
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props != nullptr)
    {
        result = props->GetFloat(key, defaultValue);
    }
    return result;
}

bool BeastLight::GetCastShadows()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetBool("editor.staticlight.castshadows", true);
    }
    return true;
}

DAVA::int32 BeastLight::GetShadowSamples()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetInt32("editor.staticlight.shadowsamples", 1);
    }
    return 1;
}

const ILBMatrix4x4& BeastLight::GetMatrix() const
{
    return matrix;
}

void BeastLight::SetIntensity()
{
    DAVA::float32 intensity = GetFloat("editor.intensity", 1.0f);
    BEAST_VERIFY(ILBSetLightIntensity(light, intensity));
}

DAVA::float32 BeastLight::GetShadowAngle()
{
    return GetFloat("editor.staticlight.shadowangle", 0.0f);
}

DAVA::float32 BeastLight::GetShadowRadius()
{
    return GetFloat("editor.staticlight.shadowradius", 0.0f);
}

DAVA::float32 BeastLight::GetFalloffCutoff()
{
    return GetFloat("editor.staticlight.falloffcutoff", 1000.f);
}

DAVA::float32 BeastLight::GetFalloffExponent()
{
    return GetFloat("editor.staticlight.falloffexponent", 1.0f);
}

DAVA::float32 BeastLight::GetConeAngle()
{
    return GetFloat("editor.staticlight.cone.angle", 90.0f);
}

DAVA::float32 BeastLight::GetConePenumbraAngle()
{
    return GetFloat("editor.staticlight.cone.penumbra.angle", 0.0f);
}

DAVA::float32 BeastLight::GetConePenumbraExponent()
{
    return GetFloat("editor.staticlight.cone.penumbra.exponent", 1.0f);
}

void BeastLight::UpdateLightParamsFromHandle(ILBLightHandle lightHandle)
{
    ILBStringHandle name1, name2;
    ILBGetLightName(lightHandle, &name1);
    ILBGetLightName(light, &name2);
    DAVA::String davaName1 = ConvertBeastString(name1);
    DAVA::String davaName2 = ConvertBeastString(name2);

    if (davaName1 != davaName2)
        return;

    ILBLinearRGB color;
    BEAST_VERIFY(ILBGetLightColor(lightHandle, &color));
    lightObject->SetDiffuseColor(DAVA::Color(color.r, color.g, color.b, 1.f));

    ILBMatrix4x4 lightTransform;
    BEAST_VERIFY(ILBGetLightTransform(lightHandle, &lightTransform));
    DAVA::Matrix4 davaTransform = ConvertBeastMatrix(lightTransform);
    GetTransformComponent(ownerNode)->SetLocalTransform(&davaTransform);

    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        DAVA::float32 intensity;
        BEAST_VERIFY(ILBGetLightIntensity(lightHandle, &intensity));

        if (lightObject->GetType() == DAVA::Light::TYPE_DIRECTIONAL)
        {
            DAVA::float32 shadowAngle;
            BEAST_VERIFY(ILBGetShadowAngle(lightHandle, &shadowAngle));
            props->SetFloat("editor.staticlight.shadowangle", shadowAngle);
        }
        else if (lightObject->GetType() != DAVA::Light::TYPE_SKY)
        {
            DAVA::float32 falloffCutoff, falloffExponent;
            BEAST_VERIFY(ILBGetLightMaxRangeFalloff(lightHandle, &falloffCutoff, &falloffExponent));
            props->GetFloat("editor.staticlight.falloffcutoff", falloffCutoff);
            props->GetFloat("editor.staticlight.falloffexponent", falloffExponent);
        }

        if (lightObject->GetType() == DAVA::Light::TYPE_POINT || lightObject->GetType() == DAVA::Light::TYPE_SPOT)
        {
            DAVA::float32 shadowRadius;
            BEAST_VERIFY(ILBGetShadowRadius(lightHandle, &shadowRadius));
            props->GetFloat("editor.staticlight.shadowradius", shadowRadius);
        }

        DAVA::int32 shadowSamples;
        BEAST_VERIFY(ILBGetLightShadowSamples(lightHandle, &shadowSamples));

        props->SetFloat("editor.intensity", intensity);
        props->GetInt32("editor.staticlight.shadowsamples", shadowSamples);
    }
}
}