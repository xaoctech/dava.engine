#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <Base/BaseTypes.h>
#include <Math/AABBox3.h>
#include <Render/Highlevel/Landscape.h>

//#define TERRAIN_UNITTEST

#ifdef TERRAIN_UNITTEST
#include "terrain/space.h"
#endif

class Terrain
{
public:
    Terrain(DAVA::Landscape* _land);
    ~Terrain();
    bool PlacePoint(const DAVA::Vector3& point, DAVA::Vector3& outResult, DAVA::Vector3* outNormal);
    bool PlaceLandscapePoint(const DAVA::Vector3& point, DAVA::Vector3& outResult, DAVA::Vector3* outNormal);
    DAVA::Vector<DAVA::float32>& GetHeightMap();
    DAVA::int32 GetHeightMapSize();
#ifdef TERRAIN_UNITTEST
    DAVA::int32 BuildSolidHM(DAVA::Vector<DAVA::float32>& hmap);
    bool OldPlacePoint(const DAVA::Vector3& point, DAVA::Vector3& result, DAVA::Vector3* normal);
    DAVA::float32 HeightAt(const Chunk& chunk, DAVA::int32 x, DAVA::int32 y);
    virtual void Draw(DAVA::RenderHelper* drawer);
#endif
    DAVA::AABBox3 GetBBox();
    inline DAVA::Landscape* GetLandscape()
    {
        return land;
    };

protected:
    DAVA::Landscape* land;
    DAVA::Vector<DAVA::float32> heightMap; 
#ifdef TERRAIN_UNITTEST
    Space space;
#endif

    DAVA::int32 sizeInChunks;
    DAVA::float32 newRealSize;

private:
    static const DAVA::float32 CHUNK_SIZE_IN_METERS;
    static const DAVA::int32 STRIDE_SIZE;
};

const DAVA::float32 QUANTISATION_LEVEL = 1000.0f; // quantise heights to mm
inline DAVA::int32 Quantise(DAVA::float32 h)
{
    return (DAVA::int32)(h * QUANTISATION_LEVEL + 0.5f);
}

inline DAVA::float32 Unquantise(DAVA::int32 q)
{
    return (DAVA::float32)q * (1.f / QUANTISATION_LEVEL);
}

#endif
