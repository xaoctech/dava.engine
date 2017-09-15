#include "FBXImporter.h"
#include "DAVAEngine.h"

#include "Animation/AnimationClip.h"
#include "Utils/CRC32.h"

#define FBXSDK_SHARED //requested only for dynamic linking

#include <fbxsdk.h>

namespace DAVA
{
namespace FBXImporterDetails
{
using VertexInfluence = std::pair<uint32, float32>; //[jointIndex, jointWeight]
using FbxControlPointInfluences = Vector<VertexInfluence>;
using GeometrySet = Vector<std::pair<PolygonGroup*, NMaterial*>>;

struct FBXVertex
{
    struct JointWeightComparator
    {
        bool operator()(const VertexInfluence& l, const VertexInfluence& r) const
        {
            return l.second > r.second; //for sorting in descending order
        }
    };

    union
    {
        float32 data[14];
        struct
        {
            Vector3 position;
            Vector2 texCoord[4];
            Vector3 normal;
        };
    };
    Set<VertexInfluence, JointWeightComparator> joints;

    //////////////////////////////////////////////////////////////////////////

    FBXVertex();
    FBXVertex(const FBXVertex& other);

    bool operator<(const FBXVertex& other) const;
};

struct FBXJoint
{
    const FbxSkeleton* joint = nullptr;
    const FbxSkeleton* parentJoint = nullptr;
    uint32 hierarchyDepth = 0;

    uint32 parentIndex = SkeletonComponent::INVALID_JOINT_INDEX;
};

struct ProcessedMesh
{
    GeometrySet geometry;
    SkeletonComponent* skeleton = nullptr;
    uint32 maxVertexInfluenceCount = 0;
};

//////////////////////////////////////////////////////////////////////////

void ProcessHierarchyRecursive(FbxNode* node, Entity* entity);
void ClearCache();

void ProcessMesh(const FbxMesh* fbxMesh, Entity* entity);
NMaterial* RetrieveMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence);

void ProcessSkeletonsRecursive(const FbxNode* fbxSkeleton);
void CollectSkeletonNodes(const FbxSkeleton* joint, Vector<FBXJoint>* fbxJoints, const FbxSkeleton* parentJoint = nullptr, uint32 depth = 0);
void ProcessSkeletonHierarchy(const FbxSkeleton* fbxSkeleton);

//////////////////////////////////////////////////////////////////////////

struct FBXAnimationKey
{
    float32 time;
    Vector3 value;
};

struct FBXAnimationChannelData
{
    FBXAnimationChannelData() = default;
    FBXAnimationChannelData(AnimationTrack::eChannelTarget _channel, const Vector<FBXAnimationKey>& _animationKeys)
        : channel(_channel)
        , animationKeys(_animationKeys)
    {
    }

    AnimationTrack::eChannelTarget channel = AnimationTrack::CHANNEL_TARGET_COUNT;
    Vector<FBXAnimationKey> animationKeys;
};

struct FBXNodeAnimationData
{
    FbxNode* fbxNode = nullptr;
    Vector<FBXAnimationChannelData> animationTrackData;
};

struct FBXAnimationStackData
{
    String name;
    float32 duration = 0.f;
    Vector<FBXNodeAnimationData> nodesAnimations;
};

Vector<FBXAnimationStackData> ProcessAnimations(FbxScene* fbxScene);
void SaveAnimation(const FBXAnimationStackData& fbxStackAnimationData, const FilePath& filePath);

//////////////////////////////////////////////////////////////////////////

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix);
Vector3 ToVector3(const FbxVector4& fbxVector);
const char* GetFBXTexturePath(const FbxProperty& textureProperty);
FastName GenerateUID(const FbxNode* node);

FbxAMatrix GetGeometricTransform(const FbxNode* pNode);
const FbxSkeleton* GetSkeletonAttribute(const FbxNode* fbxNode);
const FbxSkeleton* FindSkeletonRoot(const FbxSkeleton* fbxNode);
SkeletonComponent* ProcessSkin(FbxSkin* fbxSkin, Vector<FbxControlPointInfluences>* controlPointsInfluences, uint32* outMaxInfluenceCount);

//////////////////////////////////////////////////////////////////////////

Map<const FbxMesh*, ProcessedMesh> meshCache; //in ProcessedMesh::GeometrySet materials isn't retained. It's owned by materialCache
Map<std::pair<const FbxSurfaceMaterial*, uint32>, NMaterial*> materialCache;
Map<const FbxSkeleton*, Vector<FBXJoint>> linkedSkeletons; // [rootJoint, jointsArray]

static uint32 materialInstanceIndex = 0;
}

//////////////////////////////////////////////////////////////////////////

