#include "SpeedTreeConverter.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#define LEAF_BASE_ANGLE_DIFFERENCE_FACTOR 30
#define LEAF_AMPLITUDE_USERFRIENDLY_FACTOR 0.01f
#define TRUNK_AMPLITUDE_USERFRIENDLY_FACTOR 0.1f

namespace DAVA
{
const FastName SpeedTreeConverter::SPEED_TREE_MATERIAL_NAME_OLD("~res:/Materials/SpeedTreeLeaf.material");
const FastName SpeedTreeConverter::SPEED_TREE_SPERICAL_LIT_MATERIAL_NAME_OLD("~res:/Materials/SphericalLitAllQualities.SpeedTreeLeaf.material");

void SpeedTreeConverter::CalculateAnimationParams(SpeedTreeObject* object)
{
    float32 treeHeight = object->GetBoundingBox().GetSize().z;

    uint32 size = object->GetRenderBatchCount();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* rb = object->GetRenderBatch(k);
        PolygonGroup* pg = rb->GetPolygonGroup();
        if (nullptr != pg)
        {
            int32 vertexFormat = pg->GetFormat();
            DVASSERT((vertexFormat & EVF_FLEXIBILITY) > 0);

            int32 vxCount = pg->GetVertexCount();
            for (int32 i = 0; i < vxCount; ++i)
            {
                float32 flexebility = CalculateVertexFlexibility(pg, i, treeHeight);
                pg->SetFlexibility(i, flexebility);

                if ((pg->GetFormat() & EVF_ANGLE_SIN_COS) > 0)
                {
                    Vector2 leafAngle = CalculateVertexAngle(pg, i, treeHeight);
                    pg->SetAngle(i, leafAngle);
                }
            }

            pg->BuildBuffers();
        }
    }
}

float32 SpeedTreeConverter::CalculateVertexFlexibility(PolygonGroup* pg, int32 vi, float32 treeHeight)
{
    Vector3 vxPosition;
    pg->GetCoord(vi, vxPosition);
    float32 t0 = vxPosition.Length() * LEAF_BASE_ANGLE_DIFFERENCE_FACTOR;

    float32 x = vxPosition.z / treeHeight;
    float32 flexebility = std::log((std::exp(1.0f) - 1) * x + 1);

    return flexebility * TRUNK_AMPLITUDE_USERFRIENDLY_FACTOR;
}

Vector2 SpeedTreeConverter::CalculateVertexAngle(PolygonGroup* pg, int32 vi, float32 treeHeight)
{
    Vector3 vxPosition;
    pg->GetCoord(vi, vxPosition);

    float32 t0 = vxPosition.Length() * LEAF_BASE_ANGLE_DIFFERENCE_FACTOR;
    float32 x = vxPosition.z / treeHeight;

    float32 leafHeightOscillationCoeff = (.5f + x / 2);
    //leafAngle: x: cos(T0);  y: sin(T0)
    Vector2 leafAngle(std::cos(t0) * leafHeightOscillationCoeff * LEAF_AMPLITUDE_USERFRIENDLY_FACTOR,
                      std::sin(t0) * leafHeightOscillationCoeff * LEAF_AMPLITUDE_USERFRIENDLY_FACTOR);

    return leafAngle;
}

bool SpeedTreeConverter::IsTreeLeafBatch(RenderBatch* batch)
{
    if (batch && batch->GetMaterial())
    {
        bool isLeaf = false;
        NMaterial* material = batch->GetMaterial();

        const FastName& fxName = material->GetEffectiveFXName();

        isLeaf |= (fxName == SPEED_TREE_MATERIAL_NAME_OLD);
        isLeaf |= (fxName == SPEED_TREE_SPERICAL_LIT_MATERIAL_NAME_OLD);
        isLeaf |= (fxName == NMaterialName::FORWARD_PBS_TRANSMITTANCE) && (material->GetEffectiveFlagValue(FastName("SPEED_TREE_LEAVES")) != 0);
        isLeaf |= (fxName == NMaterialName::FORWARD_PBS_TRANSMITTANCE_BILLBOARDS) && (material->GetEffectiveFlagValue(FastName("SPEED_TREE_BILLBOARD_LEAVES")) != 0);

        return isLeaf;
    }

    return false;
}

