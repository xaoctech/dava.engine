/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
 * Created by Yury Danilov
   =====================================================================================*/
#pragma once

#include "DAVAEngine.h"
#include "physics2/p2conf.hpp"
#include "physics2/worldtri.hpp"
#include "physics2/bsp.hpp"

namespace DAVA
{
class EditorConfig;
} // namespace DAVA

enum eModelPreset
{
    eMP_NO_COLLISION = 0,
    eMP_TREE,
    eMP_BUSH,
    eMP_FRAGILE_PR,
    eMP_FRAGILE_NPR,
    eMP_FALLING_ATOM,
    eMP_BUILDING,
    eMP_INVISIBLE_WALL,
    eMP_SPEED_TREE,
    eMP_WATER
};

enum eFallType
{
    eFT_TREE = 0,
    eFT_PALMA,
    eFT_STOLB,
    eFT_BUSH
};

enum eSpawnPreferredVehicleType
{
    eSPVT_ANY = 0,
    eSPVT_LIGHT_TANK,
    eSPVT_MEDIUM_TANK,
    eSPVT_HEAVY_TANK,
    eSPVT_AT_SPG,

    eSPVT_COUNT
};

#define DEFAULT_DENSITY 0.9f

struct VertexHeader
{
    char vertexFormat_[64];
    int nVertices_;
};

struct VertexXYZNUV
{
    DAVA::Vector3 pos_;
    DAVA::Vector3 normal_;
    DAVA::Vector2 uv_;
};

struct IndexHeader
{
    char indexFormat_[64];
    int nIndices_;
    int nTriangleGroups_;
};

struct PrimitiveGroup
{
    int startIndex_;
    int nPrimitives_;
    int startVertex_;
    int nVertices_;
};

struct MaterialPrimitiveGroup
{
    DAVA::String material;
    DAVA::String identifier;
    bool twoSided;
    bool hasAlpha;
    PrimitiveGroup group;
    eModelPreset matCollision;
};

typedef DAVA::uint16 IndexValue;

class Model
: public DAVA::BaseObject
{
public:
    Model(const DAVA::String& fileName, const DAVA::String& mapName, eModelPreset _collision, DAVA::Entity* model, DAVA::EditorConfig* config);

    std::vector<DataSection*> children_;
    const DAVA::String& GetModelName(void)
    {
        return modelName;
    };

    static const DAVA::Matrix4 globalTransform;
    static const DAVA::Matrix4 globalTransformInverse;

    inline bool IsWrong()
    {
        return isWrongModel;
    };

    inline eModelPreset GetModelPreset(void)
    {
        return collision;
    };
    static DAVA::String GetPhysicsParams(DAVA::int32 fallType_);

private:
    void RecursiveAddGeometry(DAVA::Entity* curr, DAVA::int32 switchIndex);
    void AddGeometry(DAVA::Entity* entity, DAVA::RenderObject* renderObject, DAVA::int32 requestedLodIndex = -1,
                     DAVA::int32 requestedSwitchIndex = -1);
    void Batch(DAVA::Entity* curr);
    void Save();
    void Build(void);
    void createTris(void);
    void CreatePolygonGroup();
    BinaryPtr Recollect(void);
    void PrepareDestructible();
    void ReadCustomProperties(DAVA::Entity* model, DAVA::EditorConfig* config);

    DAVA::int32 FindMaxLodIndex(DAVA::RenderObject* object, DAVA::int32 switchIndex);
    DAVA::uint32 GetSwitchCount(DAVA::RenderObject* object);

    DAVA::Vector<VertexXYZNUV> vertices;
    DAVA::Vector<IndexValue> primitive;
    DAVA::int32 indexOffset = 0;
    DAVA::Vector<MaterialPrimitiveGroup> groups;
    RealWTriangleSet tris;
    BSPTree* pBSP;
    DAVA::int32 degenerateCount;
    BoundingBox bbox;
    DAVA::String modelName;
    eModelPreset collision;
    bool isWrongModel = false;
    //	bool isDestr;
    DAVA::int32 health = 5;
    eFallType fallType;
    DAVA::float32 density = DEFAULT_DENSITY;
    DAVA::int32 GetBWMaterialKind(DAVA::int32 primitiveGroup);
    DAVA::int32 GetBWCollision(DAVA::int32 primitiveGroup);

    DAVA::String mapName;
    DAVA::String materialKind;
};