FbxScene* ImportFbxScene(FbxManager* fbxManager, const FilePath& fbxPath)
{
    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    FbxImporter* importer = FbxImporter::Create(fbxManager, "fbxImporter");

    bool initSuccess = importer->Initialize(fbxPath.GetAbsolutePathname().c_str());
    if (!initSuccess)
    {
        Logger::Error("FbxImporter Initialization error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "importedScene");
    bool importSuccess = importer->Import(fbxScene);
    if (!importSuccess)
    {
        Logger::Error("FBX Import error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }
    importer->Destroy();

    FbxAxisSystem::MayaZUp.ConvertScene(fbxScene); // UpVector = ZAxis, CoordSystem = RightHanded

    FbxSystemUnit::ConversionOptions fbxConversionOptions = {
        false, /* mConvertRrsNodes */
        true, /* mConvertLimits */
        true, /* mConvertClusters */
        true, /* mConvertLightIntensity */
        true, /* mConvertPhotometricLProperties */
        true /* mConvertCameraClipPlanes */
    };
    FbxSystemUnit::m.ConvertScene(fbxScene, fbxConversionOptions); //Unit = meter

    return fbxScene;
}

Scene* FBXImporter::ConstructSceneFromFBX(const FilePath& fbxPath)
{
    FbxManager* fbxManager = FbxManager::Create();

    Scene* scene = nullptr;
    FbxScene* fbxScene = ImportFbxScene(fbxManager, fbxPath);
    if (fbxScene != nullptr)
    {
        FbxGeometryConverter fbxGeometryConverter(fbxManager);
        fbxGeometryConverter.Triangulate(fbxScene, true); //Triangulate whole scene

        scene = new Scene();
        FBXImporterDetails::ProcessSkeletonsRecursive(fbxScene->GetRootNode());
        FBXImporterDetails::ProcessHierarchyRecursive(fbxScene->GetRootNode(), scene);
        FBXImporterDetails::ClearCache();
        fbxScene->Destroy();
    }
    fbxManager->Destroy();

    return scene;
}

bool FBXImporter::ConvertToSC2(const FilePath& fbxPath, const FilePath& sc2Path)
{
    Scene* scene = ConstructSceneFromFBX(fbxPath);
    if (scene)
    {
        scene->SaveScene(sc2Path);
        return true;
    }

    return false;
}

bool FBXImporter::ConvertAnimations(const FilePath& fbxPath)
{
    FbxManager* fbxManager = FbxManager::Create();

    FbxScene* fbxScene = ImportFbxScene(fbxManager, fbxPath);
    if (fbxScene != nullptr)
    {
        Vector<FBXImporterDetails::FBXAnimationStackData> animations = FBXImporterDetails::ProcessAnimations(fbxScene);

        for (FBXImporterDetails::FBXAnimationStackData& animation : animations)
        {
            FilePath animationPath = (animations.size() == 1) ? FilePath::CreateWithNewExtension(fbxPath, ".anim") : FilePath::CreateWithNewExtension(fbxPath, animation.name + ".anim");
            FBXImporterDetails::SaveAnimation(animation, animationPath);
        }

        fbxScene->Destroy();
    }
    fbxManager->Destroy();

    return (fbxScene != nullptr);
}

//////////////////////////////////////////////////////////////////////////
//Details implementation
namespace FBXImporterDetails
{
void ProcessHierarchyRecursive(FbxNode* fbxNode, Entity* entity)
{
    entity->SetName(fbxNode->GetName());

    Matrix4 transform = ToMatrix4(fbxNode->EvaluateLocalTransform());
    entity->SetLocalTransform(transform);

    int32 attrCount = fbxNode->GetNodeAttributeCount();
    for (int32 a = 0; a < attrCount; ++a)
    {
        const FbxNodeAttribute* attr = fbxNode->GetNodeAttributeByIndex(a);
        if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            ProcessMesh(static_cast<const FbxMesh*>(attr), entity);
        }
    }

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
    {
        ScopedPtr<Entity> childEntity(new Entity());
        entity->AddNode(childEntity);

        ProcessHierarchyRecursive(fbxNode->GetChild(c), childEntity);
    }
}

void ClearCache()
{
    for (auto& it : meshCache)
    {
        SafeDelete(it.second.skeleton);

        for (auto& p : it.second.geometry)
            SafeRelease(p.first);
        //in geometry cache material isn't retained
    }
    meshCache.clear();

    for (auto& it : materialCache)
    {
        SafeRelease(it.second);
    }
    materialCache.clear();

    linkedSkeletons.clear();

    materialInstanceIndex = 0;
}

//////////////////////////////////////////////////////////////////////////

void ProcessMesh(const FbxMesh* fbxMesh, Entity* entity)
{
    auto found = meshCache.find(fbxMesh);
    if (found == meshCache.end())
    {
        FbxNode* fbxNode = fbxMesh->GetNode();
        DVASSERT(fbxNode);

        bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
        bool hasSkinning = fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;

        uint32 maxControlPointInfluence = 0;
        Vector<FbxControlPointInfluences> controlPointsInfluences;
        SkeletonComponent* skeleton = nullptr;
        if (hasSkinning)
        {
            FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
            skeleton = ProcessSkin(skin, &controlPointsInfluences, &maxControlPointInfluence);
        }
        maxControlPointInfluence = Min(maxControlPointInfluence, PolygonGroup::MAX_JOINTS_COUNT);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        int32 uvCount = Min(uvNames.GetCount(), 4);

        int32 meshFormat = EVF_VERTEX;
        if (uvCount > 0)
            meshFormat |= EVF_TEXCOORD0;
        if (uvCount > 1)
            meshFormat |= EVF_TEXCOORD1;
        if (uvCount > 2)
            meshFormat |= EVF_TEXCOORD2;
        if (uvCount > 3)
            meshFormat |= EVF_TEXCOORD3;
        if (hasNormal)
            meshFormat |= EVF_NORMAL | EVF_TANGENT | EVF_BINORMAL;
        if (maxControlPointInfluence == 1)
            meshFormat |= EVF_HARD_JOINTINDEX;
        if (maxControlPointInfluence > 1)
            meshFormat |= EVF_JOINTINDEX | EVF_JOINTWEIGHT;

        FbxVector4 tmpNormal;
        FbxVector2 tmpUV;
        bool tmpUnmapped = false;

        using VerticesMap = Map<FBXVertex, Vector<int32>>; //[vertex, indices]
        using MaterialGeometryMap = Map<FbxSurfaceMaterial*, VerticesMap>;

        MaterialGeometryMap materialGeometry;
        int32 polygonCount = fbxMesh->GetPolygonCount();
        for (int32 p = 0; p < polygonCount; p++)
        {
            int32 polygonSize = fbxMesh->GetPolygonSize(p);
            DVASSERT(polygonSize == 3);

            for (int32 v = 0; v < polygonSize; ++v)
            {
                FBXVertex vertex;

                int32 vIndex = fbxMesh->GetPolygonVertex(p, v);
                const FbxVector4& coords = fbxMesh->GetControlPointAt(vIndex);
                vertex.position = ToVector3(coords);

                if (hasNormal)
                {
                    fbxMesh->GetPolygonVertexNormal(p, v, tmpNormal);
                    vertex.normal = ToVector3(tmpNormal);
                }

                for (int32 t = 0; t < uvCount; ++t)
                {
                    fbxMesh->GetPolygonVertexUV(p, v, uvNames[t], tmpUV, tmpUnmapped);
                    vertex.texCoord[t] = Vector2(float32(tmpUV[0]), -float32(tmpUV[1]));
                }

                if (hasSkinning)
                {
                    const FbxControlPointInfluences& vertexInfluences = controlPointsInfluences[vIndex];

                    float32 weightsSum = 0.f;
                    for (const VertexInfluence& vInf : vertexInfluences)
                        weightsSum += vInf.second;

                    for (const VertexInfluence& vInf : vertexInfluences)
                        vertex.joints.insert(VertexInfluence(vInf.first, vInf.second / weightsSum));
                }

                FbxSurfaceMaterial* fbxMaterial = nullptr;
                for (int32 me = 0; me < fbxMesh->GetElementMaterialCount(); me++)
                {
                    fbxMaterial = fbxNode->GetMaterial(fbxMesh->GetElementMaterial(me)->GetIndexArray().GetAt(p));
                    if (fbxMaterial != nullptr)
                        break;
                }

                materialGeometry[fbxMaterial][vertex].push_back(p * 3 + v);
            }
        }

        GeometrySet geometrySet;
        for (auto& it : materialGeometry)
        {
            FbxSurfaceMaterial* fbxMaterial = it.first;
            const VerticesMap& vertices = it.second;

            int32 vxCount = int32(vertices.size());
            int32 indCount = polygonCount * 3;

            PolygonGroup* polygonGroup = new PolygonGroup();
            polygonGroup->AllocateData(meshFormat, vxCount, indCount);

            int32 vertexIndex = 0;
            for (auto it = vertices.cbegin(); it != vertices.cend(); ++it)
            {
                const FBXVertex& fbxVertex = it->first;

                polygonGroup->SetCoord(vertexIndex, fbxVertex.position);

                for (int32 t = 0; t < uvCount; ++t)
                    polygonGroup->SetTexcoord(t, vertexIndex, fbxVertex.texCoord[t]);

                if (hasNormal)
                    polygonGroup->SetNormal(vertexIndex, fbxVertex.normal);

                if (hasSkinning)
                {
                    if (maxControlPointInfluence == 1) //hard-skinning
                    {
                        polygonGroup->SetHardJointIndex(vertexIndex, int32(fbxVertex.joints.cbegin()->first));
                    }
                    else
                    {
                        auto vInf = fbxVertex.joints.cbegin();
                        for (uint32 j = 0; j < PolygonGroup::MAX_JOINTS_COUNT; ++j)
                        {
                            if (vInf != fbxVertex.joints.end())
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, int32(vInf->first));
                                polygonGroup->SetJointWeight(vertexIndex, j, vInf->second);

                                ++vInf;
                            }
                            else
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, 0);
                                polygonGroup->SetJointWeight(vertexIndex, j, 0.f);
                            }
                        }
                    }
                }

                for (int32 index : it->second)
                    polygonGroup->SetIndex(index, uint16(vertexIndex));

                ++vertexIndex;
            }

            if (hasNormal)
                MeshUtils::RebuildMeshTangentSpace(polygonGroup);

            geometrySet.emplace_back(polygonGroup, RetrieveMaterial(fbxMaterial, maxControlPointInfluence));
        }

        found = meshCache.emplace(fbxMesh, ProcessedMesh()).first;
        found->second.geometry = std::move(geometrySet);
        found->second.skeleton = skeleton;
        found->second.maxVertexInfluenceCount = maxControlPointInfluence;
    }

    bool isSkinned = (found->second.skeleton != nullptr);
    if (isSkinned)
    {
        ScopedPtr<SkinnedMesh> mesh(new SkinnedMesh());

        uint32 maxVertexInfluenceCount = found->second.maxVertexInfluenceCount;
        const GeometrySet& geometrySet = found->second.geometry;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

            ScopedPtr<NMaterial> materialInstance(new NMaterial());
            materialInstance->SetParent(material);
            materialInstance->SetMaterialName(FastName(Format("Instance%d", materialInstanceIndex++).c_str()));

            auto splitedPolygons = MeshUtils::SplitSkinnedMeshGeometry(polygonGroup, SkinnedMesh::MAX_TARGET_JOINTS);
            for (auto& p : splitedPolygons)
            {
                PolygonGroup* pg = p.first;
                //TODO?
                //pg->ApplyMatrix(bindShapeMatrix);
                pg->RecalcAABBox();

                ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
                renderBatch->SetPolygonGroup(pg);
                renderBatch->SetMaterial(materialInstance);

                mesh->AddRenderBatch(renderBatch);
                mesh->SetJointTargets(renderBatch, p.second);

                SafeRelease(pg);
            }
        }

        entity->AddComponent(new RenderComponent(mesh));
        entity->AddComponent(found->second.skeleton->Clone(entity));
    }
    else
    {
        ScopedPtr<Mesh> mesh(new Mesh());

        const GeometrySet& geometrySet = found->second.geometry;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

            ScopedPtr<NMaterial> materialInstance(new NMaterial());
            materialInstance->SetParent(material);
            materialInstance->SetMaterialName(FastName(Format("Instance%d", materialInstanceIndex++).c_str()));

            mesh->AddPolygonGroup(polygonGroup, materialInstance);
        }

        entity->AddComponent(new RenderComponent(mesh));
    }
}

