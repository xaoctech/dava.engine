#include "Classes/Export/Mitsuba/MitsubaExporter.h"
#include "Classes/Export/Mitsuba/Private/MitsubaExporterTools.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Scene/SceneEditor2.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/RefPtr.h>
#include <Base/String.h>
#include <FileSystem/FileSystem.h>
#include <Functional/Function.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/LandscapeThumbnails.h>
#include <Render/Highlevel/Light.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Material/NMaterial.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/LightComponent.h>

namespace MitsubaExporterDetail
{
struct TextureExport
{
    enum class Option : DAVA::uint32
    {
        ExportAsIs = 0,
        ReconstructZ,
        ExtractRoughness,
        ExtractMetallness
    };

    DAVA::Texture* sourceTexture = nullptr;
    DAVA::FilePath originalFilePath;
    DAVA::String destinationFilePath;
    Option option = Option::ExportAsIs;
};

struct MaterialExport
{
    DAVA::NMaterial* material = nullptr;
    DAVA::String diffuseTextureId;
    DAVA::String specularTextureId;
    DAVA::String nxnyTextureId;
    DAVA::String TextureId;
    DAVA::String bumpTextureId;
    DAVA::String roughnessTextureId;
    DAVA::String metallnessTextureId;
    DAVA::String materialType;
};

struct RenderBatchExport
{
    DAVA::RenderBatch* rb = nullptr;
    DAVA::Matrix4 transform;
    DAVA::String material;
};

struct LightExport
{
    DAVA::LightComponent* light = nullptr;
};

class Exporter
{
public:
    static const DAVA::uint32 MeshExporterVersion = 1;

    DAVA::String exportFolder;
    DAVA::String meshesFolder;
    DAVA::String texturesFolder;
    DAVA::UnorderedMap<DAVA::String, MitsubaExporterDetail::TextureExport> texturesToExport;
    DAVA::UnorderedMap<DAVA::String, MitsubaExporterDetail::MaterialExport> materialsToExport;
    DAVA::Vector<std::pair<DAVA::String, MitsubaExporterDetail::RenderBatchExport>> renderBatchesToExport;
    DAVA::UnorderedMap<DAVA::String, MitsubaExporterDetail::LightExport> lightsToExport;
    DAVA::Landscape* landscapeToExport = nullptr;

    void CollectExportObjects(const DAVA::Entity* entity);
    bool Export(DAVA::Scene*, const DAVA::FilePath&);

    void ExportTexture(const DAVA::String& name, const MitsubaExporterDetail::TextureExport& tex);
    void ExportMaterial(const DAVA::String& name, MitsubaExporterDetail::MaterialExport mat);
    void ExportLandscape(DAVA::Landscape* landscape);

    void ExportCamera(const DAVA::Camera* cam, const DAVA::Size2i& viewport);
    void ExportBatch(const DAVA::String& name, const MitsubaExporterDetail::RenderBatchExport& rb);
    void ExportLight(DAVA::LightComponent* light);

    bool RenderObjectsIsValidForExport(DAVA::RenderObject* object);
    bool RenderBatchIsValidForExport(DAVA::RenderBatch* batch, DAVA::int32 lodIndex, DAVA::int32 swIndex);
    bool MaterialIsValidForExport(DAVA::NMaterial* material);

    void CollectMaterialTextures(MitsubaExporterDetail::MaterialExport& material);
    bool AddAlbedoTextureToExport(DAVA::Texture*, DAVA::Vector<DAVA::String>& textureIds);
    bool AddNormalTextureToExport(DAVA::Texture*, DAVA::Vector<DAVA::String>& textureIds);
};

const bool ExportParametersFromMaterials = false;
static const DAVA::String FileExtension = ".xml";
void LandscapeThumbnailCallback(DAVA::String fileName, DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture);
}

void MitsubaExporter::PostInit()
{
    using namespace DAVA;

    QtAction* exportAction = new QtAction(GetAccessor(), "Export to Mitsuba...");
    FieldDescriptor fieldDescriptor(DAVA::ReflectedTypeDB::Get<SceneData>(), DAVA::FastName(SceneData::scenePropertyName));
    exportAction->SetStateUpdationFunction(QtAction::Enabled, fieldDescriptor, [](const DAVA::Any& value) -> DAVA::Any
                                           {
                                               return value.CanCast<SceneData::TSceneType>();
                                           });

    GetUI()->AddAction(DAVA::mainWindowKey, CreateMenuPoint("DebugFunctions"), exportAction);
    connections.AddConnection(exportAction, &QAction::triggered, DAVA::MakeFunction(this, &MitsubaExporter::Export));
}

