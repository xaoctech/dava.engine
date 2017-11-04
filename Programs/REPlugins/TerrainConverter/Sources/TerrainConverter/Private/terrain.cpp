#include "terrain.h"
#include <physics2/p2conf.hpp>
#include <physics2/worldtri.hpp>
#include "Render/RenderHelper.h"

#include <Base/BaseTypes.h>
#include <Logger/Logger.h>
#include <Math/AABBox3.h>
#include <Math/Vector.h>
#include <Render/Highlevel/Heightmap.h>
#include <Render/Highlevel/Landscape.h>

using namespace DAVA;

#define MINX -3
#define MAXX 3
#define CHUNK_SIZE 64

const float32 Terrain::CHUNK_SIZE_IN_METERS = 100.f;
const int32 Terrain::STRIDE_SIZE = (MAXX - MINX) * CHUNK_SIZE;

inline bool CollideSector(const WorldTriangle& triA, const WorldTriangle& triB, const DAVA::Vector3& start, DAVA::Vector3& outResult, DAVA::Vector3* normal);

Terrain::Terrain(Landscape* _land)
    : land(_land)
#ifdef TERRAIN_UNITTEST
    , space(MINX, MAXX, CHUNK_SIZE + 5)
#endif
{
    if (land)
    {
        heightMap.resize(STRIDE_SIZE * STRIDE_SIZE);
        AABBox3 box = land->GetBoundingBox();
        float32 realSize = (box.max.x - box.min.x) * 0.5f;

        sizeInChunks = (int32)ceil(realSize / CHUNK_SIZE_IN_METERS);
        newRealSize = sizeInChunks * CHUNK_SIZE_IN_METERS;

        //int16 nW = chunkSize + 5;
        //int16 nH = chunkSize + 5;
        int32 offset;
        int32 offsetHM;
        float32 min;
        float32 max;

        Vector3 point(0, 0, 0);
        Vector3 resultV;
        const float32 kf = CHUNK_SIZE_IN_METERS / (float32)CHUNK_SIZE;
        for (int32 yc = MINX; yc < MAXX; yc++)
        {
            for (int32 xc = MINX; xc < MAXX; xc++)
            {
#ifdef TERRAIN_UNITTEST
                Chunk& image32 = space.GetChunk(xc, yc);
#endif

                min = (float32)INT_MAX;
                max = (float32)-INT_MAX;
                offset = 0;

                point.y = (float32)(CHUNK_SIZE * yc - 3) * kf;
                for (int32 y = CHUNK_SIZE * yc - 2; y < CHUNK_SIZE * (yc + 1) + 3; ++y)
                {
                    point.y += kf;
                    point.x = (float32)(CHUNK_SIZE * xc - 3) * kf;
                    for (int32 x = CHUNK_SIZE * xc - 2; x < CHUNK_SIZE * (xc + 1) + 3; ++x)
                    {
                        point.x += kf;
                        resultV.z = 0.0f;

                        if (point.x < box.max.x && point.y < box.max.y)
                        {
                            PlaceLandscapePoint(point, resultV, NULL);
                        }
#ifdef TERRAIN_UNITTEST
                        image32[offset++] = Quantise(resultV.z);
#endif
                        if (x >= MINX * CHUNK_SIZE && x < MAXX * CHUNK_SIZE
                            && y >= MINX * CHUNK_SIZE && y < MAXX * CHUNK_SIZE)
                        {
                            offsetHM = (x - MINX * CHUNK_SIZE) + (y - MINX * CHUNK_SIZE) * STRIDE_SIZE;
                            heightMap[offsetHM] = Unquantise(Quantise(resultV.z));
                        }

                        if (resultV.z > max)
                            max = resultV.z;
                        if (resultV.z < min)
                            min = resultV.z;
                    }
                }
            }
        }
    }


#ifdef TERRAIN_UNITTEST
    Vector<float32> hmap;
    BuildSolidHM(hmap);
    DVASSERT(hmap.size() == heightMap.size());
    for (int32 i = 0; i < (int32)hmap.size(); i++)
    {
        DVASSERT(hmap[i] == heightMap[i]);
    }

    Vector3 res1, res2;
    Vector3 norm1, norm2;
    for (int32 i = 0; i < (int32)hmap.size(); i++)
    {
        Vector3 point;
        point.x = ((float32)(DAVA::Rand() % (MAXX * 4 * 100 * 100))) / 100.f - (float32)(MAXX * 2 * 100.f);
        point.y = ((float32)(DAVA::Rand() % (MAXX * 2 * 100 * 100))) / 100.f - (float32)(MAXX * 100.f);
        PlacePoint(point, res1, &norm1);
        OldPlacePoint(point, res2, &norm2);
        DVASSERT(res1.x == res2.x && res1.y == res2.y && fabsf(res1.z - res2.z) < 0.001);
        //DVASSERT(norm1 == norm2); we don't use normals at all
    }

#endif
}

