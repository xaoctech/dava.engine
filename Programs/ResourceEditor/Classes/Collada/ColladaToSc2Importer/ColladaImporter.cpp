#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneUtils.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Collada/ColladaMeshInstance.h"
#include "Collada/ColladaSceneNode.h"
#include "Collada/ColladaScene.h"
#include "Collada/ColladaToSc2Importer/ColladaImporter.h"
#include "Collada/ColladaToSc2Importer/ImportSettings.h"
#include "Utils/UTF8Utils.h"
#include "Qt/Main/QtUtils.h"

namespace DAVA
{
ColladaImporter::ColladaImporter()
{
}

// Creates Dava::RenderObject from ColladaMeshInstance and puts it
RenderObject* ColladaImporter::GetMeshFromCollada(ColladaMeshInstance* mesh, const FastName& name)
{
    ColladaSkinnedMesh* colladaSkinnedMesh = mesh->GetSkinnedMesh();

    RenderObject* ro = nullptr;
    if (colladaSkinnedMesh != nullptr)
    {
        ro = new SkinnedMesh();
    }
    else
    {
        ro = new Mesh();
    }

    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        PolygonGroup* davaPolygon = library.GetOrCreatePolygon(polygonGroupInstance);

        if (colladaSkinnedMesh != nullptr)
        {
            davaPolygon->ApplyMatrix(colladaSkinnedMesh->bindShapeMatrix);
        }

        bool isShadow = (strstr(name.c_str(), ImportSettings::shadowNamePattern.c_str()) != nullptr);
        if (isShadow)
        {
            davaPolygon = DAVA::MeshUtils::CreateShadowPolygonGroup(davaPolygon);
        }

        ScopedPtr<NMaterial> davaMaterial(library.CreateMaterialInstance(polygonGroupInstance, isShadow));
        ScopedPtr<RenderBatch> davaBatch(new RenderBatch());

        davaBatch->SetPolygonGroup(davaPolygon);
        davaBatch->SetMaterial(davaMaterial);
        ro->AddRenderBatch(davaBatch);
    }
    // TO VERIFY?
    DVASSERT(0 < ro->GetRenderBatchCount() && "Empty mesh");
    return ro;
}

bool ColladaImporter::VerifyColladaMesh(ColladaMeshInstance* mesh, const FastName& nodeName)
{
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        if (polygonGroupInstance->material == nullptr)
        {
            Logger::Error("[DAE to SC2] Node %s has no material", nodeName.c_str());
            return false;
        }

        auto polyGroup = polygonGroupInstance->polyGroup;
        if ((polyGroup == nullptr) || polyGroup->GetVertices().empty())
        {
            Logger::Error("[DAE to SC2] Node %s has no geometric data", nodeName.c_str());
            return false;
        }
    }

    return true;
}