void MitsubaExporter::Export()
{
    using namespace DAVA;

    FileDialogParams parameters;
    parameters.title = "Export to";
    parameters.filters = "XML Files (*.xml)";
    QString exportFolder = GetUI()->GetSaveFileName(DAVA::mainWindowKey, parameters);
    if (!exportFolder.isEmpty())
    {
        DAVA::FilePath exportFile(exportFolder.toUtf8().toStdString());
        if (exportFile.GetExtension() != MitsubaExporterDetail::FileExtension)
        {
            exportFile.ReplaceExtension(MitsubaExporterDetail::FileExtension);
        }

        MitsubaExporterDetail::Exporter exporter;
        SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
        exporter.Export(sceneData->GetScene().Get(), exportFile);
    }
}

bool MitsubaExporterDetail::Exporter::Export(DAVA::Scene* scene, const DAVA::FilePath& toFile)
{
    exportFolder = toFile.GetDirectory().GetAbsolutePathname();

    meshesFolder = toFile.GetBasename() + "_meshes";
    DAVA::FileSystem::Instance()->CreateDirectory(toFile.GetDirectory() + meshesFolder, true);

    texturesFolder = toFile.GetBasename() + "_textures";
    DAVA::FileSystem::Instance()->CreateDirectory(toFile.GetDirectory() + texturesFolder, true);

    CollectExportObjects(scene);

    if (renderBatchesToExport.empty())
        return false;

    std::ofstream fOut(toFile.GetAbsolutePathname());
    mitsuba::currentOutput = &fOut;
    {
        fOut << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
        mitsuba::scope sceneTag("scene", DAVA::String("version"), DAVA::String("0.5.0"));
        {
            mitsuba::scope integrator("integrator", mitsuba::kType, DAVA::String("direct"));
        }
        DAVA::Size2i vpSize(DAVA::Renderer::GetFramebufferWidth() - 6, DAVA::Renderer::GetFramebufferHeight() - 6);
        ExportCamera(scene->GetCurrentCamera(), vpSize);

        for (const auto& tex : texturesToExport)
        {
            ExportTexture(tex.first, tex.second);
        }

        for (const auto& lit : lightsToExport)
        {
            ExportLight(lit.second.light);
        }

        for (const auto& mat : materialsToExport)
        {
            ExportMaterial(mat.first, mat.second);
        }

        for (const auto& rb : renderBatchesToExport)
        {
            ExportBatch(rb.first, rb.second);
        }

        if (landscapeToExport != nullptr)
        {
            ExportLandscape(landscapeToExport);
        }
    }
    fOut.flush();
    fOut.close();
    mitsuba::currentOutput = nullptr;

    // hardcoded for now
    return true;
}

void MitsubaExporterDetail::Exporter::ExportCamera(const DAVA::Camera* cam, const DAVA::Size2i& viewport)
{
    DAVA::String origin = DAVA::Format("%f, %f, %f", cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z);
    DAVA::String target = DAVA::Format("%f, %f, %f", cam->GetTarget().x, cam->GetTarget().y, cam->GetTarget().z);
    DAVA::String up = DAVA::Format("%f, %f, %f", cam->GetUp().x, cam->GetUp().y, cam->GetUp().z);

    mitsuba::scope sensor("sensor", mitsuba::kType, DAVA::String("perspective"));
    mitsuba::tag(mitsuba::kFloat, mitsuba::kName, DAVA::String("fov"), mitsuba::kValue, cam->GetFOV());
    {
        mitsuba::scope sampler("sampler", mitsuba::kType, DAVA::String("ldsampler"));
        mitsuba::tag(mitsuba::kInteger, mitsuba::kName, DAVA::String("sampleCount"), mitsuba::kValue, 32);
    }
    {
        mitsuba::scope transform("transform", mitsuba::kName, DAVA::String("toWorld"));
        mitsuba::tag("lookat", DAVA::String("origin"), origin, DAVA::String("target"), target, DAVA::String("up"), up);
    }
    {
        mitsuba::scope film("film", mitsuba::kType, DAVA::String("hdrfilm"));
        mitsuba::tag(mitsuba::kInteger, mitsuba::kName, DAVA::String("width"), mitsuba::kValue, viewport.dx);
        mitsuba::tag(mitsuba::kInteger, mitsuba::kName, DAVA::String("height"), mitsuba::kValue, viewport.dy);
        {
            mitsuba::scope rfilter("rfilter", mitsuba::kType, DAVA::String("gaussian"));
            mitsuba::tag(mitsuba::kFloat, mitsuba::kName, DAVA::String("stddev"), mitsuba::kValue, 0.25f);
            /*
            mitsuba::tag(mitsuba::kFloat, mitsuba::kName, DAVA::String("exposure"), mitsuba::kValue, 0.0f);
            mitsuba::tag(mitsuba::kFloat, mitsuba::kName, DAVA::String("gamma"), mitsuba::kValue, -1.0f);
            mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("tonemapMethod"), mitsuba::kValue, DAVA::String("reinhard"));
            */
        }
    }
}

