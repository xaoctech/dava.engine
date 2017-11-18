#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastResource.h"
#include "Beast/BeastManager.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class Entity;
class Light;
}

namespace Beast
{
class BeastLight : public BeastResource<BeastLight>
{
public:
    void InitWithLight(DAVA::Entity* node, DAVA::Light* lightObject);
    void UpdateLightParamsFromHandle(ILBLightHandle lightHandle);

    ILBLightHandle GetILBLight();

private:
    friend class BeastResource<BeastLight>;
    BeastLight(const DAVA::String& name, BeastManager* manager);

    void CreateSkyLight();
    void CreateDirectionalLight();
    void CreateSpotLight();
    void CreatePointLight();
    void CreateAmbientLight();
    void SetIntensity();
    bool GetCastShadows();
    void SetShadowParameters();

    const ILBLinearRGB& GetColor() const;
    const ILBMatrix4x4& GetMatrix() const;
    DAVA::int32 GetShadowSamples();
    DAVA::float32 GetShadowAngle();
    DAVA::float32 GetShadowRadius();
    DAVA::float32 GetFalloffCutoff();
    DAVA::float32 GetFalloffExponent();
    DAVA::float32 GetConeAngle();
    DAVA::float32 GetConePenumbraAngle();
    DAVA::float32 GetConePenumbraExponent();

    DAVA::float32 GetFloat(const DAVA::String& key, DAVA::float32 defaultValue);

private:
    ILBLightHandle light = nullptr;
    DAVA::Light* lightObject = nullptr;
    DAVA::Entity* ownerNode = nullptr;
    ILBMatrix4x4 matrix;
    ILBLinearRGB linearRGB;
};
}