eColladaErrorCodes ColladaImporter::VerifyDavaMesh(RenderObject* mesh, const FastName name)
{
    eColladaErrorCodes retValue = eColladaErrorCodes::COLLADA_OK;

    uint32 batchesCount = mesh->GetRenderBatchCount();
    if (0 == batchesCount)
    {
        Logger::Error("[DAE to SC2] %s has no render batches.", name.c_str());
        retValue = eColladaErrorCodes::COLLADA_ERROR;
    }
    else
    {
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            auto batch = mesh->GetRenderBatch(i);
            if (nullptr == batch)
            {
                Logger::Error("[DAE to SC2] Node %s has no %i render batch", i);
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            auto polygon = batch->GetPolygonGroup();
            if (nullptr == polygon)
            {
                Logger::Error("[DAE to SC2] Node %s has no polygon in render batch %i ", i);
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            if (0 >= polygon->GetVertexCount())
            {
                Logger::Error("[DAE to SC2] Node %s has no geometric data", name.c_str());
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }
        }
    }

    return retValue;
}

eColladaErrorCodes ColladaImporter::ImportMeshes(const Vector<ColladaMeshInstance*>& meshInstances, Entity* node)
{
    eColladaErrorCodes retValue = eColladaErrorCodes::COLLADA_OK;

    DVASSERT(meshInstances.size() <= 1 && "Should be only one meshInstance in one collada node");
    for (auto meshInstance : meshInstances)
    {
        if (VerifyColladaMesh(meshInstance, node->GetName()))
        {
            ScopedPtr<RenderObject> renderObject(GetMeshFromCollada(meshInstance, node->GetName()));
            RenderComponent* davaRenderComponent = GetRenderComponent(node);
            if (nullptr == davaRenderComponent)
            {
                davaRenderComponent = new RenderComponent();
                node->AddComponent(davaRenderComponent);
            }
            davaRenderComponent->SetRenderObject(renderObject);

            // Verification!
            eColladaErrorCodes iterationRet = VerifyDavaMesh(renderObject.get(), node->GetName());
            retValue = Max(iterationRet, retValue);
        }
        else
        {
            retValue = eColladaErrorCodes::COLLADA_ERROR;
        }
    }

    return retValue;
}

void ColladaImporter::ImportAnimation(ColladaSceneNode* colladaNode, Entity* nodeEntity)
{
    if (nullptr != colladaNode->animation)
    {
        auto* animationComponent = new AnimationComponent();
        animationComponent->SetEntity(nodeEntity);
        nodeEntity->AddComponent(animationComponent);

        // Calculate actual transform and bake it into animation keys.
        // NOTE: for now usage of the same animation more than once is bad idea
        // NOTE: animation->BakeTransform(totalTransform); actually was removed because of poor rotation behavior
        // when pose baked into keys. So just set correct invPose to animation instead.
        AnimationData* animation = library.GetOrCreateAnimation(colladaNode->animation);
        Matrix4 totalTransform = colladaNode->AccumulateTransformUptoFarParent(colladaNode->scene->rootNode);

        Matrix4 invPose;
        totalTransform.GetInverse(invPose);
        animation->SetInvPose(invPose);
        animationComponent->SetAnimation(animation);
    }
}

void ColladaImporter::ImportSkeleton(ColladaSceneNode* colladaNode, Entity* node)
{
    DVASSERT(colladaNode->meshInstances.size() <= 1);

    ColladaSkinnedMesh* skinnedMesh = colladaNode->meshInstances.empty() ? nullptr : colladaNode->meshInstances[0]->GetSkinnedMesh();
    if (skinnedMesh == nullptr)
        return;

    SkeletonComponent* davaSkeletonComponent = new SkeletonComponent();
    node->AddComponent(davaSkeletonComponent);

    int32 jointsCount = int32(skinnedMesh->joints.size());
    Vector<SkeletonComponent::JointConfig> jointConfigs(jointsCount);
    for (int32 i = 0; i < jointsCount; ++i)
    {
        const ColladaSkinnedMesh::Joint& colladaJoint = skinnedMesh->joints[i];
        const ColladaSceneNode* jointNode = colladaJoint.node;
        SkeletonComponent::JointConfig& jointConfig = jointConfigs[i];

        DVASSERT(colladaJoint.parentIndex < colladaJoint.index);

        bool isRootJoint = (colladaJoint.parentIndex == -1);

        jointConfig.targetId = colladaJoint.index;
        jointConfig.parentIndex = isRootJoint ? SkeletonComponent::INVALID_JOINT_INDEX : colladaJoint.parentIndex;
        jointConfig.name = FastName(colladaJoint.jointName);
        jointConfig.uid = FastName(colladaJoint.jointUID);

        Matrix4 transform = isRootJoint ? jointNode->AccumulateTransformUptoFarParent(colladaNode->scene->rootNode) : jointNode->localTransform;
        Vector3 scale, translation;
        Quaternion orientation;
        transform.Decomposition(translation, scale, orientation);

        //TODO: *Skinning* remove local transforms from JointConfig. Keep bindTransformInv only
        jointConfig.orientation = orientation;
        jointConfig.position = translation;
        jointConfig.scale = scale.x;
        jointConfig.bindTransformInv = colladaJoint.inverse0;

        jointConfig.bbox.Empty();
    }

    //TODO: *Skinning* calc bounding sphere instead bbox?
    for (auto& pgi : colladaNode->meshInstances[0]->polyGroupInstances)
    {
        PolygonGroup* polygonGroup = library.GetOrCreatePolygon(pgi);
        int32 jointIndex = -1;
        float32 jointWeight = 0.f;
        Vector3 position;
        int32 vxCount = polygonGroup->GetVertexCount();
        for (int32 v = 0; v < vxCount; ++v)
        {
            for (int32 j = 0; j < 4; ++j)
            {
                polygonGroup->GetJointWeight(v, j, jointWeight);
                if (jointWeight > EPSILON)
                {
                    polygonGroup->GetCoord(v, position);
                    polygonGroup->GetJointIndex(v, j, jointIndex);

                    jointConfigs[jointIndex].bbox.AddPoint(position * jointConfigs[jointIndex].bindTransformInv);
                }
            }
        }
    }

    davaSkeletonComponent->SetConfigJoints(jointConfigs);
}

eColladaErrorCodes ColladaImporter::BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode)
{
    eColladaErrorCodes res = eColladaErrorCodes::COLLADA_OK;

    if (colladaNode->originalNode->GetJointFlag())
        return res;

    ScopedPtr<Entity> nodeEntity(new Entity());
    String name = UTF8Utils::MakeUTF8String(colladaNode->originalNode->GetName().c_str());
    if (name.empty())
    {
        name = Format("UNNAMED");

        res = eColladaErrorCodes::COLLADA_ERROR;
        Logger::Error("[DAE to SC2] Unnamed node found as a child of %s", root->GetName().c_str());
        if (0 < colladaNode->childs.size())
        {
            Logger::Error("[DAE to SC2] It's childs:");
            for (auto child : colladaNode->childs)
            {
                Logger::Error("[DAE to SC2] %s", child->originalNode->GetName().c_str());
            }
        }
    }

    nodeEntity->SetName(FastName(name));

    // take mesh from node and put it into entity's render component
    const eColladaErrorCodes importMeshesResult = ImportMeshes(colladaNode->meshInstances, nodeEntity);
    res = Max(importMeshesResult, res);

    // Import skeleton
    ImportSkeleton(colladaNode, nodeEntity);

    // Import animation
    ImportAnimation(colladaNode, nodeEntity);

    auto* transformComponent = GetTransformComponent(nodeEntity);
    transformComponent->SetLocalTransform(&colladaNode->localTransform);

    root->AddNode(nodeEntity);

    for (auto childNode : colladaNode->childs)
    {
        const eColladaErrorCodes childRes = BuildSceneAsCollada(nodeEntity, childNode);
        res = Max(res, childRes);
    }

    return res;
}

