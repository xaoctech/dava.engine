#pragma once

#include "Base/BaseTypes.h"
#include "Animation/AnimatedObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class NMaterial;

class ReflectionProbe : public RenderObject
{
public:
    enum eType
    {
        TYPE_NONE = 0,
        TYPE_GLOBAL = 1,
        TYPE_STATIC_CUBEMAP = 2,
    };

    static const uint32 INVALID_QUALITY_LEVEL = 0xFFFFFFFF;

    ReflectionProbe();
    virtual ~ReflectionProbe();

    void SetReflectionType(eType type_);
    eType GetReflectionType() const;
    void SetPosition(const Vector3& position_);
    const Vector3& GetPosition() const;

    void SetCapturePosition(const Vector3& capturePosition_);
    const Vector3& GetCapturePosition() const;
    void SetCaptureSize(const Vector3& captureSize_);
    const Vector3& GetCaptureSize() const;

    void UpdateProbe();

    virtual RenderObject* Clone(RenderObject* newObject);

    void SetCurrentTexture(Texture* texture_);
    Texture* GetCurrentTexture() const;

    void SetNextQualityLevel(uint32 qualityLevel_);
    uint32 GetNextQualityLevel() const;
    void SetActiveQualityLevel(uint32 activeQualityLevel_);
    uint32 GetActiveQualityLevel() const;

    const Vector3& GetCapturePositionInWorldSpace() const;
    const Matrix4& GetCaptureWorldToLocalMatrix() const;

protected:
    eType reflectionType = TYPE_STATIC_CUBEMAP;
    Vector3 position;
    Vector3 capturePosition;
    Vector3 captureSize;

    Vector3 capturePositionInWorldSpace;
    Matrix4 captureWorldToLocalMatrix;

    Texture* currentTexture = nullptr;
    //uint32 textureCacheIndex = -1;

    uint32 nextQualityLevel = INVALID_QUALITY_LEVEL;
    uint32 activeQualityLevel = INVALID_QUALITY_LEVEL;
};

inline void ReflectionProbe::SetPosition(const Vector3& position_)
{
    position = position_;
}

inline ReflectionProbe::eType ReflectionProbe::GetReflectionType() const
{
    return reflectionType;
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

inline void ReflectionProbe::SetCurrentTexture(Texture* texture_)
{
    currentTexture = texture_;
}

inline Texture* ReflectionProbe::GetCurrentTexture() const
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

} // ns