Terrain::~Terrain()
{
    land = NULL;
}

#ifdef TERRAIN_UNITTEST
float32 Terrain::HeightAt(const Chunk& chunk, int32 x, int32 y)
{
    return Unquantise(chunk[(y + 2) * (CHUNK_SIZE + 5) + x + 2]);
}
#endif

bool Terrain::PlacePoint(const Vector3& point, Vector3& outResult, Vector3* outNormal)
{
    if (point.x >= MAXX * CHUNK_SIZE_IN_METERS - CHUNK_SIZE_IN_METERS / (float32)CHUNK_SIZE ||
        point.x < MINX * CHUNK_SIZE_IN_METERS ||
        point.y >= MAXX * CHUNK_SIZE_IN_METERS - CHUNK_SIZE_IN_METERS / (float32)CHUNK_SIZE ||
        point.y < MINX * CHUNK_SIZE_IN_METERS)
    {
        return false;
    }

    const float32 kW = (float32)CHUNK_SIZE / CHUNK_SIZE_IN_METERS;

    float32 x = point.x * kW;
    float32 y = point.y * kW;

    float32 x1 = floor(x);
    float32 y1 = floor(y);

    int32 gridX = (int32)x1;
    int32 gridY = (int32)y1;

    int32 offsetHM = (gridX - MINX * CHUNK_SIZE) + (gridY - MINX * CHUNK_SIZE) * STRIDE_SIZE;

    Vector3 bottomLeft(x1, y1, heightMap[offsetHM]);
    Vector3 bottomRight(x1 + 1.f, y1, heightMap[offsetHM + 1]);
    Vector3 topLeft(x1, y1 + 1.f, heightMap[offsetHM + STRIDE_SIZE]);
    Vector3 topRight(x1 + 1.f, y1 + 1.f, heightMap[offsetHM + STRIDE_SIZE + 1]);

    // Construct our triangles
    WorldTriangle triA;
    WorldTriangle triB;

    // Create the world triangles for the collision intersection tests
    if ((gridX ^ gridY) & 1)
    {
        triA = WorldTriangle(bottomLeft, topLeft, bottomRight,
                             TRIANGLE_TERRAIN);
        triB = WorldTriangle(topLeft, topRight, bottomRight,
                             TRIANGLE_TERRAIN);
    }
    else
    {
        triA = WorldTriangle(bottomLeft, topLeft, topRight,
                             TRIANGLE_TERRAIN);
        triB = WorldTriangle(topRight, bottomRight, bottomLeft,
                             TRIANGLE_TERRAIN);
    }

    outResult.x = point.x;
    outResult.y = point.y;

    Vector3 start(x, y, bottomLeft.z + 100.0f);
    return CollideSector(triA, triB, start, outResult, outNormal);
};