NMaterial* RetrieveMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence)
{
    auto found = materialCache.find(std::make_pair(fbxMaterial, maxVertexInfluence));
    if (found == materialCache.end())
    {
        NMaterial* material = new NMaterial();
        material->SetFXName(NMaterialName::TEXTURED_OPAQUE);

        if (maxVertexInfluence > 0)
        {
            if (maxVertexInfluence == 1)
                material->AddFlag(NMaterialFlagName::FLAG_HARD_SKINNING, 1);
            else
                material->AddFlag(NMaterialFlagName::FLAG_SOFT_SKINNING, maxVertexInfluence);
        }

        if (fbxMaterial != nullptr)
        {
            material->SetMaterialName(FastName(fbxMaterial->GetName()));

            Vector<std::pair<const char*, FastName>> texturesToImport = {
                { FbxSurfaceMaterial::sDiffuse, NMaterialTextureName::TEXTURE_ALBEDO },
                { FbxSurfaceMaterial::sNormalMap, NMaterialTextureName::TEXTURE_NORMAL }
            };

            for (auto& tex : texturesToImport)
            {
                const char* texturePath = GetFBXTexturePath(fbxMaterial->FindProperty(tex.first));
                if (texturePath)
                    material->AddTexture(tex.second, Texture::CreateFromFile(FilePath(texturePath)));
            }
        }
        else
        {
            material->SetMaterialName(FastName("UNNAMED"));
        }

        found = materialCache.emplace(std::make_pair(fbxMaterial, maxVertexInfluence), material).first;
    }

    return found->second;
}

