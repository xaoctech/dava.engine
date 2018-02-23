#pragma once

#include "Base/StaticSingleton.h"
#include "Math/Vector.h"
#include "Scene3D/Systems/ParticlesQualitySettings.h"
#include "Render/Renderer.h"

namespace DAVA
{
enum QualityGroup : uint32
{
    Anisotropy,
    Textures,
    Shadow,
    Scattering,
    Antialiasing,

    Count,
};

struct ShadowQuality
{
    uint32 cascadesCount = 1;
    bool enablePCF = false;
    ShadowQuality(uint32 c, bool pcf)
        : cascadesCount(c)
        , enablePCF(pcf)
    {
    }
};

struct TexturesQuality
{
    uint32 baseLevel = 0;
    TexturesQuality(uint32 b)
        : baseLevel(b)
    {
    }
};

struct ScatteringQuality
{
    uint32 scatteringSamples = 8;
    ScatteringQuality(uint32 smp)
        : scatteringSamples(smp)
    {
    }
};

template <QualityGroup group>
struct QualityGroupValueClass;
template <>
struct QualityGroupValueClass<QualityGroup::Anisotropy>
{
    using ValueClass = uint32;
};
template <>
struct QualityGroupValueClass<QualityGroup::Antialiasing>
{
    using ValueClass = rhi::AntialiasingType;
};
template <>
struct QualityGroupValueClass<QualityGroup::Shadow>
{
    using ValueClass = ShadowQuality;
};
template <>
struct QualityGroupValueClass<QualityGroup::Textures>
{
    using ValueClass = TexturesQuality;
};
template <>
struct QualityGroupValueClass<QualityGroup::Scattering>
{
    using ValueClass = ScatteringQuality;
};

struct MaterialQuality
{
    FastName qualityName;
};

struct LandscapeQuality
{
    union
    {
        struct
        {
            float32 normalMaxHeightError;
            float32 normalMaxPatchRadiusError;
            float32 normalMaxAbsoluteHeightError;

            float32 zoomMaxHeightError;
            float32 zoomMaxPatchRadiusError;
            float32 zoomMaxAbsoluteHeightError;
        };
        std::array<float32, 6> metricsArray;
    };
    bool morphing;
};

class QualitySettingsComponent;
class QualitySettingsSystem : public StaticSingleton<QualitySettingsSystem>
{
public:
    static const FastName QUALITY_OPTION_VEGETATION_ANIMATION;
    static const FastName QUALITY_OPTION_STENCIL_SHADOW;
    static const FastName QUALITY_OPTION_WATER_DECORATIONS;
    static const FastName QUALITY_OPTION_DISABLE_EFFECTS;
    static const FastName QUALITY_OPTION_LOD0_EFFECTS;

    static const FastName QUALITY_OPTION_DISABLE_FOG;
    static const FastName QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_ATTENUATION;
    static const FastName QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_SCATTERING;
    static const FastName QUALITY_OPTION_DISABLE_FOG_HALF_SPACE;

    static const FastName QUALITY_OPTION_DEFERRED_DRAW_FORWARD;
    static const FastName QUALITY_OPTION_HALF_RESOLUTION_3D;

    QualitySettingsSystem();

    void Load(const FilePath& path);

    const FastName& GetQualityGroupName(QualityGroup group) const;
    const FastName& GetCurrentQualityForGroup(QualityGroup group) const;
    Vector<FastName> GetAvailableQualitiesForGroup(QualityGroup group) const;
    void SetCurrentQualityForGroup(QualityGroup group, const FastName& value);
    bool HasQualityInGroup(QualityGroup group, const FastName& value) const;

    template <QualityGroup group>
    typename QualityGroupValueClass<group>::ValueClass GetCurrentQualityValue() const;

    // materials quality
    size_t GetMaterialQualityGroupCount() const;
    FastName GetMaterialQualityGroupName(size_t index) const;
    size_t GetMaterialQualityCount(const FastName& group) const;
    FastName GetMaterialQualityName(const FastName& group, size_t index) const;
    FastName GetCurMaterialQuality(const FastName& group) const;
    void SetCurMaterialQuality(const FastName& group, const FastName& quality);
    const MaterialQuality* GetMaterialQuality(const FastName& group, const FastName& quality) const;