void MitsubaExporterDetail::Exporter::ExportBatch(const DAVA::String& name, const MitsubaExporterDetail::RenderBatchExport& rb)
{
    DAVA::String uniqueName = DAVA::Format("%s/%u_%s.obj", meshesFolder.c_str(), MeshExporterVersion, name.c_str(), reinterpret_cast<DAVA::uint64>(rb.rb));

    DAVA::String outFile = exportFolder + uniqueName;
    if (!DAVA::FileSystem::Instance()->Exists(outFile))
    {
        DAVA::PolygonGroup* poly = rb.rb->GetPolygonGroup();
        if (poly == nullptr)
        {
            DAVA::Logger::Info("Object %s does not contain polygon group.", name.c_str());
            return;
        }
        DAVA::int32 vertexCount = poly->GetVertexCount();
        DAVA::int32 vertexFormat = poly->GetFormat();

        if ((vertexFormat & DAVA::EVF_NORMAL) == 0)
        {
            DAVA::Logger::Error("%s data does not contain normals", name.c_str());
            return;
        }

        bool hasPivot = (vertexFormat & DAVA::EVF_PIVOT4) == DAVA::EVF_PIVOT4;
        if (hasPivot)
        {
            DAVA::Logger::Error("%s data contains pivot point, which are not supported", name.c_str());
        }

        bool hasTexCoords = (vertexFormat & DAVA::EVF_TEXCOORD0) == DAVA::EVF_TEXCOORD0;

        std::ofstream fOut(outFile.c_str(), std::ios::out);
        for (DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            DAVA::Vector3 v;
            poly->GetCoord(vertexIndex, v);
            fOut << DAVA::Format("v %.7f %.7f %.7f\n", v.x, v.y, v.z);

            DAVA::Vector3 n;
            poly->GetNormal(vertexIndex, n);
            fOut << DAVA::Format("vn %.7f %.7f %.7f\n", n.x, n.y, n.z);

            if (hasTexCoords)
            {
                DAVA::Vector2 t;
                poly->GetTexcoord(0, vertexIndex, t);
                fOut << DAVA::Format("vt %.7f %.7f\n", t.x, 1.0f - t.y);
            }
        }

        fOut << std::endl
             << DAVA::Format("g group_%llu_%llu", poly->GetNodeID(), reinterpret_cast<DAVA::uint64>(poly)) << std::endl;
        for (DAVA::int32 ti = 0, te = poly->GetPrimitiveCount(); ti < te; ++ti)
        {
            DAVA::int32 tri[3] = {};
            poly->GetIndex(3 * ti + 0, tri[0]);
            poly->GetIndex(3 * ti + 1, tri[1]);
            poly->GetIndex(3 * ti + 2, tri[2]);
            if (hasTexCoords)
            {
                fOut << "f " <<
                tri[0] + 1 << "/" << tri[0] + 1 << "/" << tri[0] + 1 << mitsuba::kSpace <<
                tri[1] + 1 << "/" << tri[1] + 1 << "/" << tri[1] + 1 << mitsuba::kSpace <<
                tri[2] + 1 << "/" << tri[2] + 1 << "/" << tri[2] + 1 << mitsuba::kSpace << std::endl;
            }
            else
            {
                fOut << "f " <<
                tri[0] + 1 << "//" << tri[0] + 1 << mitsuba::kSpace <<
                tri[1] + 1 << "//" << tri[1] + 1 << mitsuba::kSpace <<
                tri[2] + 1 << "//" << tri[2] + 1 << mitsuba::kSpace << std::endl;
            }
        }
        fOut.flush();
        fOut.close();
    }

    // write to Mitsuba xml
    mitsuba::scope shape("shape", mitsuba::kType, DAVA::String("obj"));
    mitsuba::tag("ref", mitsuba::kId, rb.material);
    mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("filename"), mitsuba::kValue, uniqueName);
    {
        const float* w = rb.transform.data;
        mitsuba::scope transform("transform", mitsuba::kName, DAVA::String("toWorld"));
        mitsuba::tag("matrix", mitsuba::kValue, DAVA::Format(
                                                "%.7f %.7f %.7f %.7f "
                                                "%.7f %.7f %.7f %.7f "
                                                "%.7f %.7f %.7f %.7f "
                                                "%.7f %.7f %.7f %.7f ",
                                                w[0], w[4], w[8], w[12],
                                                w[1], w[5], w[9], w[13],
                                                w[2], w[6], w[10], w[14],
                                                w[3], w[7], w[11], w[15])
                     );
    }
}