void ColladaImporter::LoadMaterialParents(ColladaScene* colladaScene)
{
    for (auto cmaterial : colladaScene->colladaMaterials)
    {
        NMaterial* globalMaterial = library.GetOrCreateMaterialParent(cmaterial, false);
        DVASSERT(nullptr != globalMaterial);
    }
}

void ColladaImporter::LoadAnimations(ColladaScene* colladaScene)
{
    for (auto canimation : colladaScene->colladaAnimations)
    {
        for (auto& pair : canimation->animations)
        {
            SceneNodeAnimation* colladaAnimation = pair.second;
            AnimationData* animation = library.GetOrCreateAnimation(colladaAnimation);
            DVASSERT(nullptr != animation);
        }
    }
}

eColladaErrorCodes ColladaImporter::SaveSC2(ColladaScene* colladaScene, const FilePath& scenePath)
{
    ScopedPtr<Scene> scene(new Scene());

    // Load scene global materials.
    LoadMaterialParents(colladaScene);

    // Load scene global animations
    LoadAnimations(colladaScene);

    // Iterate recursive over collada scene and build Dava Scene with same hierarchy

    eColladaErrorCodes convertRes = BuildSceneAsCollada(scene, colladaScene->rootNode);
    if (eColladaErrorCodes::COLLADA_OK == convertRes)
    {
        // Apply transforms to render batches and use identity local transforms
        SceneUtils::BakeTransformsUpToFarParent(scene, scene);

        // post process Entities and create Lod nodes.
        bool combinedSuccessfull = SceneUtils::CombineLods(scene);
        if (combinedSuccessfull)
        {
            SceneFileV2::eError saveRes = scene->SaveScene(scenePath);
            if (saveRes > SceneFileV2::eError::ERROR_NO_ERROR)
            {
                Logger::Error("[DAE to SC2] Cannot save SC2. Error %d", saveRes);
                convertRes = eColladaErrorCodes::COLLADA_ERROR;
            }
        }
        else
        {
            convertRes = eColladaErrorCodes::COLLADA_ERROR;
        }
    }

    return convertRes;
}

eColladaErrorCodes ColladaImporter::SaveAnimations(ColladaScene* colladaScene, const FilePath& path)
{
    return COLLADA_OK;
}
};