    // sound quality
    size_t GetSFXQualityCount() const;
    FastName GetSFXQualityName(size_t index) const;
    FastName GetCurSFXQuality() const;
    void SetCurSFXQuality(const FastName& name);
    FilePath GetSFXQualityConfigPath(const FastName& name) const;
    FilePath GetSFXQualityConfigPath(size_t index) const;

    // landscape
    size_t GetLandscapeQualityCount() const;
    FastName GetLandscapeQualityName(size_t index) const;

    FastName GetCurLandscapeQuality() const;
    void SetCurLandscapeQuality(const FastName& name);

    const LandscapeQuality* GetLandscapeQuality(const FastName& name) const;

    // particles
    const ParticlesQualitySettings& GetParticlesQualitySettings() const;
    ParticlesQualitySettings& GetParticlesQualitySettings();

    // ------------------------------------------

    void EnableOption(const FastName& option, bool enabled);
    bool IsOptionEnabled(const FastName& option) const;
    int32 GetOptionsCount() const;
    FastName GetOptionName(int32 index) const;

    bool IsQualityVisible(const Entity* entity);

    void UpdateEntityAfterLoad(Entity* entity);

    bool GetAllowCutUnusedVertexStreams();
    void SetAllowCutUnusedVertexStreams(bool cut);

    void SetKeepUnusedEntities(bool keep);
    bool GetKeepUnusedEntities();

    void SetForceRescale(bool force);
    bool GetForceRescale();

    void SetRuntimeQualitySwitching(bool enabled);
    bool GetRuntimeQualitySwitching();

    void UpdateEntityVisibility(Entity* e);

private:
    struct QualityGroupHolder
    {
        FastName groupName;
        FastName currentValue;
        Map<FastName, Any, FastName::LexicographicalComparator> values;
    };
    QualityGroupHolder qualityGroups[uint32(QualityGroup::Count)];

protected:
    void UpdateEntityVisibilityRecursively(Entity* e, bool qualityVisible);

protected:
    struct MAGrQ
    {
        uint32 curQuality = 0;
        Vector<MaterialQuality> qualities;
    };
    UnorderedMap<FastName, MAGrQ> materialGroups;

    struct SFXQ
    {
        FastName name;
        FilePath configPath;
    };

    struct LCQ
    {
        FastName name;
        LandscapeQuality quality;
    };
    Vector<SFXQ> soundQualities;
    int32 curSoundQuality = 0;

    // landscape
    int32 curLandscapeQuality = 0;
    Vector<LCQ> landscapeQualities;

    Map<FastName, bool, FastName::LexicographicalComparator> qualityOptions;

    ParticlesQualitySettings particlesQualitySettings;

    bool cutUnusedVertexStreams = false;
    bool keepUnusedQualityEntities = false; // for editor to prevent cutting entities with unused quality
    bool runtimeQualitySwitching = false;
    bool forceRescale = false; //GFX_COMPLETE for editor to correctly draw debug stuff - uses additional render pass. later might be replaced with txaa settings
};

inline void QualitySettingsSystem::SetKeepUnusedEntities(bool keep)
{
    keepUnusedQualityEntities = keep;
}

inline bool QualitySettingsSystem::GetKeepUnusedEntities()
{
    return keepUnusedQualityEntities;
}

inline void QualitySettingsSystem::SetRuntimeQualitySwitching(bool enabled)
{
    runtimeQualitySwitching = enabled;
}
inline bool QualitySettingsSystem::GetRuntimeQualitySwitching()
{
    return runtimeQualitySwitching;
}

inline void QualitySettingsSystem::SetForceRescale(bool force)
{
    forceRescale = force;
}
inline bool QualitySettingsSystem::GetForceRescale()
{
    return forceRescale;
}

template <QualityGroup group>
inline typename QualityGroupValueClass<group>::ValueClass QualitySettingsSystem::GetCurrentQualityValue() const
{
    const FastName& currentValue = GetCurrentQualityForGroup(group);
    Any value = qualityGroups[group].values.at(currentValue);
    DVASSERT(value.CanGet<typename QualityGroupValueClass<group>::ValueClass>());
    return value.Get<typename QualityGroupValueClass<group>::ValueClass>();
}
}