bool SpeedTreeConverter::IsBillboardBatch(RenderBatch* batch)
{
    if (batch && batch->GetMaterial())
    {
        NMaterial* material = batch->GetMaterial();
        const FastName& fxName = material->GetEffectiveFXName();
        return (fxName == NMaterialName::FORWARD_PBS_TRANSMITTANCE_BILLBOARDS) && (material->GetEffectiveFlagValue(FastName("SPEED_TREE_BILLBOARD_LEAVES")) != 0);
    }

    return false;
}

void SpeedTreeConverter::ConvertTrees(Entity* scene)
{
    uniqLeafPGs.clear();
    uniqTrunkPGs.clear();
    uniqTreeObjects.clear();

    ConvertingPathRecursive(scene);

    Set<PolygonGroup*>::iterator leafPGIt = uniqLeafPGs.begin();
    for (; leafPGIt != uniqLeafPGs.end(); ++leafPGIt)
        ConvertLeafPGForAnimations((*leafPGIt));

    Set<PolygonGroup*>::iterator pgTrunkPGIt = uniqTrunkPGs.begin();
    for (; pgTrunkPGIt != uniqTrunkPGs.end(); ++pgTrunkPGIt)
        ConvertTrunkForAnimations((*pgTrunkPGIt));

    Set<SpeedTreeObject*>::iterator treeIt = uniqTreeObjects.begin();
    for (; treeIt != uniqTreeObjects.end(); ++treeIt)
    {
        (*treeIt)->RecalcBoundingBox();
        CalculateAnimationParams((*treeIt));
    }
}

void SpeedTreeConverter::ConvertingPathRecursive(Entity* node)
{
    for (int32 c = 0; c < node->GetChildrenCount(); ++c)
    {
        Entity* childNode = node->GetChild(c);
        ConvertingPathRecursive(childNode);
    }

    RenderComponent* rc = GetRenderComponent(node);
    if (nullptr == rc)
    {
        return;
    }

    RenderObject* ro = rc->GetRenderObject();
    if (nullptr == ro)
    {
        return;
    }

    if (ro->GetType() != RenderObject::TYPE_SPEED_TREE)
    {
        return;
    }

    SpeedTreeObject* treeObject = CastIfEqual<SpeedTreeObject*>(ro);
    if (nullptr == treeObject)
    {
        treeObject = new SpeedTreeObject();
        ro->Clone(treeObject);
        rc->SetRenderObject(treeObject);
        treeObject->Release();
    }

    if (GetSpeedTreeComponent(node) == nullptr)
    {
        node->AddComponent(new SpeedTreeComponent());
    }

    uniqTreeObjects.insert(treeObject);

    uint32 size = treeObject->GetRenderBatchCount();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* rb = treeObject->GetRenderBatch(k);
        PolygonGroup* pg = rb->GetPolygonGroup();
        if (IsTreeLeafBatch(rb))
        {
            uniqLeafPGs.insert(pg);

            if (IsBillboardBatch(rb))
                uniqBillboardLeafPGs.insert(pg);
        }
        else
        {
            uniqTrunkPGs.insert(pg);
        }
        uniqPGs.insert(pg);

        NMaterial* mat = rb->GetMaterial();
        while (mat)
        {
            materials.insert(mat);
            mat = mat->GetParent();
        }
    }
}

void SpeedTreeConverter::ConvertLeafPGForAnimations(PolygonGroup* pg)
{
    int32 vertexFormat = pg->GetFormat();
    int32 vxCount = pg->GetVertexCount();
    int32 indCount = pg->GetIndexCount();

    int32 oldLeafFormat = (EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_TANGENT);
    DVASSERT((vertexFormat & oldLeafFormat) == oldLeafFormat); //old tree leaf vertex format

    PolygonGroup* pgCopy = new PolygonGroup();
    pgCopy->AllocateData(vertexFormat, vxCount, indCount);

    Memcpy(pgCopy->meshData, pg->meshData, vxCount * pg->vertexStride);
    Memcpy(pgCopy->indexArray, pg->indexArray, indCount * sizeof(int16));

    pg->ReleaseData();
    pg->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_PIVOT_DEPRECATED | EVF_FLEXIBILITY | EVF_ANGLE_SIN_COS, vxCount, indCount);

    //copy indices
    for (int32 i = 0; i < indCount; ++i)
    {
        int32 index;
        pgCopy->GetIndex(i, index);
        pg->SetIndex(i, index);
    }

    //copy vertex data
    for (int32 i = 0; i < vxCount; ++i)
    {
        Vector3 vxPosition;
        uint32 color;
        Vector2 vxTx;
        Vector3 vxPivot;

        pgCopy->GetCoord(i, vxPosition);
        pgCopy->GetColor(i, color);
        pgCopy->GetTexcoord(0, i, vxTx);
        pgCopy->GetTangent(i, vxPivot);

        pg->SetCoord(i, vxPosition);
        pg->SetColor(i, color);
        pg->SetTexcoord(0, i, vxTx);
        pg->SetPivotDeprecated(i, vxPivot);
    }
    SafeRelease(pgCopy);

    pg->BuildBuffers();
}

