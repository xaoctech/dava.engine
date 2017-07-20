#include "Scene3D/SceneUtils.h"

#include "Utils/StringFormat.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Scene3D/Entity.h"
#include "Entity/Component.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Logger/Logger.h"
#include "Base/Map.h"

namespace DAVA
{
namespace SceneUtils
{
namespace SceneUtilsDetails
{
void RetainRenderEntitiesRecursive(Entity* node, Vector<Entity*>& entities)
{
    for (Entity* child : node->children)
        RetainRenderEntitiesRecursive(child, entities);

    if (GetRenderObject(node) != nullptr)
        entities.push_back(SafeRetain(node));
}
}

bool CombineLods(Scene* scene)
{
    bool result = true;
    for (auto child : scene->children)
    {
        result = result && CombineEntityLods(child);
    }

    return result;
}

String LodNameForIndex(const String& pattern, uint32 lodIndex)
{
    return Format(pattern.c_str(), lodIndex);
}

bool VerifyNames(const List<Entity*>& lodNodes, const String& lodSubString)
{
    bool allNamesAreDifferent = true;

    Map<String, String> validationMap;
    for (Entity* oneLodNode : lodNodes)
    {
        const String lodName(oneLodNode->GetName().c_str());
        const String nodeWithoutLodsName(lodName, 0, lodName.find(lodSubString));

        if (validationMap.count(nodeWithoutLodsName) != 0)
        {
            Logger::Error("Geometry Error: %s will overwrite geometry of %s", lodName.c_str(), validationMap[nodeWithoutLodsName].c_str());
            allNamesAreDifferent = false;
        }
        else
        {
            validationMap[nodeWithoutLodsName] = lodName;
        }
    }

    return allNamesAreDifferent;
}

bool CombineEntityLods(Entity* forRootNode)
{
    const String lodNamePattern("_lod%d");
    const String dummyLodNamePattern("_lod%ddummy");

    List<Entity*> lod0Nodes;

    // try to find nodes which have lodNamePattrn.
    const String lod0 = LodNameForIndex(lodNamePattern, 0);
    if (!forRootNode->FindNodesByNamePart(lod0, lod0Nodes))
    {
        // There is no lods.
        return true;
    }

    //model validation step: we should ignore combination of lods if we found several geometry meshes per lod
    if (VerifyNames(lod0Nodes, lod0) == false)
    {
        return false;
    }

    // ok. We have some nodes with lod 0 in the name. Try to find other lods for same name.
    for (Entity* oneLodNode : lod0Nodes)
    {
        // node name which contains lods
        const String lodName(oneLodNode->GetName().c_str());
        const String nodeWithLodsName(lodName, 0, lodName.find(lod0));

        Entity* oldParent = oneLodNode->GetParent();

        ScopedPtr<Entity> newNodeWithLods(new Entity());
        newNodeWithLods->SetName(nodeWithLodsName.c_str());

        Array<Vector<Entity*>, LodComponent::MAX_LOD_LAYERS> lodEntities;

        int32 lodCount = 0;
        for (int32 lodNo = 0; lodNo < LodComponent::MAX_LOD_LAYERS; ++lodNo)
        {
            // Try to find node with same name but with other lod
            const FastName lodIName(nodeWithLodsName + LodNameForIndex(lodNamePattern, lodNo));
            Entity* ln = oldParent->FindByName(lodIName.c_str());

            if (nullptr != ln)
            {
                SceneUtilsDetails::RetainRenderEntitiesRecursive(ln, lodEntities[lodNo]);
                CollapseAnimationsUpToFarParent(ln, newNodeWithLods);

                oldParent->RemoveNode(ln);

                ++lodCount;
            }

            // Try to find dummy lod node
            const FastName dummyLodName(nodeWithLodsName + LodNameForIndex(dummyLodNamePattern, lodNo));
            ln = oldParent->FindByName(dummyLodName.c_str());

            if (nullptr != ln)
            {
                // Remove dummy nodes
                ln->RemoveAllChildren();
                oldParent->RemoveNode(ln);
            }
        }

        if (lodCount > 0)
        {
            LodComponent* lc = new LodComponent();
            newNodeWithLods->AddComponent(lc);
            if (lodCount < LodComponent::MAX_LOD_LAYERS && lodCount > LodComponent::INVALID_LOD_LAYER)
            {
                // Fix max lod distance for max used lod index
                lc->SetLodLayerDistance(lodCount, LodComponent::MAX_LOD_DISTANCE);
            }
        }

        SkeletonComponent* skeleton = nullptr;
        for (int32 lodNo = 0; lodNo < LodComponent::MAX_LOD_LAYERS; ++lodNo)
        {
            for (Entity* node : lodEntities[lodNo])
            {
                //Search any skeleton. Skeleton should be the same in all lods
                skeleton = GetSkeletonComponent(node);
                if (skeleton)
                {
                    node->DetachComponent(skeleton);
                    newNodeWithLods->AddComponent(skeleton);
                    break;
                }
            }
            if (skeleton)
                break;
        }

        RenderObject* newMesh = nullptr;
        if (skeleton)
        {
            newMesh = new SkinnedMesh();
        }
        else
        {
            newMesh = new Mesh();
        }

        for (int32 lodNo = 0; lodNo < LodComponent::MAX_LOD_LAYERS; ++lodNo)
        {
            for (Entity* node : lodEntities[lodNo])
            {
                RenderObject* lodRenderObject = GetRenderObject(node);

                while (lodRenderObject->GetRenderBatchCount() > 0)
                {
                    ScopedPtr<RenderBatch> batch(SafeRetain(lodRenderObject->GetRenderBatch(0)));

                    const SkinnedMesh::JointTargets* jointTargets = nullptr;
                    if (lodRenderObject->GetType() == RenderObject::TYPE_SKINNED_MESH)
                        jointTargets = static_cast<SkinnedMesh*>(lodRenderObject)->GetJointTargets(batch);

                    lodRenderObject->RemoveRenderBatch(batch);
                    newMesh->AddRenderBatch(batch, lodNo, -1);

                    if (jointTargets)
                        static_cast<SkinnedMesh*>(newMesh)->SetJointTargets(batch, *jointTargets);
                };

                SafeRelease(node);
            }
        }

        RenderComponent* rc = new RenderComponent();
        rc->SetRenderObject(newMesh);
        SafeRelease(newMesh);

        newNodeWithLods->AddComponent(rc);
        oldParent->AddNode(newNodeWithLods);

        DVASSERT(oldParent->GetScene());
        DVASSERT(newNodeWithLods->GetScene());
    }

    return true;
}

void BakeTransformsUpToFarParent(Entity* parent, Entity* currentNode)
{
    for (auto child : currentNode->children)
    {
        BakeTransformsUpToFarParent(parent, child);
    }

    // Bake transforms to geometry
    RenderObject* ro = GetRenderObject(currentNode);
    if (ro)
    {
        // Get actual transformation for current entity
        Matrix4 totalTransform = currentNode->AccamulateTransformUptoFarParent(parent);
        ro->BakeGeometry(totalTransform);
    }

    // Set local transform as Ident because transform is already baked up into geometry
    auto transformComponent = GetTransformComponent(currentNode);
    transformComponent->SetLocalTransform(&Matrix4::IDENTITY);
}

void CollapseAnimationsUpToFarParent(Entity* node, Entity* parent)
{
    for (auto child : parent->children)
    {
        CollapseAnimationsUpToFarParent(child, parent);
    }

    Component* ac = GetAnimationComponent(node);
    if (ac)
    {
        node->DetachComponent(ac);
        parent->AddComponent(ac);
    }
}

} //namespace SceneUtils

} //namespace DAVA