void MitsubaExporterDetail::Exporter::ExportLight(DAVA::LightComponent* light)
{
    DAVA::Light::eType lightType = static_cast<DAVA::Light::eType>(light->GetLightType());
    const DAVA::Color& clr = light->GetColor();

    if (lightType == DAVA::Light::eType::TYPE_SUN)
    {
        DAVA::Vector3 dir = light->GetDirection();
        mitsuba::scope emitter("emitter", mitsuba::kType, DAVA::String("directional"));
        mitsuba::tag("spectrum", mitsuba::kName, DAVA::String("irradiance"), mitsuba::kValue, DAVA::Format("%f, %f, %f", clr.r, clr.g, clr.b));
        mitsuba::tag("vector", mitsuba::kName, DAVA::String("direction"), DAVA::String("x"), dir.x, DAVA::String("y"), dir.y, DAVA::String("z"), dir.z);
    }
    else if (lightType == DAVA::Light::eType::TYPE_UNIFORM_COLOR)
    {
        mitsuba::scope emitter("emitter", mitsuba::kType, DAVA::String("constant"));
        mitsuba::tag("spectrum", mitsuba::kName, DAVA::String("radiance"), mitsuba::kValue, DAVA::Format("%f, %f, %f", clr.r, clr.g, clr.b));
    }
    else if (lightType == DAVA::Light::eType::TYPE_ENVIRONMENT_IMAGE)
    {
        DAVA::FilePath envMapPath = light->GetEnvironmentMap();
        envMapPath.ReplaceExtension(".hdr");
        mitsuba::scope emitter("emitter", mitsuba::kType, DAVA::String("envmap"));
        mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("filename"), mitsuba::kValue, envMapPath.GetAbsolutePathname());
        {
            mitsuba::scope transform("transform", mitsuba::kName, DAVA::String("toWorld"));
            mitsuba::tag("scale", DAVA::String("x"), -1, DAVA::String("y"), 1, DAVA::String("z"), -1);
            mitsuba::tag("rotate", DAVA::String("x"), 1, DAVA::String("angle"), 90);
            mitsuba::tag("rotate", DAVA::String("z"), 1, DAVA::String("angle"), -90);
        }
    }
    else if (lightType == DAVA::Light::eType::TYPE_POINT)
    {
        const DAVA::Vector3& pos = light->GetPosition();
        mitsuba::scope emitter("emitter", mitsuba::kType, DAVA::String("point"));
        mitsuba::tag("spectrum", mitsuba::kName, DAVA::String("intensity"), mitsuba::kValue, DAVA::Format("%f, %f, %f", clr.r, clr.g, clr.b));
        mitsuba::tag("point", mitsuba::kName, DAVA::String("position"), DAVA::String("x"), pos.x, DAVA::String("y"), pos.y, DAVA::String("z"), pos.z);
    }
    else
    {
        DAVA::Logger::Warning("Unsupported light type skipped");
    }
}

void MitsubaExporterDetail::Exporter::ExportLandscape(DAVA::Landscape* landscape)
{
    DAVA::Vector<DAVA::Landscape::LandscapeVertex> vertices;
    DAVA::Vector<DAVA::int32> indices;
    DAVA::Vector<DAVA::Vector3> normals;

    if (!landscape->GetLevel0Geometry(vertices, indices))
        return;

    DVASSERT(indices.size() % 3 == 0);

    DAVA::String landscapeTextureName = DAVA::Format("%s%s/landscape.png", exportFolder.c_str(), texturesFolder.c_str(), reinterpret_cast<DAVA::uint64>(landscape));
    auto callback = DAVA::Bind(&MitsubaExporterDetail::LandscapeThumbnailCallback, landscapeTextureName, std::placeholders::_1, std::placeholders::_2);
    DAVA::LandscapeThumbnails::Create(landscape, callback);

    {
        // compute landscape normals
        normals.resize(vertices.size());
        for (DAVA::size_type i = 0, e = indices.size() / 3; i < e; ++i)
        {
            DAVA::int32 i0 = indices.at(3 * i + 0);
            DAVA::int32 i1 = indices.at(3 * i + 1);
            DAVA::int32 i2 = indices.at(3 * i + 2);
            const DAVA::Vector3& p1 = vertices[i0].position;
            const DAVA::Vector3& p2 = vertices[i1].position;
            const DAVA::Vector3& p3 = vertices[i2].position;
            DAVA::Vector3 normal = (p2 - p1).CrossProduct(p3 - p1);
            DVASSERT(normal.Length() > std::numeric_limits<float>::epsilon(), "Landscape contains degenerate triangles");
            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }
        for (auto& n : normals)
        {
            n.Normalize();
        }
    }

    DAVA::String uniqueName = DAVA::Format("%s/landscape.obj", meshesFolder.c_str(), reinterpret_cast<DAVA::uint64>(landscape));
    DAVA::String outFile = exportFolder + uniqueName;
    std::ofstream fOut(outFile.c_str(), std::ios::out);
    for (const auto& v : vertices)
    {
        fOut << DAVA::Format("v %.7f %.7f %.7f\n", v.position.x, v.position.y, v.position.z);
        fOut << DAVA::Format("vt %.7f %.7f\n", v.texCoord.x, v.texCoord.y);
    }
    for (const auto& v : normals)
    {
        fOut << DAVA::Format("vn %.7f %.7f %.7f\n", v.x, v.y, v.z);
    }
    fOut << "\ng landscape\n";
    for (DAVA::uint64 i = 0, e = indices.size() / 3; i < e; ++i)
    {
        DAVA::int32 i0 = 1 + indices.at(3 * i + 0);
        DAVA::int32 i1 = 1 + indices.at(3 * i + 1);
        DAVA::int32 i2 = 1 + indices.at(3 * i + 2);
        fOut << DAVA::Format("f %d/%d/%d %d/%d/%d %d/%d/%d\n", i0, i0, i0, i1, i1, i1, i2, i2, i2);
    }
    fOut.flush();
    fOut.close();

    mitsuba::scope shape("shape", mitsuba::kType, DAVA::String("obj"));
    mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("filename"), mitsuba::kValue, uniqueName);
    {
        mitsuba::scope bsdf("bsdf", mitsuba::kType, DAVA::String("diffuse"));
        mitsuba::scope texture("texture", mitsuba::kType, DAVA::String("bitmap"), mitsuba::kName, DAVA::String("reflectance"));
        mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("filename"), mitsuba::kValue, landscapeTextureName);
    }
}