void SpeedTreeConverter::ConvertTrunkForAnimations(PolygonGroup* pg)
{
    int32 vertexFormat = pg->GetFormat();
    int32 vxCount = pg->GetVertexCount();
    int32 indCount = pg->GetIndexCount();

    int32 oldTrunkFormat = (EVF_VERTEX | EVF_TEXCOORD0);
    DVASSERT((vertexFormat & oldTrunkFormat) == oldTrunkFormat); //old tree trunk vertex format

    PolygonGroup* pgCopy = new PolygonGroup();
    pgCopy->AllocateData(vertexFormat, vxCount, indCount);

    Memcpy(pgCopy->meshData, pg->meshData, vxCount * pg->vertexStride);
    Memcpy(pgCopy->indexArray, pg->indexArray, indCount * sizeof(int16));

    pg->ReleaseData();
    pg->AllocateData(EVF_VERTEX | EVF_TEXCOORD0 | EVF_FLEXIBILITY, vxCount, indCount);

    //copy indices
    for (int32 i = 0; i < indCount; ++i)
    {
        int32 index;
        pgCopy->GetIndex(i, index);
        pg->SetIndex(i, index);
    }

    //copy vertex data
    for (int32 i = 0; i < vxCount; ++i)
    {
        Vector3 vxPosition;
        Vector2 vxTx;

        pgCopy->GetCoord(i, vxPosition);
        pgCopy->GetTexcoord(0, i, vxTx);

        pg->SetCoord(i, vxPosition);
        pg->SetTexcoord(0, i, vxTx);
    }
    SafeRelease(pgCopy);

    pg->BuildBuffers();
}

