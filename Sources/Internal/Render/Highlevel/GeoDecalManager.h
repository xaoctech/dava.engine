#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include <atomic>

namespace DAVA
{
class RenderSystem;
class RenderObject;
class RenderBatch;
class Texture;
class GeoDecalManager
{
public:
    enum Mapping : uint32
    {
        PLANAR,
        SPHERICAL,
        CYLINDRICAL,

        COUNT
    };

    enum SyncFields : uint32
    {
        SYNC_FLAGS = 1 << 0,
        SYNC_LOD = 1 << 1,
        SYNC_SWITCH = 1 << 2,
        SYNC_LIGHTS = 1 << 3,
        SYNC_SKELETON = 1 << 4,

        SYNC_ALL = 0xFFFFFFFF
    };

    struct DecalConfig
    {
        AABBox3 boundingBox;
        Mapping mapping = Mapping::PLANAR;
        FilePath albedo;
        FilePath normal;
        Vector2 uvOffset;
        Vector2 uvScale = Vector2(1.0f, 1.0f);

        bool operator==(const DecalConfig&) const;
        bool operator!=(const DecalConfig&) const;
        void invalidate();
    };

    using Decal = struct
    {
        uint32 key = 0xDECAAAAA;
    }*;

    static const Decal InvalidDecal;

public:
    GeoDecalManager(RenderSystem* renderSystem);

    Decal BuildDecal(const DecalConfig& config, const Matrix4& decalWorldTransform, RenderObject* object);
    void DeleteDecal(Decal decal);

    void SyncDecals(uint32 fields = SYNC_ALL);
    void EnumerateDecalRenderObjects(RenderObject* ro, Function<void(RenderObject*)> func) const;

private:
    struct DecalVertex;
    struct DecalBuildInfo;

    struct BuiltDecal
    {
        RefPtr<RenderObject> sourceObject = RefPtr<RenderObject>(nullptr);
        RefPtr<RenderObject> renderObject = RefPtr<RenderObject>(nullptr);
        bool registered = false;
    };

    void SyncDecalsWithRenderObject(RenderObject* ro, uint32 fields);

    RenderObject* GetDecalRenderObject(Decal decal) const;

    void RegisterDecal(Decal decal);
    void UnregisterDecal(Decal decal);
    void SyncRenderObjects(RenderObject* source, RenderObject* decal, uint32 fields);
    bool BuildDecal(const DecalBuildInfo& info, RenderBatch* dstBatch);
    void ClipToPlane(DecalVertex* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v);
    void ClipToBoundingBox(DecalVertex* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper);
    int8_t Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalVertex& p_v);
    void Lerp(float t, const DecalVertex& v1, const DecalVertex& v2, DecalVertex& result);

private:
    RenderSystem* renderSystem = nullptr;
    Map<Decal, BuiltDecal> builtDecals;
    std::atomic<uintptr_t> decalCounter{ 0 };
};

inline bool GeoDecalManager::DecalConfig::operator==(const GeoDecalManager::DecalConfig& r) const
{
    return (boundingBox == r.boundingBox) && (albedo == r.albedo) && (normal == r.normal) && (mapping == r.mapping) &&
    (uvOffset == r.uvOffset) && (uvScale == r.uvScale);
}

inline bool GeoDecalManager::DecalConfig::operator!=(const GeoDecalManager::DecalConfig& r) const
{
    return (boundingBox != r.boundingBox) || (albedo != r.albedo) || (normal != r.normal) || (mapping != r.mapping) ||
    (uvOffset != r.uvOffset) || (uvScale != r.uvScale);
}

inline void GeoDecalManager::DecalConfig::invalidate()
{
    boundingBox.Empty();
    albedo = FilePath();
    normal = FilePath();
}
}