void ProcessSkeletonsRecursive(const FbxNode* fbxNode)
{
    int32 attrCount = fbxNode->GetNodeAttributeCount();
    for (int32 a = 0; a < attrCount; ++a)
    {
        const FbxSkeleton* fbxSkeleton = GetSkeletonAttribute(fbxNode);
        if (fbxSkeleton != nullptr && fbxSkeleton->IsSkeletonRoot())
        {
            ProcessSkeletonHierarchy(fbxSkeleton);
            return;
        }
    }

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
        ProcessSkeletonsRecursive(fbxNode->GetChild(c));
}

void CollectSkeletonNodes(const FbxSkeleton* joint, Vector<FBXJoint>* fbxJoints, const FbxSkeleton* parentJoint, uint32 depth)
{
    if (joint != nullptr && joint->GetSkeletonType())
    {
        fbxJoints->emplace_back();
        fbxJoints->back().joint = joint;
        fbxJoints->back().parentJoint = parentJoint;
        fbxJoints->back().hierarchyDepth = depth;

        const FbxNode* fbxNode = joint->GetNode();
        int32 childCount = fbxNode->GetChildCount();
        for (int32 c = 0; c < childCount; ++c)
        {
            const FbxSkeleton* childJoint = GetSkeletonAttribute(fbxNode->GetChild(c));
            if (childJoint != nullptr)
            {
                CollectSkeletonNodes(childJoint, fbxJoints, joint, depth + 1);
            }
        }
    }
}

