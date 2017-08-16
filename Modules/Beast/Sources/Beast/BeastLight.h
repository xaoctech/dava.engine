#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_LIGHT__
#define __BEAST_LIGHT__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"
#include "BeastManager.h"

class BeastLight : public BeastResource<BeastLight>
{
public:
    void InitWithLight(DAVA::Entity* node, DAVA::Light* lightObject);
    void UpdateLightParamsFromHandle(DAVA_BEAST::ILBLightHandle lightHandle);

    DAVA_BEAST::ILBLightHandle GetILBLight();

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

    const DAVA_BEAST::ILBLinearRGB& GetColor() const;
    const DAVA_BEAST::ILBMatrix4x4& GetMatrix() const;
    DAVA::int32 GetShadowSamples();
    DAVA::float32 GetShadowAngle();
    DAVA::float32 GetShadowRadius();
    DAVA::float32 GetFalloffCutoff();
    DAVA::float32 GetFalloffExponent();

private:
    DAVA_BEAST::ILBLightHandle light = nullptr;
    DAVA::Light* lightObject = nullptr;
    DAVA::Entity* ownerNode = nullptr;
    DAVA_BEAST::ILBMatrix4x4 matrix;
    DAVA_BEAST::ILBLinearRGB linearRGB;
};

#endif //__BEAST_LIGHT__

#endif //__DAVAENGINE_BEAST__