void MitsubaExporterDetail::Exporter::CollectExportObjects(const DAVA::Entity* entity)
{
    DAVA::RenderObject* renderObject = DAVA::GetRenderObject(entity);
    if (RenderObjectsIsValidForExport(renderObject))
    {
        if (renderObject->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE)
        {
            landscapeToExport = static_cast<DAVA::Landscape*>(renderObject);
        }
        else
        {
            for (DAVA::uint32 i = 0, e = renderObject->GetRenderBatchCount(); i < e; ++i)
            {
                DAVA::int32 lodIndex = -1;
                DAVA::int32 swIndex = -1;
                DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i, lodIndex, swIndex);
                if (RenderBatchIsValidForExport(batch, lodIndex, swIndex))
                {
                    DAVA::NMaterial* material = batch->GetMaterial();
                    DAVA::String materialID = DAVA::Format("mat_%p", material);

                    materialsToExport[materialID].material = material;
                    CollectMaterialTextures(materialsToExport[materialID]);

                    DAVA::String batchID = DAVA::Format("%s_batch_%u", entity->GetName().c_str(), i);
                    renderBatchesToExport.emplace_back(batchID, MitsubaExporterDetail::RenderBatchExport());
                    renderBatchesToExport.back().second.rb = batch;
                    renderBatchesToExport.back().second.material = materialID;
                    renderBatchesToExport.back().second.transform = entity->GetWorldTransform();
                }
            }
        }
    }

    DAVA::LightComponent* lightComponent = DAVA::GetLightComponent(entity);
    if (lightComponent != nullptr)
    {
        DAVA::String lightID = DAVA::Format("light_%p", lightComponent);
        lightsToExport[lightID].light = lightComponent;
    }

    for (DAVA::int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
    {
        CollectExportObjects(entity->GetChild(i));
    }
}

bool MitsubaExporterDetail::Exporter::RenderObjectsIsValidForExport(DAVA::RenderObject* object)
{
    static const DAVA::Set<DAVA::RenderObject::eType> validRenderObjectTypes =
    {
      DAVA::RenderObject::TYPE_RENDEROBJECT,
      DAVA::RenderObject::TYPE_MESH,
      DAVA::RenderObject::TYPE_LANDSCAPE,
      DAVA::RenderObject::TYPE_SPEED_TREE
    };

    return (object != nullptr) && (validRenderObjectTypes.count(object->GetType()) > 0);
}

bool MitsubaExporterDetail::Exporter::RenderBatchIsValidForExport(DAVA::RenderBatch* batch, DAVA::int32 lodIndex, DAVA::int32 swIndex)
{
    if ((batch == nullptr) || (lodIndex > 0) || (swIndex > 0))
        return false;

    return MaterialIsValidForExport(batch->GetMaterial());
}

bool MitsubaExporterDetail::Exporter::MaterialIsValidForExport(DAVA::NMaterial* material)
{
    if (material == nullptr)
        return false;

    if (material->GetEffectiveFXName().find("Sky") != DAVA::String::npos)
        return false;

    if (material->GetEffectiveFXName().find("Leaf") != DAVA::String::npos)
        return false;

    if (material->GetEffectiveFXName().find("Shadow") != DAVA::String::npos)
        return false;

    return true;
}

