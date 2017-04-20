#pragma once

#include "Base/BaseTypes.h"

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

    struct DecalConfig
    {
        AABBox3 boundingBox;
        Matrix4 worldTransform;
        Mapping mapping = Mapping::PLANAR;
        FilePath image;

        bool operator==(const DecalConfig&) const;
        bool operator!=(const DecalConfig&) const;
        void invalidate();
    };

    using Decal = struct
    {
        uint32 key = 0xDECA1111;
    }*;

public:
    GeoDecalManager();

    Decal BuildDecals(const DecalConfig& config, const Vector<RenderObject*>& objects);
    void DeleteDelal(Decal decal);

private:
    struct DecalVertex;
    struct DecalBuildInfo;
    struct DecalRenderBatch;

    struct DecalBatches
    {
        RenderObject* object = nullptr;
        Vector<RenderBatch*> batches;
    };

    struct BuiltDecal
    {
        Vector<DecalBatches> batches;
    };

    bool BuildDecal(const DecalBuildInfo& info, DecalRenderBatch& batch);
    void ClipToPlane(DecalVertex* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v);
    void ClipToBoundingBox(DecalVertex* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper);
    int8_t Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalVertex& p_v);
    void Lerp(float t, const DecalVertex& v1, const DecalVertex& v2, DecalVertex& result);

private:
    Texture* defaultNormalMap = nullptr;
    Map<Decal, BuiltDecal> builtDecals;
};

inline bool GeoDecalManager::DecalConfig::operator==(const GeoDecalManager::DecalConfig& r) const
{
    return (boundingBox == r.boundingBox) &&
    (image == r.image) &&
    (mapping == r.mapping);
}

inline bool GeoDecalManager::DecalConfig::operator!=(const GeoDecalManager::DecalConfig& r) const
{
    return (boundingBox != r.boundingBox) ||
    (image != r.image) ||
    (mapping != r.mapping);
}

inline void GeoDecalManager::DecalConfig::invalidate()
{
    boundingBox.Empty();
    image = FilePath();
}
}