void ProcessSkeletonHierarchy(const FbxSkeleton* fbxSkeleton)
{
    DVASSERT(linkedSkeletons.count(fbxSkeleton) == 0);

    Vector<FBXJoint> fbxJoints; //[node, hierarchy-depth]
    CollectSkeletonNodes(fbxSkeleton, &fbxJoints);

    //sort nodes by hierarchy depth, and by uid inside hierarchy-level
    std::sort(fbxJoints.begin(), fbxJoints.end(), [](const FBXJoint& l, const FBXJoint& r)
              {
                  if (l.hierarchyDepth == r.hierarchyDepth)
                      return (l.joint->GetUniqueID() < r.joint->GetUniqueID());
                  else
                      return (l.hierarchyDepth < r.hierarchyDepth);
              });

    for (FBXJoint& fbxJoint : fbxJoints)
    {
        size_t parentIndex = std::distance(fbxJoints.begin(), std::find_if(fbxJoints.begin(), fbxJoints.end(), [&fbxJoint](const FBXJoint& item) {
                                               return (item.joint == fbxJoint.parentJoint);
                                           }));

        fbxJoint.parentIndex = (parentIndex == fbxJoints.size()) ? SkeletonComponent::INVALID_JOINT_INDEX : uint32(parentIndex);
    }

    linkedSkeletons.emplace(fbxSkeleton, std::move(fbxJoints));
}

//////////////////////////////////////////////////////////////////////////

Vector<FBXAnimationKey> GetChannelAnimationKeys(FbxPropertyT<FbxDouble3>& fbxProperty, FbxAnimLayer* fbxAnimLayer)
{
    Vector<FBXAnimationKey> animationKeys;

    FbxAnimCurve* fbxAnimCurve[3] = {
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X),
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y),
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z),
    };

    Set<FbxTime> keyTimes;
    for (FbxAnimCurve* curve : fbxAnimCurve)
    {
        if (curve)
        {
            int keyCount = curve->KeyGetCount();
            for (int keyIndex = 0; keyIndex < keyCount; keyIndex++)
                keyTimes.insert(curve->KeyGet(keyIndex).GetTime());
        }
    }

    if (!keyTimes.empty())
    {
        for (const FbxTime& t : keyTimes)
        {
            animationKeys.emplace_back();
            animationKeys.back().time = float32(t.GetSecondDouble());

            for (int32 c = 0; c < 3; ++c)
                animationKeys.back().value.data[c] = (fbxAnimCurve[c] != nullptr) ? fbxAnimCurve[c]->Evaluate(t) : float32(fbxProperty.EvaluateValue(t)[c]);
        }
    }

    return animationKeys;
}

FBXNodeAnimationData GetNodeAnimationData(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer)
{
    FBXNodeAnimationData result;
    result.fbxNode = fbxNode;

    result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_POSITION, GetChannelAnimationKeys(fbxNode->LclTranslation, fbxAnimLayer));
    result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_ORIENTATION, GetChannelAnimationKeys(fbxNode->LclRotation, fbxAnimLayer));
    result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_SCALE, GetChannelAnimationKeys(fbxNode->LclScaling, fbxAnimLayer));

    return result;
}

void ProcessNodeAnimationRecursive(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer, Vector<FBXNodeAnimationData>* outNodesAnimations)
{
    DVASSERT(fbxNode != nullptr);

    FBXNodeAnimationData nodeAnimationData = GetNodeAnimationData(fbxNode, fbxAnimLayer);
    if (!nodeAnimationData.animationTrackData.empty())
        outNodesAnimations->emplace_back(std::move(nodeAnimationData));

    int childrenCount = fbxNode->GetChildCount();
    for (int child = 0; child < childrenCount; child++)
        ProcessNodeAnimationRecursive(fbxNode->GetChild(child), fbxAnimLayer, outNodesAnimations);
}