void MitsubaExporterDetail::Exporter::CollectMaterialTextures(MitsubaExporterDetail::MaterialExport& material)
{
    DAVA::FastName fxName = material.material->GetEffectiveFXName();
    if (fxName.find("PBS") == DAVA::String::npos)
    {
        DAVA::Logger::Warning("Non-PBS material skipped: %s", fxName.c_str());
        return;
    }

    DAVA::Texture* albedoTexture = material.material->GetEffectiveTexture(DAVA::FastName(mitsuba::kAlbedo.c_str()));
    DAVA::Texture* normalTexture = material.material->GetEffectiveTexture(DAVA::FastName(mitsuba::kNormalMap.c_str()));

    DAVA::Vector<DAVA::String> textureIds;
    if (AddAlbedoTextureToExport(albedoTexture, textureIds))
    {
        material.diffuseTextureId = textureIds.at(0);
        material.specularTextureId = textureIds.at(1);
    }

    textureIds.clear();
    if (AddNormalTextureToExport(normalTexture, textureIds))
    {
        DVASSERT(textureIds.size() == 3);
        material.bumpTextureId = textureIds.at(0);
        material.roughnessTextureId = textureIds.at(1);
        material.metallnessTextureId = textureIds.at(2);
    }
}

bool MitsubaExporterDetail::Exporter::AddAlbedoTextureToExport(DAVA::Texture* texture, DAVA::Vector<DAVA::String>& textureIds)
{
    if (texture == nullptr)
        return false;

    DAVA::FilePath sourceFile = texture->GetDescriptor()->GetSourceTexturePathname();
    if (DAVA::FileSystem::Instance()->Exists(sourceFile) == false)
        return false;

    DAVA::String baseName = sourceFile.GetFilename();
    DAVA::String textureId = DAVA::Format("%s/diffuse_%s.png", texturesFolder.c_str(), baseName.c_str());
    texturesToExport[textureId].sourceTexture = texture;
    texturesToExport[textureId].originalFilePath = sourceFile;
    texturesToExport[textureId].destinationFilePath = exportFolder + textureId;
    textureIds.emplace_back(textureId);

    textureId = DAVA::Format("%s/specular_%s.png", texturesFolder.c_str(), baseName.c_str());
    texturesToExport[textureId].sourceTexture = texture;
    texturesToExport[textureId].originalFilePath = sourceFile;
    texturesToExport[textureId].destinationFilePath = exportFolder + textureId;
    textureIds.emplace_back(textureId);

    return true;
}

bool MitsubaExporterDetail::Exporter::AddNormalTextureToExport(DAVA::Texture* texture, DAVA::Vector<DAVA::String>& textureIds)
{
    if (texture == nullptr)
        return false;

    DAVA::FilePath sourceFile = texture->GetDescriptor()->GetSourceTexturePathname();
    if (DAVA::FileSystem::Instance()->Exists(sourceFile) == false)
        return false;

    DAVA::String baseName = sourceFile.GetFilename();
    DAVA::String textureId;

    textureId = DAVA::Format("%s/%s-normal.png", texturesFolder.c_str(), baseName.c_str());
    texturesToExport[textureId].sourceTexture = texture;
    texturesToExport[textureId].originalFilePath = sourceFile;
    texturesToExport[textureId].destinationFilePath = exportFolder + textureId;
    texturesToExport[textureId].option = TextureExport::Option::ReconstructZ;
    textureIds.emplace_back(textureId);

    textureId = DAVA::Format("%s/%s-roughness.png", texturesFolder.c_str(), baseName.c_str());
    texturesToExport[textureId].sourceTexture = texture;
    texturesToExport[textureId].originalFilePath = sourceFile;
    texturesToExport[textureId].destinationFilePath = exportFolder + textureId;
    texturesToExport[textureId].option = TextureExport::Option::ExtractRoughness;
    textureIds.emplace_back(textureId);

    textureId = DAVA::Format("%s/%s-metallness.png", texturesFolder.c_str(), baseName.c_str());
    texturesToExport[textureId].sourceTexture = texture;
    texturesToExport[textureId].originalFilePath = sourceFile;
    texturesToExport[textureId].destinationFilePath = exportFolder + textureId;
    texturesToExport[textureId].option = TextureExport::Option::ExtractMetallness;
    textureIds.emplace_back(textureId);

    return true;
}

