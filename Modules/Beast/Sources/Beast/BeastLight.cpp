#ifdef __DAVAENGINE_BEAST__

#include "BeastLight.h"
#include "BeastDebug.h"

#include "Scene3D/Components/ComponentHelpers.h"

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
        linearRGB = DAVA_BEAST::ILBLinearRGB(nodeColor.r, nodeColor.g, nodeColor.b);
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
        SetIntensity();
    }
}

DAVA_BEAST::ILBLightHandle BeastLight::GetILBLight()
{
    return light;
}

void BeastLight::CreateSkyLight()
{
    DAVA::Matrix4 davaMatrix;
    davaMatrix.Identity();
    DAVA_BEAST::ILBMatrix4x4 matrix = ConvertDavaMatrix(davaMatrix);

    BEAST_VERIFY(DAVA_BEAST::ILBCreateSkyLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &matrix, &GetColor(), &light));
    BEAST_VERIFY(DAVA_BEAST::ILBSetCastShadows(light, GetCastShadows()));
}

void BeastLight::CreateAmbientLight()
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreateAmbientLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
}

void BeastLight::CreateDirectionalLight()
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreateDirectionalLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
    BEAST_VERIFY(DAVA_BEAST::ILBSetCastShadows(light, GetCastShadows()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetShadowSamples(light, GetShadowSamples()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetShadowAngle(light, GetShadowAngle()));
}

void BeastLight::CreatePointLight()
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreatePointLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
    BEAST_VERIFY(DAVA_BEAST::ILBSetCastShadows(light, GetCastShadows()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetShadowRadius(light, GetShadowRadius()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetLightMaxRangeFalloff(light, GetFalloffCutoff(), GetFalloffExponent()));
}

void BeastLight::CreateSpotLight()
{
    BEAST_VERIFY(DAVA_BEAST::ILBCreateSpotLight(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &GetMatrix(), &GetColor(), &light));
    BEAST_VERIFY(DAVA_BEAST::ILBSetCastShadows(light, GetCastShadows()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetShadowRadius(light, GetShadowRadius()));
    BEAST_VERIFY(DAVA_BEAST::ILBSetLightMaxRangeFalloff(light, GetFalloffCutoff(), GetFalloffExponent()));
}

const DAVA_BEAST::ILBLinearRGB& BeastLight::GetColor() const
{
    return linearRGB;
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

const DAVA_BEAST::ILBMatrix4x4& BeastLight::GetMatrix() const
{
    return matrix;
}

void BeastLight::SetIntensity()
{
    DAVA::float32 intensity = 1.f;

    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        intensity = props->GetFloat("editor.intensity", 1.f);
    }

    BEAST_VERIFY(DAVA_BEAST::ILBSetLightIntensity(light, intensity));
}

DAVA::float32 BeastLight::GetShadowAngle()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetFloat("editor.staticlight.shadowangle", 0.f);
    }

    return 0.f;
}

DAVA::float32 BeastLight::GetShadowRadius()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetFloat("editor.staticlight.shadowradius", 0.f);
    }

    return 0.f;
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

DAVA::float32 BeastLight::GetFalloffCutoff()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetFloat("editor.staticlight.falloffcutoff", 1000.f);
    }

    return 1000.f;
}

DAVA::float32 BeastLight::GetFalloffExponent()
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        return props->GetFloat("editor.staticlight.falloffexponent", 1.f);
    }

    return 1.f;
}

void BeastLight::UpdateLightParamsFromHandle(DAVA_BEAST::ILBLightHandle lightHandle)
{
    DAVA_BEAST::ILBStringHandle name1, name2;
    DAVA_BEAST::ILBGetLightName(lightHandle, &name1);
    DAVA_BEAST::ILBGetLightName(light, &name2);
    DAVA::String davaName1 = ConvertBeastString(name1);
    DAVA::String davaName2 = ConvertBeastString(name2);

    if (davaName1 != davaName2)
        return;

    DAVA_BEAST::ILBLinearRGB color;
    BEAST_VERIFY(DAVA_BEAST::ILBGetLightColor(lightHandle, &color));
    lightObject->SetDiffuseColor(DAVA::Color(color.r, color.g, color.b, 1.f));

    DAVA_BEAST::ILBMatrix4x4 lightTransform;
    BEAST_VERIFY(DAVA_BEAST::ILBGetLightTransform(lightHandle, &lightTransform));
    DAVA::Matrix4 davaTransform = ConvertBeastMatrix(lightTransform);
    GetTransformComponent(ownerNode)->SetLocalTransform(&davaTransform);

    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (props)
    {
        DAVA::float32 intensity;
        BEAST_VERIFY(DAVA_BEAST::ILBGetLightIntensity(lightHandle, &intensity));

        if (lightObject->GetType() == DAVA::Light::TYPE_DIRECTIONAL)
        {
            DAVA::float32 shadowAngle;
            BEAST_VERIFY(DAVA_BEAST::ILBGetShadowAngle(lightHandle, &shadowAngle));
            props->SetFloat("editor.staticlight.shadowangle", shadowAngle);
        }
        else if (lightObject->GetType() != DAVA::Light::TYPE_SKY)
        {
            DAVA::float32 falloffCutoff, falloffExponent;
            BEAST_VERIFY(DAVA_BEAST::ILBGetLightMaxRangeFalloff(lightHandle, &falloffCutoff, &falloffExponent));
            props->GetFloat("editor.staticlight.falloffcutoff", falloffCutoff);
            props->GetFloat("editor.staticlight.falloffexponent", falloffExponent);
        }

        if (lightObject->GetType() == DAVA::Light::TYPE_POINT || lightObject->GetType() == DAVA::Light::TYPE_SPOT)
        {
            DAVA::float32 shadowRadius;
            BEAST_VERIFY(DAVA_BEAST::ILBGetShadowRadius(lightHandle, &shadowRadius));
            props->GetFloat("editor.staticlight.shadowradius", shadowRadius);
        }

        DAVA::int32 shadowSamples;
        BEAST_VERIFY(DAVA_BEAST::ILBGetLightShadowSamples(lightHandle, &shadowSamples));

        props->SetFloat("editor.intensity", intensity);
        props->GetInt32("editor.staticlight.shadowsamples", shadowSamples);
    }
}

#endif //__DAVAENGINE_BEAST__
