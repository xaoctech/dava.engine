#include "Render/Highlevel/ReflectionProbe.h"

namespace DAVA
{
ReflectionProbe::~ReflectionProbe()
{
    SetCurrentTexture(nullptr);
}

RenderObject* ReflectionProbe::Clone(RenderObject* newObject)
{
    DVASSERT(!"Should not clone reflection probes");
    return nullptr;
}

void ReflectionProbe::UpdateProbe()
{
    Matrix4 probeWorldToLocalMatrix = *GetWorldTransformPtr();
    probeWorldToLocalMatrix.Inverse();

    Matrix4 scaleMatrix = Matrix4::MakeScale(2.0f / GetCaptureSize());
    Matrix4 finalWorldToLocal = probeWorldToLocalMatrix * scaleMatrix;

    captureWorldToLocalMatrix = finalWorldToLocal;
    capturePositionInWorldSpace = GetPosition() + GetCapturePosition();
}

void ReflectionProbe::SetSphericalHarmonics(const Vector4 sh[9])
{
    memcpy(diffuseSphericalHarmonics, sh, sizeof(Vector4) * 9);
    containsUnprocessedSphericalHarmonics = true;

    /*
    DAVA::Logger::Info("Spherical harmonics:");
    for (uint32_t i = 0; i < 9; ++i)
    {
        DAVA::Logger::Info("globalDiffuseSphericalHarmonics[%u] = Vector4(%ff, %ff, %ff, 1.0f);", i,
                           diffuseSphericalHarmonics[i].x, diffuseSphericalHarmonics[i].y, diffuseSphericalHarmonics[i].z);

        if (!std::isfinite(diffuseSphericalHarmonics[i].x) || std::isnan(diffuseSphericalHarmonics[i].x))
            diffuseSphericalHarmonics[i].x = 0.0f;

        if (!std::isfinite(diffuseSphericalHarmonics[i].y) || std::isnan(diffuseSphericalHarmonics[i].y))
            diffuseSphericalHarmonics[i].y = 0.0f;

        if (!std::isfinite(diffuseSphericalHarmonics[i].z) || std::isnan(diffuseSphericalHarmonics[i].z))
            diffuseSphericalHarmonics[i].z = 0.0f;
    }
    // */
}

void ReflectionProbe::OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded)
{
    DVASSERT(original == currentTexture);
    currentTexture = std::static_pointer_cast<Texture>(reloaded);
}
};