void ReconstructHeight(DAVA::uint8* inData, DAVA::int32 w, DAVA::int32 h)
{
    float* laplacian = reinterpret_cast<float*>(calloc(w * h, sizeof(float))); // Image1::createEmpty(w, h, WrapMode::ERROR);

    struct RGBA
    {
        DAVA::uint8 r, g, b, a;
    };
    RGBA* data = reinterpret_cast<RGBA*>(inData);
    auto sampleXY = [&](DAVA::int32 ax, DAVA::int32 ay) -> DAVA::Vector2
    {
        RGBA* sampled = data + (DAVA::Clamp(ay, 0, h - 1) * w + DAVA::Clamp(ax, 0, w - 1));
        return DAVA::Vector2(static_cast<float>(sampled->r), static_cast<float>(sampled->g));
        /*
        DAVA::Vector3 n;
        RGBA* sampled = data + (DAVA::Clamp(ay, 0, h - 1) * w + DAVA::Clamp(ax, 0, w - 1));
        n.x = 2.0f * static_cast<float>(sampled->r) / 256.0f - 1.0f;
        n.y = 2.0f * static_cast<float>(sampled->g) / 256.0f - 1.0f;
        n.z = std::sqrt(1.0f - std::min(1.0f, n.x*n.x + n.y*n.y));
        n.Normalize();
        return n;
        */
    };

    static const float signConvention = 1.0f;

    auto put = [w, h](float* buffer, DAVA::int32 x, DAVA::int32 y, float value)
    {
        x = DAVA::Clamp(x, 0, w - 1);
        y = DAVA::Clamp(y, 0, h - 1);
        *(buffer + y * w + x) = value;
    };

    auto get = [w, h](float* buffer, DAVA::int32 x, DAVA::int32 y)
    {
        x = DAVA::Clamp(x, 0, w - 1);
        y = DAVA::Clamp(y, 0, h - 1);
        return *(buffer + y * w + x);
    };

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            float ddx = sampleXY(x + 1, y).x - sampleXY(x - 1, y).x;
            float ddy = sampleXY(x, y + 1).y - sampleXY(x, y - 1).y;
            put(laplacian, x, y, (ddy - ddx) / 2.0f);
        }
    }

    float* src = reinterpret_cast<float*>(calloc(w * h, sizeof(float))); // Image1::createEmpty(w, h, WrapMode::ERROR);
    float* dst = reinterpret_cast<float*>(calloc(w * h, sizeof(float))); // Image1::createEmpty(w, h, WrapMode::ERROR);

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
            put(dst, x, y, 0.5);
    }

    // Number of Poisson iterations
    const int N = 128;
    for (int i = 0; i < N; ++i)
    {
        // Swap buffers
        float* tmp = src;
        src = dst;
        dst = tmp;
        for (int x = 0; x < w; ++x)
        {
            for (int y = 0; y < h; ++y)
            {
                float value = 0.25f * (get(src, x - 1, y) + get(src, x, y - 1) + get(src, x + 1, y) + get(src, x, y + 1) + get(laplacian, x, y));
                put(dst, x, y, value);
            }
        }
    }

    float lo = std::numeric_limits<float>::max();
    float hi = -lo;
    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            float v = get(dst, x, y);
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
    }

    bool isFlat = std::abs(hi - lo) <= std::numeric_limits<float>::epsilon();
    for (DAVA::int32 y = 0; y < h; ++y)
    {
        for (DAVA::int32 x = 0; x < w; ++x)
        {
            float h = get(dst, x, y);
            float height = isFlat ? 0.0 : DAVA::Clamp(1.0f - 0.25f * (h - lo) / (hi - lo), 0.0f, 1.0f);
            (data + y * w + x)->r = static_cast<DAVA::uint8>(255.0f * height);
            (data + y * w + x)->g = static_cast<DAVA::uint8>(255.0f * height);
            (data + y * w + x)->b = static_cast<DAVA::uint8>(255.0f * height);
            (data + y * w + x)->a = 255;
        }
    }
}