Vector<FBXAnimationStackData> ProcessAnimations(FbxScene* fbxScene)
{
    Vector<FBXAnimationStackData> result;

    int animationStackCount = fbxScene->GetSrcObjectCount<FbxAnimStack>();
    for (int as = 0; as < fbxScene->GetSrcObjectCount<FbxAnimStack>(); as++)
    {
        FbxAnimStack* animationStack = fbxScene->GetSrcObject<FbxAnimStack>(as);

        result.emplace_back();
        result.back().name = animationStack->GetName();

        int animationLayersCount = animationStack->GetMemberCount<FbxAnimLayer>();
        if (animationLayersCount > 0)
        {
            if (animationLayersCount > 1)
            {
                Logger::Warning("[FBXImporter] FBX animation '%s' contains more than one animation layer. Import only first (base) layer", animationStack->GetName());
            }

            FbxAnimLayer* animationLayer = animationStack->GetMember<FbxAnimLayer>(0);
            ProcessNodeAnimationRecursive(fbxScene->GetRootNode(), animationLayer, &result.back().nodesAnimations);

            //Calculate duration
            float32 minTimeStamp = std::numeric_limits<float32>::infinity();
            float32 maxTimeStamp = -std::numeric_limits<float32>::infinity();
            for (auto& nodeAnimation : result.back().nodesAnimations)
            {
                for (auto& channelData : nodeAnimation.animationTrackData)
                {
                    if (!channelData.animationKeys.empty())
                    {
                        minTimeStamp = Min(minTimeStamp, channelData.animationKeys.front().time);
                        maxTimeStamp = Max(maxTimeStamp, channelData.animationKeys.back().time);
                    }
                }
            }

            result.back().duration = maxTimeStamp - minTimeStamp;
        }
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

void WriteToBuffer(Vector<uint8>& buffer, const void* data, uint32 size)
{
    DVASSERT(data != nullptr && size != 0);

    const uint8* bytes = reinterpret_cast<const uint8*>(data);
    buffer.insert(buffer.end(), bytes, bytes + size);
}

template <class T>
void WriteToBuffer(Vector<uint8>& buffer, const T* value)
{
    WriteToBuffer(buffer, value, sizeof(T));
}

void WriteToBuffer(Vector<uint8>& buffer, const String& string)
{
    uint32 stringBytes = uint32(string.length() + 1);
    WriteToBuffer(buffer, string.c_str(), stringBytes);

    //as we load animation data directly to memory and use it without any processing we have to align strings data
    uint32 stringAlignment = 4 - (stringBytes & 0x3);
    if (stringAlignment > 0 && stringAlignment < 4)
    {
        uint32 pad = 0;
        WriteToBuffer(buffer, &pad, stringAlignment);
    }
}

void SaveAnimation(const FBXAnimationStackData& fbxStackAnimationData, const FilePath& filePath)
{
    //binary file format described in 'AnimationBinaryFormat.md'
    struct ChannelHeader
    {
        //Track part
        uint8 target;
        uint8 pad0[3];

        //Channel part
        uint32 signature = AnimationChannel::ANIMATION_CHANNEL_DATA_SIGNATURE;
        uint8 dimension = 0;
        uint8 interpolation = 0;
        uint16 compression = 0;
        uint32 keyCount = 0;
    } channelHeader;

    ScopedPtr<File> file(File::Create(filePath, File::CREATE | File::WRITE));
    if (file)
    {
        Vector<uint8> animationData;

        WriteToBuffer(animationData, &fbxStackAnimationData.duration);

        uint32 nodeCount = uint32(fbxStackAnimationData.nodesAnimations.size());
        WriteToBuffer(animationData, &nodeCount);

        //Write nodes data
        for (auto& fbxNodeData : fbxStackAnimationData.nodesAnimations)
        {
            //TODO: FBX_COMPLETE bake parents transform to root-joint animation ?

            String nodeUID = GenerateUID(fbxNodeData.fbxNode).c_str();
            String nodeName = fbxNodeData.fbxNode->GetName();

            WriteToBuffer(animationData, nodeUID);
            WriteToBuffer(animationData, nodeName);

            //Write Track data
            uint32 signature = AnimationTrack::ANIMATION_TRACK_DATA_SIGNATURE;
            WriteToBuffer(animationData, &signature);

            uint32 channelsCount = 0;
            for (auto& fbxChannelData : fbxNodeData.animationTrackData)
            {
                if (!fbxChannelData.animationKeys.empty())
                    ++channelsCount;
            }

            WriteToBuffer(animationData, &channelsCount);

            for (auto& fbxChannelData : fbxNodeData.animationTrackData)
            {
                if (!fbxChannelData.animationKeys.empty())
                {
                    channelHeader.target = fbxChannelData.channel;
                    channelHeader.interpolation = uint8(AnimationChannel::INTERPOLATION_LINEAR);
                    channelHeader.keyCount = uint32(fbxChannelData.animationKeys.size());
                    channelHeader.dimension =
                    (channelHeader.target == AnimationTrack::CHANNEL_TARGET_POSITION) ? 3 :
                                                                                        (channelHeader.target == AnimationTrack::CHANNEL_TARGET_ORIENTATION) ? 4 :
                                                                                                                                                               (channelHeader.target == AnimationTrack::CHANNEL_TARGET_SCALE) ? 1 : 0;

                    WriteToBuffer(animationData, &channelHeader);

                    for (uint32 k = 0; k < channelHeader.keyCount; ++k)
                    {
                        const FBXAnimationKey& key = fbxChannelData.animationKeys[k];

                        WriteToBuffer(animationData, &key.time);

                        if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_POSITION)
                        {
                            WriteToBuffer(animationData, &key.value);
                        }
                        else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_ORIENTATION)
                        {
                            Vector3 euler(DegToRad(key.value.x), DegToRad(key.value.y), DegToRad(key.value.z));
                            Quaternion orientation;
                            orientation.Construct(euler);

                            WriteToBuffer(animationData, &orientation);
                        }
                        else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_SCALE)
                        {
                            WriteToBuffer(animationData, &key.value.x);
                        }
                    }
                }
            }
        }

        uint32 markerCount = 0;
        WriteToBuffer(animationData, &markerCount); //TODO: *Skinning* marks count

        uint32 animationDataSize = uint32(animationData.size());

        AnimationClip::FileHeader header;
        header.signature = AnimationClip::ANIMATION_CLIP_FILE_SIGNATURE;
        header.version = 1;
        header.crc32 = CRC32::ForBuffer(animationData.data(), animationDataSize);
        header.dataSize = animationDataSize;

        file->Write(&header);
        file->Write(animationData.data(), animationDataSize);

        file->Flush();
    }
    else
    {
        Logger::Error("[FBXImporter] Failed to open file for writing: %s", filePath.GetAbsolutePathname().c_str());
    }
}