bool Terrain::PlaceLandscapePoint(const Vector3& point, Vector3& outResult, Vector3* outNormal)
{
    const AABBox3& bbox = land->GetBoundingBox();
    if (point.x > bbox.max.x ||
        point.x < bbox.min.x ||
        point.y > bbox.max.y ||
        point.y < bbox.min.y)
    {
        return false;
    }
    Heightmap* heightmap = land->GetHeightmap();
    if (heightmap->Data() == NULL)
    {
        Logger::Error("[Landscape::PlacePoint] Trying to place point on empty heightmap data!");
        return false;
    }

    uint16 x1 = uint16(floor(heightmap->Size() * (point.x - bbox.min.x) / Abs(bbox.max.x - bbox.min.x)));
    uint16 y1 = uint16(floor(heightmap->Size() * (point.y - bbox.min.y) / Abs(bbox.max.y - bbox.min.y)));

    Vector3 bottomLeft = heightmap->GetPoint(x1, y1, bbox);
    Vector3 topRight = heightmap->GetPoint(x1 + 1, y1 + 1, bbox);
    Vector3 bottomRight = heightmap->GetPoint(x1 + 1, y1, bbox);
    Vector3 topLeft = heightmap->GetPoint(x1, y1 + 1, bbox);

    // Construct our triangles
    WorldTriangle triA;
    WorldTriangle triB;

    triA = WorldTriangle(bottomLeft, topLeft, bottomRight,
                         TRIANGLE_TERRAIN);
    triB = WorldTriangle(topLeft, topRight, bottomRight,
                         TRIANGLE_TERRAIN);
    outResult.x = point.x;
    outResult.y = point.y;

    Vector3 start(point.x, point.y, bottomLeft.z + 100.0f);
    return CollideSector(triA, triB, start, outResult, outNormal);
};

bool CollideSector(const WorldTriangle& triA, const WorldTriangle& triB, const Vector3& start, Vector3& outResult, Vector3* outNormal)
{
    Vector3 dir(0.f, 0.f, -1.f);
    float dist = 200.f;
    float dist2 = 200.f;

    // Check if we intersect with either triangle
    bool intersectsA = triA.intersects(start, dir, dist);
    bool intersectsB = triB.intersects(start, dir, dist2);

    // If we have a callback pass colliding triangles to it
    // We always pass in triangles from closest to furthest away
    // so that we can stop colliding if the callback only wants
    // near collisions.

    WorldTriangle resTr;
    float32 resDist = 0.f;

    if (intersectsA && intersectsB)
    {
        if (dist < dist2)
        {
            resTr = triA;
            resDist = dist;
        }
        else
        {
            resTr = triB;
            resDist = dist2;
        }
    }
    else if (intersectsA)
    {
        resTr = triA;
        resDist = dist;
    }
    else if (intersectsB)
    {
        resTr = triB;
        resDist = dist2;
    }

    outResult.z = (start + dir * resDist).z;
    if (outNormal != NULL)
    {
        *outNormal = resTr.normal();
        outNormal->Normalize();
    }
    return true;
}


#ifdef TERRAIN_UNITTEST
bool Terrain::OldPlacePoint(const Vector3& point, Vector3& result, Vector3* normal)
{
    if (point.x >= MAXX * CHUNK_SIZE_IN_METERS - CHUNK_SIZE_IN_METERS / (float32)CHUNK_SIZE ||
        point.x < MINX * CHUNK_SIZE_IN_METERS ||
        point.y >= MAXX * CHUNK_SIZE_IN_METERS - CHUNK_SIZE_IN_METERS / (float32)CHUNK_SIZE ||
        point.y < MINX * CHUNK_SIZE_IN_METERS)
    {
        return false;
    }

    int32 chunkx = (int32)floor(point.x / CHUNK_SIZE_IN_METERS);
    int32 chunky = (int32)floor(point.y / CHUNK_SIZE_IN_METERS);

    Chunk& chunk = space.GetChunk(chunkx, chunky);

    float32 inx = point.x - (float32)chunkx * CHUNK_SIZE_IN_METERS;
    float32 iny = point.y - (float32)chunky * CHUNK_SIZE_IN_METERS;

    float32 kW = (float32)CHUNK_SIZE / CHUNK_SIZE_IN_METERS;

    float32 x = inx * kW;
    float32 y = iny * kW;

    float32 x1 = floor(x);
    float32 y1 = floor(y);

    //todo just ADD 1
    float32 x2 = ceil(x);
    float32 y2 = ceil(y);

    if (x1 == x2)
        x2 += 1.0f;

    if (y1 == y2)
        y2 += 1.0f;

    Vector3 bottomLeft(x1, y1, HeightAt(chunk, (int32)x1, (int32)y1));
    Vector3 bottomRight(x2, y1, HeightAt(chunk, (int32)x2, (int32)y1));
    Vector3 topLeft(x1, y2, HeightAt(chunk, (int32)x1, (int32)y2));
    Vector3 topRight(x2, y2, HeightAt(chunk, (int32)x2, (int32)y2));

    int32 gridX = (int32)x1;
    int32 gridZ = (int32)y1;

    // Construct our triangles
    WorldTriangle triA;
    WorldTriangle triB;

    result.x = point.x;
    result.y = point.y;

    Vector3 start(x, y, bottomLeft.z + 100.0f);
    return CollideSector(triA, triB, start, result, normal);
};
#endif