void MitsubaExporterDetail::Exporter::ExportTexture(const DAVA::String& name, const MitsubaExporterDetail::TextureExport& tex)
{
    if (!DAVA::FileSystem::Instance()->Exists(tex.destinationFilePath))
    {
        struct RGBA
        {
            DAVA::uint8 r, g, b, a;
        };
        DAVA::ScopedPtr<DAVA::Image> image(DAVA::ImageSystem::LoadSingleMip(tex.originalFilePath));
        RGBA* data = reinterpret_cast<RGBA*>(image->GetData());

        switch (tex.option)
        {
        case TextureExport::Option::ReconstructZ:
        {
            DVASSERT(image->GetPixelFormat() == DAVA::PixelFormat::FORMAT_RGBA8888);
            DVASSERT(image->GetDataSize() % 4 == 0);
            ReconstructHeight(image->GetData(), image->GetWidth(), image->GetHeight());
            break;
        }
        case TextureExport::Option::ExtractRoughness:
        {
            DVASSERT(image->GetPixelFormat() == DAVA::PixelFormat::FORMAT_RGBA8888);
            DVASSERT(image->GetDataSize() % 4 == 0);
            RGBA* data = reinterpret_cast<RGBA*>(image->GetData());
            for (DAVA::uint32 y = 0; y < image->GetHeight(); ++y)
            {
                for (DAVA::uint32 x = 0; x < image->GetWidth(); ++x)
                {
                    data->r = data->b;
                    data->g = 0;
                    data->b = 0;
                    data->a = 255;
                    ++data;
                }
            }
            break;
        }
        case TextureExport::Option::ExtractMetallness:
        {
            DVASSERT(image->GetPixelFormat() == DAVA::PixelFormat::FORMAT_RGBA8888);
            DVASSERT(image->GetDataSize() % 4 == 0);
            RGBA* data = reinterpret_cast<RGBA*>(image->GetData());
            for (DAVA::uint32 y = 0; y < image->GetHeight(); ++y)
            {
                for (DAVA::uint32 x = 0; x < image->GetWidth(); ++x)
                {
                    data->r = data->a;
                    data->g = 0;
                    data->b = 0;
                    data->a = 255;
                    ++data;
                }
            }
            break;
        }
        default:
            break;
        }

        DAVA::ImageSystem::Save(tex.destinationFilePath, image);
    }

    mitsuba::scope texture("texture", mitsuba::kType, DAVA::String("bitmap"), mitsuba::kId, name);
    mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("filename"), mitsuba::kValue, tex.destinationFilePath);
    if ((tex.option == TextureExport::Option::ExtractRoughness) || (tex.option == TextureExport::Option::ExtractMetallness))
    {
        mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("channel"), mitsuba::kValue, DAVA::String("r"));
    }
}

void MitsubaExporterDetail::Exporter::ExportMaterial(const DAVA::String& name, MitsubaExporterDetail::MaterialExport mtl)
{
    const DAVA::float32* alphaValue = mtl.material->GetEffectivePropValue(DAVA::FastName("roughnessScale"));
    const DAVA::float32* metallnessValue = mtl.material->GetEffectivePropValue(DAVA::FastName("metallnessScale"));
    float alpha = alphaValue ? *alphaValue : 1.0f;
    float metallness = metallnessValue ? *metallnessValue : 1.0f;

    mitsuba::scope bump("bsdf", mitsuba::kType, mitsuba::kBumpmap, mitsuba::kId, name);
    mitsuba::tag("ref", mitsuba::kName, DAVA::String("scale"), mitsuba::kId, mtl.bumpTextureId);

    {
        mitsuba::scope bump("bsdf", mitsuba::kType, DAVA::String("blendbsdf"));
        if (ExportParametersFromMaterials)
        {
            mitsuba::tag("float", mitsuba::kName, DAVA::String("weight"), mitsuba::kValue, metallness);
        }
        else
        {
            mitsuba::tag("ref", mitsuba::kName, DAVA::String("weight"), mitsuba::kId, mtl.metallnessTextureId);
        }

        {
            mitsuba::scope conductor("bsdf", mitsuba::kType, mitsuba::kRoughPlastic);
            mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("distribution"), mitsuba::kValue, DAVA::String("ggx"));
            mitsuba::tag("ref", mitsuba::kName, mitsuba::kDiffuseReflectance, mitsuba::kId, mtl.diffuseTextureId);
            mitsuba::tag("ref", mitsuba::kName, mitsuba::kSpecularReflectance, mitsuba::kId, mtl.specularTextureId);
            if (ExportParametersFromMaterials)
            {
                mitsuba::tag("float", mitsuba::kName, DAVA::String("alpha"), mitsuba::kValue, alpha);
            }
            else
            {
                mitsuba::tag("ref", mitsuba::kName, DAVA::String("alpha"), mitsuba::kId, mtl.roughnessTextureId);
            }
        }
        {
            mitsuba::scope conductor("bsdf", mitsuba::kType, mitsuba::kRoughConductor);
            mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("distribution"), mitsuba::kValue, DAVA::String("ggx"));
            mitsuba::tag(mitsuba::kString, mitsuba::kName, DAVA::String("material"), mitsuba::kValue, DAVA::String("Ag"));
            mitsuba::tag("ref", mitsuba::kName, mitsuba::kSpecularReflectance, mitsuba::kId, mtl.specularTextureId);
            if (ExportParametersFromMaterials)
            {
                mitsuba::tag("float", mitsuba::kName, DAVA::String("alpha"), mitsuba::kValue, alpha);
            }
            else
            {
                mitsuba::tag("ref", mitsuba::kName, DAVA::String("alpha"), mitsuba::kId, mtl.roughnessTextureId);
            }
        }
    }
}

void MitsubaExporterDetail::LandscapeThumbnailCallback(DAVA::String fileName, DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture)
{
    DAVA::ScopedPtr<DAVA::Image> image(landscapeTexture->CreateImageFromMemory());
    DAVA::ImageSystem::Save(fileName, image);
}

DECL_TARC_MODULE(MitsubaExporter);