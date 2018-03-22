#pragma once

#include "Base/BaseTypes.h"
#include "Animation/AnimatedObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Asset/AssetListener.h"
#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class NMaterial;

class ReflectionProbe : public RenderObject, public AssetListener
{
public:
    enum class ProbeType : uint32
    {
        LOCAL,
        LOCAL_STATIC,
        GLOBAL,
        GLOBAL_STATIC,
    };

    static const uint32 INVALID_QUALITY_LEVEL = 0xFFFFFFFF;

    ReflectionProbe() = default;
    ~ReflectionProbe();

    void SetReflectionType(ProbeType type_);
    ProbeType GetReflectionType() const;

    bool IsLocalProbe() const;
    bool IsGlobalProbe() const;
    bool IsDynamicProbe() const;
    bool IsStaticProbe() const;

    void SetPosition(const Vector3& position_);
    const Vector3& GetPosition() const;

    void SetCapturePosition(const Vector3& capturePosition_);
    const Vector3& GetCapturePosition() const;
    void SetCaptureSize(const Vector3& captureSize_);
    const Vector3& GetCaptureSize() const;

    void UpdateProbe();

    RenderObject* Clone(RenderObject* newObject) override;

    void SetCurrentTexture(const Asset<Texture>& texture);
    const Asset<Texture>& GetCurrentTexture() const;

    void SetNextQualityLevel(uint32 qualityLevel_);
    uint32 GetNextQualityLevel() const;
    void SetActiveQualityLevel(uint32 activeQualityLevel_);
    uint32 GetActiveQualityLevel() const;

    Vector4* GetSphericalHarmonicsArray();
    const Vector4* GetSphericalHarmonicsArray() const;
    void SetSphericalHarmonics(const Vector4 sh[9]);

    bool ContainsUnprocessedSphericalHarmonics() const;
    void MarkSphericalHarmonicsAsProcessed();

    const Vector3& GetCapturePositionInWorldSpace() const;
    const Matrix4& GetCaptureWorldToLocalMatrix() const;

    void OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded) override;

protected:
    ProbeType probeType = ProbeType::GLOBAL;
    Vector3 position;
    Vector3 capturePosition;
    Vector3 captureSize;
    Vector3 capturePositionInWorldSpace;
    Matrix4 captureWorldToLocalMatrix;
    Asset<Texture> currentTexture;
    uint32 nextQualityLevel = INVALID_QUALITY_LEVEL;
    uint32 activeQualityLevel = INVALID_QUALITY_LEVEL;
    Vector4 diffuseSphericalHarmonics[9];
    bool containsUnprocessedSphericalHarmonics = false;
};

inline void ReflectionProbe::SetPosition(const Vector3& position_)
{
    position = position_;
}

inline ReflectionProbe::ProbeType ReflectionProbe::GetReflectionType() const
{
    return probeType;
}

inline void ReflectionProbe::SetReflectionType(ProbeType t)
{
    probeType = t;
}

inline const Vector3& ReflectionProbe::GetPosition() const
{
    return position;
}

inline void ReflectionProbe::SetCapturePosition(const Vector3& capturePosition_)
{
    capturePosition = capturePosition_;
}

inline const Vector3& ReflectionProbe::GetCapturePosition() const
{
    return capturePosition;
}

inline void ReflectionProbe::SetCaptureSize(const Vector3& captureSize_)
{
    captureSize = captureSize_;
}

inline const Vector3& ReflectionProbe::GetCaptureSize() const
{
    return captureSize;
}

inline void ReflectionProbe::SetCurrentTexture(const Asset<Texture>& texture_)
{
    if (texture_ != currentTexture)
    {
        AssetManager* assetManager = GetEngineContext()->assetManager;
        assetManager->UnregisterListener(currentTexture, this);
        currentTexture = texture_;
        assetManager->RegisterListener(currentTexture, this);
    }
}

inline const Asset<Texture>& ReflectionProbe::GetCurrentTexture() const
{
    return currentTexture;
}

inline void ReflectionProbe::SetNextQualityLevel(uint32 qualityLevel_)
{
    nextQualityLevel = qualityLevel_;
}

inline uint32 ReflectionProbe::GetNextQualityLevel() const
{
    return nextQualityLevel;
}

inline void ReflectionProbe::SetActiveQualityLevel(uint32 activeQualityLevel_)
{
    activeQualityLevel = activeQualityLevel_;
}

inline uint32 ReflectionProbe::GetActiveQualityLevel() const
{
    return activeQualityLevel;
}

inline const Vector3& ReflectionProbe::GetCapturePositionInWorldSpace() const
{
    return capturePositionInWorldSpace;
}

inline const Matrix4& ReflectionProbe::GetCaptureWorldToLocalMatrix() const
{
    return captureWorldToLocalMatrix;
}

inline Vector4* ReflectionProbe::GetSphericalHarmonicsArray()
{
    return diffuseSphericalHarmonics;
}

inline const Vector4* ReflectionProbe::GetSphericalHarmonicsArray() const
{
    return diffuseSphericalHarmonics;
}

inline bool ReflectionProbe::ContainsUnprocessedSphericalHarmonics() const
{
    return containsUnprocessedSphericalHarmonics;
}

inline void ReflectionProbe::MarkSphericalHarmonicsAsProcessed()
{
    containsUnprocessedSphericalHarmonics = false;
}

inline bool ReflectionProbe::IsLocalProbe() const
{
    return (probeType == ProbeType::LOCAL) || (probeType == ProbeType::LOCAL_STATIC);
}

inline bool ReflectionProbe::IsGlobalProbe() const
{
    return (probeType == ProbeType::GLOBAL) || (probeType == ProbeType::GLOBAL_STATIC);
}

inline bool ReflectionProbe::IsDynamicProbe() const
{
    return (probeType == ProbeType::LOCAL) || (probeType == ProbeType::GLOBAL);
}

inline bool ReflectionProbe::IsStaticProbe() const
{
    return (probeType == ProbeType::LOCAL_STATIC) || (probeType == ProbeType::GLOBAL_STATIC);
}

} // ns