//////////////////////////////////////////////////////////////////////////

FBXVertex::FBXVertex()
{
    Memset(data, 0, sizeof(data));
}

FBXVertex::FBXVertex(const FBXVertex& other)
    : joints(other.joints)
{
    Memcpy(data, other.data, sizeof(data));
}

inline bool FBXVertex::operator<(const FBXVertex& other) const
{
    for (int32 d = 0; d < 14; ++d)
    {
        if (!FLOAT_EQUAL(data[d], other.data[d]))
            return data[d] < other.data[d];
    }

    if (joints.size() != other.joints.size() || joints.size() == 0)
        return joints.size() < other.joints.size();

    auto i = joints.cbegin();
    auto j = other.joints.cbegin();
    while (i != joints.cend())
    {
        if (i->first != j->first)
            return i->first < j->first;

        if (!FLOAT_EQUAL(i->second, j->second))
            return i->second < j->second;

        ++i;
        ++j;
    }

    return false;
}

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix)
{
    Matrix4 mx;

    for (int32 r = 0; r < 4; ++r)
        for (int32 c = 0; c < 4; ++c)
            mx._data[r][c] = float32(fbxMatrix.Get(r, c));

    return mx;
}

Vector3 ToVector3(const FbxVector4& fbxVector)
{
    return Vector3(float32(fbxVector[0]), float32(fbxVector[1]), float32(fbxVector[2]));
}

const char* GetFBXTexturePath(const FbxProperty& textureProperty)
{
    FbxTexture* fbxTexture = nullptr;
    if (textureProperty.GetSrcObjectCount<FbxLayeredTexture>() > 0)
    {
        FbxLayeredTexture* layeredTexture = textureProperty.GetSrcObject<FbxLayeredTexture>(0);
        if (layeredTexture->GetSrcObjectCount<FbxTexture>() > 0)
            fbxTexture = layeredTexture->GetSrcObject<FbxTexture>(0);
    }
    else
    {
        if (textureProperty.GetSrcObjectCount<FbxTexture>() > 0)
            textureProperty.GetSrcObject<FbxTexture>(0);
    }

    FbxFileTexture* fbxFileTexture = FbxCast<FbxFileTexture>(fbxTexture);
    if (fbxFileTexture)
        return fbxFileTexture->GetFileName();

    return nullptr;
}

FastName GenerateUID(const FbxNode* node)
{
    //return FastName(Format("%llu", node->GetUniqueID()).c_str());
    return FastName("node-" + String(node->GetName())); //Temporary in collada-style
}

