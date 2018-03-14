#include "REPlatform/DataNodes/ProjectResources.h"
#include "REPlatform/DataNodes/ProjectManagerData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlParser.h>
#include <Render/Material/FXAsset.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>
#include <Sound/SoundSystem.h>

namespace DAVA
{
namespace ProjectResourcesDetails
{
Vector<String> LoadMaterialQualities(FilePath fxPath)
{
    Vector<String> qualities;
    ScopedPtr<YamlParser> parser(YamlParser::Create(fxPath));
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode)
        {
            const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
            if (materialTemplateNode)
            {
                static const char* QUALITIES[] = { "LOW", "MEDIUM", "HIGH", "ULTRA_HIGH" };
                for (const char* quality : QUALITIES)
                {
                    const YamlNode* qualityNode = materialTemplateNode->Get(quality);
                    if (qualityNode)
                    {
                        qualities.push_back(quality);
                    }
                }
            }
        }
    }

    return qualities;
}

static const FilePath PATH_TO_ASSIGNABLE_YAML = "~res:/assignable-materials.yaml"; // file that contains info about available materials

void LoadMaterialTemplatesInfo(Vector<MaterialTemplateInfo>& templates)
{
    templates.clear();

    if (FileSystem::Instance()->Exists(PATH_TO_ASSIGNABLE_YAML))
    {
        ScopedPtr<YamlParser> parser(YamlParser::Create(PATH_TO_ASSIGNABLE_YAML));
        YamlNode* rootNode = parser->GetRootNode();

        if (nullptr != rootNode && rootNode->GetType() == YamlNode::TYPE_MAP)
        {
            FilePath materialsListDir = PATH_TO_ASSIGNABLE_YAML.GetDirectory();

            for (uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const YamlNode* typeNode = rootNode->Get(i);
                if (nullptr != typeNode)
                {
                    const String& typeName = rootNode->GetItemKeyName(i);
                    DAVA::int32 typeValue = -1;
                    if (GlobalEnumMap<DAVA::FXDescriptor::eType>::Instance()->ToValue(typeName.c_str(), typeValue))
                    {
                        DAVA::FXDescriptor::eType materialType = DAVA::FXDescriptor::eType(typeValue);

                        for (uint32 t = 0; t < typeNode->GetCount(); ++t)
                        {
                            const YamlNode* templateNode = typeNode->Get(t);

                            const YamlNode* name = templateNode->Get("name");
                            const YamlNode* path = templateNode->Get("path");

                            if (nullptr != name && nullptr != path &&
                                name->GetType() == YamlNode::TYPE_STRING &&
                                path->GetType() == YamlNode::TYPE_STRING)
                            {
                                const FilePath templatePath = materialsListDir + path->AsString();
                                if (FileSystem::Instance()->Exists(templatePath))
                                {
                                    MaterialTemplateInfo info;

                                    info.name = name->AsString().c_str();
                                    info.path = templatePath.GetFrameworkPath().c_str();
                                    info.qualities = ProjectResourcesDetails::LoadMaterialQualities(templatePath);
                                    info.materialType = materialType;

                                    templates.push_back(info);
                                }
                                else
                                {
                                    Logger::Warning("assignable material template not found at %s", templatePath.GetAbsolutePathname().c_str());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
}

ProjectResources::ProjectResources(ContextAccessor* accessor)
    : accessor(accessor)
{
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<ProjectManagerData>());
}

ProjectResources::~ProjectResources()
{
    UnloadProject();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->DeleteData<ProjectManagerData>();
}

ProjectManagerData* ProjectResources::GetProjectManagerData()
{
    DataContext* ctx = accessor->GetGlobalContext();
    return ctx->GetData<ProjectManagerData>();
}

void ProjectResources::LoadProject(const FilePath& incomePath)
{
    ProjectManagerData* data = GetProjectManagerData();

    if (incomePath.IsDirectoryPathname() && incomePath != data->projectPath)
    {
        UnloadProject();

        data->projectPath = incomePath;
        FilePath::AddResourcesFolder(data->GetDataSourcePath());
        FilePath::AddTopResourcesFolder(data->GetDataPath());

        ProjectResourcesDetails::LoadMaterialTemplatesInfo(data->materialTemplatesInfo);

        QualitySettingsSystem::Instance()->Load(data->projectPath + "DataSource/quality.yaml");

        const EngineContext* engineCtx = accessor->GetEngineContext();
        engineCtx->soundSystem->InitFromQualitySettings();
    }
}

void ProjectResources::UnloadProject()
{
    ProjectManagerData* data = GetProjectManagerData();

    if (!data->projectPath.IsEmpty())
    {
        const EngineContext* engineCtx = accessor->GetEngineContext();
        engineCtx->soundSystem->UnloadFMODProjects();

        FilePath::RemoveResourcesFolder(data->GetDataPath());
        FilePath::RemoveResourcesFolder(data->GetDataSourcePath());
        data->projectPath = "";
    }
}
} // namespace DAVA