void SpeedTreeConverter::ConvertPolygonGroupsPivot3(Entity* scene)
{
    uniqPGs.clear();
    uniqBillboardLeafPGs.clear();
    uniqTreeObjects.clear();
    materials.clear();

    ConvertingPathRecursive(scene);

    Map<PolygonGroup*, PolygonGroup*> pgCopy;
    for (PolygonGroup* dataSource : uniqPGs)
    {
        int32 vertexFormat = dataSource->GetFormat();
        int32 vxCount = dataSource->GetVertexCount();
        int32 indCount = dataSource->GetIndexCount();
        int32 vertexSize = GetVertexSize(vertexFormat);

        PolygonGroup* pg = new PolygonGroup();
        pg->AllocateData(vertexFormat, vxCount, indCount);
        memcpy(pg->meshData, dataSource->meshData, vertexSize * vxCount);
        memcpy(pg->indexArray, dataSource->indexArray, indCount * sizeof(uint16));

        pgCopy[dataSource] = pg;
    }

    for (PolygonGroup* pg : uniqPGs)
    {
        PolygonGroup* dataSource = pgCopy[pg];

        bool isBillboard = (uniqBillboardLeafPGs.count(pg) != 0);
        dataSource->UpdateDataPointersAndStreamsLegacy();

        int32 vertexFormat = dataSource->GetFormat();
        int32 vxCount = dataSource->GetVertexCount();
        int32 indCount = dataSource->GetIndexCount();

        int32 convertedFormat = (vertexFormat & ~EVF_PIVOT_DEPRECATED) | EVF_PIVOT4;

        pg->ReleaseGeometryData();
        pg->AllocateData(convertedFormat, vxCount, indCount);

        Memcpy(pg->indexArray, dataSource->indexArray, indCount * sizeof(int16));

        uint8* dst = pg->meshData;
        const uint8* src = dataSource->meshData;
        for (int32 i = 0; i < vxCount; ++i)
        {
            float32 vec1;
            Vector2 vec2;
            Vector3 vec3;
            uint32 color;
            int32 jIndex;

            //GFX_COMPLETE: remove copying using Get/Set and remove UpdateDataPointersAndStreamsLegacy() after new_render content resaving
            //
            //We not use PolygonGroups::CopyData(), because legacy geometry have different meshData mapping for streams
            //Using UpdateDataPointersAndStreamsLegacy() we set pointers in correct valuse than use Get/Set for copy streams data
            if (vertexFormat & EVF_VERTEX)
            {
                dataSource->GetCoord(i, vec3);
                pg->SetCoord(i, vec3);
            }
            if (vertexFormat & EVF_NORMAL)
            {
                dataSource->GetNormal(i, vec3);
                pg->SetNormal(i, vec3);
            }
            if (vertexFormat & EVF_COLOR)
            {
                dataSource->GetColor(i, color);
                pg->SetColor(i, color);
            }
            if (vertexFormat & EVF_TEXCOORD0)
            {
                dataSource->GetTexcoord(0, i, vec2);
                pg->SetTexcoord(0, i, vec2);
            }
            if (vertexFormat & EVF_TEXCOORD1)
            {
                dataSource->GetTexcoord(1, i, vec2);
                pg->SetTexcoord(1, i, vec2);
            }
            if (vertexFormat & EVF_TANGENT)
            {
                dataSource->GetTangent(i, vec3);
                pg->SetTangent(i, vec3);
            }
            if (vertexFormat & EVF_BINORMAL)
            {
                dataSource->GetBinormal(i, vec3);
                pg->SetBinormal(i, vec3);
            }
            if (vertexFormat & EVF_FLEXIBILITY)
            {
                dataSource->GetFlexibility(i, vec1);
                pg->SetFlexibility(i, vec1);
            }
            if (vertexFormat & EVF_ANGLE_SIN_COS)
            {
                dataSource->GetAngle(i, vec2);
                pg->SetAngle(i, vec2);
            }
            if (vertexFormat & EVF_JOINTINDEX)
            {
                dataSource->GetJointIndex(i, 0, jIndex);
                pg->SetJointIndex(i, 0, jIndex);
            }
            if (vertexFormat & EVF_JOINTWEIGHT)
            {
                dataSource->GetJointWeight(i, 0, vec1);
                pg->SetJointWeight(i, 0, vec1);
            }
            if (vertexFormat & EVF_PIVOT_DEPRECATED)
            {
                float32 w = isBillboard ? 1.f : 0.f;
                dataSource->GetPivotDeprecated(i, vec3);
                pg->SetPivot(i, Vector4(vec3, w));
            }
            else
            {
                dataSource->GetCoord(i, vec3);
                pg->SetPivot(i, Vector4(vec3, 0.f));
            }
        }

        pg->RecalcAABBox();
        pg->BuildBuffers();
    }

    static const FastName FLAG_SPEED_TREE_LEAF("SPEED_TREE_LEAF");
    for (NMaterial* material : materials)
    {
        if (material->HasLocalFlag(FLAG_SPEED_TREE_LEAF))
        {
            material->AddFlag(NMaterialFlagName::FLAG_SPEED_TREE_OBJECT, material->GetLocalFlagValue(FLAG_SPEED_TREE_LEAF));
            material->RemoveFlag(FLAG_SPEED_TREE_LEAF);
        }

        if (material->HasLocalFXName())
        {
            if (material->GetLocalFXName() == SPEED_TREE_MATERIAL_NAME_OLD)
                material->SetFXName(NMaterialName::SPEEDTREE_ALPHATEST);

            if (material->GetLocalFXName() == SPEED_TREE_SPERICAL_LIT_MATERIAL_NAME_OLD)
                material->SetFXName(NMaterialName::SPHERICLIT_SPEEDTREE_ALPHATEST);

            if ((material->GetLocalFXName() == NMaterialName::FORWARD_PBS_TRANSMITTANCE) || (material->GetLocalFXName() == NMaterialName::FORWARD_PBS_TRANSMITTANCE_BILLBOARDS))
                material->SetFXName(NMaterialName::FORWARD_PBS_SPEEDTREE_TRANSMITTANCE);
        }
    }

    for (auto& it : pgCopy)
        SafeRelease(it.second);
}

void SpeedTreeConverter::ValidateSpeedTreeComponentCount(Entity* node)
{
    for (int32 c = 0; c < node->GetChildrenCount(); ++c)
    {
        Entity* childNode = node->GetChild(c);
        ValidateSpeedTreeComponentCount(childNode);
    }

    while (node->GetComponentCount<SpeedTreeComponent>() > 1u)
    {
        node->RemoveComponent<SpeedTreeComponent>(1u);
    };
}
};
