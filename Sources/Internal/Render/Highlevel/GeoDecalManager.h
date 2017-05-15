#pragma once

#include "Functional/Function.h"
#include "FileSystem/FilePath.h"
#include "Math/AABBox3.h"
#include <atomic>

namespace DAVA
{
class Texture;
class RenderSystem;
class RenderObject;
class RenderBatch;
class RenderBatchProvider;
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

    /*
     * Removes all decals associated with provided RenderObject
     */
    void RemoveRenderObject(RenderObject*);

private:
    struct DecalVertex;
    struct DecalBuildInfo;

    struct BuiltDecal
    {
        RenderObject* sourceObject = nullptr;
        RenderBatchProvider* batchProvider = nullptr;

        BuiltDecal() = default;
        BuiltDecal(const BuiltDecal& r);
        BuiltDecal& operator=(const BuiltDecal&);
        ~BuiltDecal();

        BuiltDecal(BuiltDecal&&) = delete;
        BuiltDecal& operator=(BuiltDecal&&) = delete;
    };

    void RegisterDecal(Decal decal);
    void UnregisterDecal(Decal decal);

    bool BuildDecal(const DecalBuildInfo& info, RenderBatch* dstBatch);
    void ClipToPlane(DecalVertex* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v);
    void ClipToBoundingBox(DecalVertex* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper);
    int8_t Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalVertex& p_v);
    void Lerp(float t, const DecalVertex& v1, const DecalVertex& v2, DecalVertex& result);

    void GetStaticMeshGeometry(const DecalBuildInfo& info, Vector<uint8>& buffer);
    void GetSkinnedMeshGeometry(const DecalBuildInfo& info, Vector<uint8>& buffer);
    void AddVerticesToGeometry(const DecalBuildInfo& info, DecalVertex* points, Vector<uint8>& buffer);

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