FbxAMatrix GetGeometricTransform(const FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

const FbxSkeleton* GetSkeletonAttribute(const FbxNode* fbxNode)
{
    if (fbxNode == nullptr)
        return nullptr;

    int32 attrCount = fbxNode->GetNodeAttributeCount();
    for (int32 a = 0; a < attrCount; ++a)
    {
        const FbxNodeAttribute* attr = fbxNode->GetNodeAttributeByIndex(a);
        if (attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
            return static_cast<const FbxSkeleton*>(attr);
    }

    return nullptr;
}

const FbxSkeleton* FindSkeletonRoot(const FbxSkeleton* fbxSkeleton)
{
    if (fbxSkeleton == nullptr)
        return nullptr;

    while (fbxSkeleton != nullptr && !fbxSkeleton->IsSkeletonRoot())
    {
        fbxSkeleton = GetSkeletonAttribute(fbxSkeleton->GetNode()->GetParent());
    }

    return fbxSkeleton;
}

SkeletonComponent* ProcessSkin(FbxSkin* fbxSkin, Vector<FbxControlPointInfluences>* controlPointsInfluences, uint32* outMaxInfluenceCount)
{
    const FbxMesh* fbxMesh = static_cast<const FbxMesh*>(fbxSkin->GetGeometry());
    int32 clusterCount = fbxSkin->GetClusterCount();

    const FbxSkeleton* skeletonRoot = nullptr;
    for (int32 c = 0; c < clusterCount; ++c)
    {
        const FbxCluster* cluster = fbxSkin->GetCluster(c);
        const FbxSkeleton* linkedJoint = GetSkeletonAttribute(cluster->GetLink());

        skeletonRoot = FindSkeletonRoot(linkedJoint);
        if (skeletonRoot != nullptr)
            break;
    }

    DVASSERT(skeletonRoot != nullptr);
    const Vector<FBXJoint>& fbxJoints = linkedSkeletons[skeletonRoot];

    Map<const FbxSkeleton*, std::pair<Matrix4, Matrix4>> linksTransforms; // [bindTransform, bindTransformInv]
    Map<const FbxSkeleton*, AABBox3> linksBBox;

    FbxAMatrix linkTransform, nodeTransform;
    Matrix4 bindTransform, bindTransformInv;

    *outMaxInfluenceCount = 0;
    controlPointsInfluences->resize(fbxMesh->GetControlPointsCount());

    for (int32 c = 0; c < clusterCount; ++c)
    {
        FbxCluster* cluster = fbxSkin->GetCluster(c);
        const FbxSkeleton* linkedJoint = GetSkeletonAttribute(cluster->GetLink());

        if (skeletonRoot == nullptr)
            skeletonRoot = FindSkeletonRoot(linkedJoint);

        cluster->GetTransformMatrix(nodeTransform);
        cluster->GetTransformLinkMatrix(linkTransform);

        nodeTransform *= GetGeometricTransform(fbxMesh->GetNode());
        linkTransform *= GetGeometricTransform(cluster->GetLink());

        bindTransformInv = ToMatrix4(linkTransform.Inverse() * nodeTransform);
        bindTransform = ToMatrix4(cluster->GetLink()->EvaluateLocalTransform());

        if (cluster->GetLinkMode() != FbxCluster::eNormalize)
        {
            static const char* linkModes[] = { "Normalize", "Additive", "Total1" };
            Logger::Warning("[FBXImporter] Skin cluster linked with %s mode (node: %s)!", linkModes[cluster->GetLinkMode()], cluster->GetLink()->GetName());
        }

        int32 indicesCount = cluster->GetControlPointIndicesCount();
        for (int32 i = 0; i < indicesCount; ++i)
        {
            uint32 jointIndex = uint32(std::distance(fbxJoints.begin(), std::find_if(fbxJoints.begin(), fbxJoints.end(), [&linkedJoint](const FBXJoint& item) {
                                                         return (item.joint == linkedJoint);
                                                     })));

            int32 controlPointIndex = cluster->GetControlPointIndices()[i];
            float32 controlPointWeight = float32(cluster->GetControlPointWeights()[i]);

            controlPointsInfluences->at(controlPointIndex).emplace_back(jointIndex, controlPointWeight);
            *outMaxInfluenceCount = Max(*outMaxInfluenceCount, uint32(controlPointsInfluences->at(controlPointIndex).size()));

            if (controlPointWeight > EPSILON)
            {
                Vector3 vertexPosition = ToVector3(fbxMesh->GetControlPointAt(controlPointIndex));
                linksBBox[linkedJoint].AddPoint(vertexPosition * bindTransformInv);
            }
        }

        linksTransforms.emplace(linkedJoint, std::make_pair(bindTransform, bindTransformInv));
    }

    uint32 jointCount = uint32(fbxJoints.size());

    Vector<SkeletonComponent::Joint> joints(jointCount);
    for (uint32 j = 0; j < jointCount; ++j)
    {
        const FBXJoint& fbxJoint = fbxJoints[j];
        const FbxNode* jointNode = fbxJoint.joint->GetNode();

        SkeletonComponent::Joint& joint = joints[j];
        joint.parentIndex = fbxJoint.parentIndex;
        joint.uid = GenerateUID(jointNode);
        joint.name = FastName(jointNode->GetName());
        joint.bbox = (linksBBox.find(fbxJoint.joint) != linksBBox.end()) ? linksBBox[fbxJoint.joint] : AABBox3(Vector3(), 0.f);
        joint.bindTransform = linksTransforms[fbxJoint.joint].first;
        joint.bindTransformInv = linksTransforms[fbxJoint.joint].second;
    }

    SkeletonComponent* component = new SkeletonComponent();
    component->SetJoints(joints);

    return component;
}

}; //ns Details

}; //ns DAVA