DAVA::Vector<DAVA::float32>& Terrain::GetHeightMap()
{
    return heightMap;
}

DAVA::int32 Terrain::GetHeightMapSize()
{
    return STRIDE_SIZE;
}

#ifdef TERRAIN_UNITTEST
int32 Terrain::BuildSolidHM(Vector<float32>& hmap)
{
    int32 sz = CHUNK_SIZE * 6;
    hmap.resize(sz * sz);

    int32 offs = 0;
    int32 ind = 0;
    for (int32 chunky = -3; chunky < 3; chunky++)
    {
        for (int32 chunkx = -3; chunkx < 3; chunkx++)
        {
            Chunk& chunk = space.GetChunk(chunkx, chunky);
            for (int32 y = 0; y < CHUNK_SIZE; y++)
            {
                offs = ((chunky + 3) * CHUNK_SIZE + y) * sz + (chunkx + 3) * CHUNK_SIZE;
                ind = (y + 2) * (CHUNK_SIZE + 5) + 2;
                for (int32 x = 0; x < CHUNK_SIZE; x++)
                {
                    hmap[offs++] = Unquantise(chunk[ind++]);
                }
            }
        }
    }
    return sz;
}
#endif

AABBox3 Terrain::GetBBox()
{
    return land->GetBoundingBox();
}

#ifdef TERRAIN_UNITTEST
void Terrain::Draw(RenderHelper* drawer)
{
    float32 kW = 100.0f / (float32)CHUNK_SIZE;

    for (int32 chunky = -3; chunky < 3; chunky++)
        for (int32 chunkx = -3; chunkx < 3; chunkx++)
        {
            float32 iny = (float32)chunky * 100.0f;
            float32 inx = (float32)chunkx * 100.0f;

            Chunk& chunk = space.GetChunk(chunkx, chunky);
            for (int32 y = 0; y < CHUNK_SIZE; y++)
            {
                inx = (float32)chunkx * 100.0f;
                for (int32 x = 0; x < CHUNK_SIZE; x++)
                {
                    Vector3 bottomLeft(inx, iny, HeightAt(chunk, x, y));
                    Vector3 bottomRight(inx + kW, iny, HeightAt(chunk, x + 1, y));
                    Vector3 topLeft(inx, iny + kW, HeightAt(chunk, x, y + 1));
                    Vector3 topRight(inx + kW, iny + kW, HeightAt(chunk, x + 1, y + 1));

                    drawer->DrawLine(bottomLeft, topLeft, Color::White);
                    drawer->DrawLine(topLeft, topRight, Color::White);

                    if ((x ^ y) & 1)
                    {
                        drawer->DrawLine(topLeft, bottomRight, Color::White);

                        //						triA = WorldTriangle( bottomLeft, topLeft, bottomRight,
                        //											 TRIANGLE_TERRAIN );
                        //						triB = WorldTriangle( topLeft, topRight, bottomRight,
                        //											 TRIANGLE_TERRAIN );
                    }
                    else
                    {
                        drawer->DrawLine(bottomLeft, topRight, Color::White);
                        //						triA = WorldTriangle( bottomLeft, topLeft, topRight,
                        //											 TRIANGLE_TERRAIN );
                        //						triB = WorldTriangle( topRight, bottomRight, bottomLeft,
                        //											 TRIANGLE_TERRAIN );
                    }

                    inx += kW;
                }
                iny += kW;
            }
        }
}
#endif
