#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastResource.h"
#include "Beast/BeastManager.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class RenderBatch;
class PolygonGroup;
}

namespace Beast
{
class LandscapeGeometry;
class BeastMesh : public BeastResource<BeastMesh>
{
public:
    void InitWithMeshInstancePart(DAVA::RenderBatch* batch, DAVA::int32 partIndex);
    void InitWithSpeedTreeLeaf(DAVA::RenderBatch* batch, DAVA::int32 partIndex, const DAVA::Matrix4& leafTransform);

    void InitWithLandscape(LandscapeGeometry* geometry, BeastMaterial* material);

    ILBMeshHandle GetILBMesh() const;
    DAVA::int32 GetTextureCoordCount() const;

private:
    friend class BeastResource<BeastMesh>;
    BeastMesh(const DAVA::String& name, BeastManager* manager);

    bool CheckVertexFormat(DAVA::PolygonGroup* polygonGroup);
    bool HasTangents(DAVA::PolygonGroup* polygonGroup);

    void AddVertices(DAVA::PolygonGroup* polygonGroup);
    void AddTranfromedVertices(DAVA::PolygonGroup* polygonGroup, const DAVA::Matrix4&);
    void AddIndices(DAVA::PolygonGroup* polygonGroup, DAVA::NMaterial* material);
    void AddUV(DAVA::PolygonGroup* polygonGroup, DAVA::int32 textureCoordIndex);
    void AddVertices(LandscapeGeometry* geometry);
    void AddTangents(DAVA::PolygonGroup* polygonGroup);
    void AddIndices(LandscapeGeometry* geometry, BeastMaterial* material);
    void AddUV(LandscapeGeometry* geometry);

private:
    ILBMeshHandle mesh = nullptr;
    DAVA::int32 indecesCount = 0;
    DAVA::int32 textureCoordCount = 0;
};
}