#include "Classes/Application/SettingsConverter.h"
#include "Classes/EditorSystems/ControlTransformationSettings.h"
#include "Classes/EditorSystems/UserAssetsSettings.h"
#include "Classes/EditorSystems/EditorControlsView.h"
#include "Classes/EditorSystems/PixelGrid.h"
#include "Classes/EditorSystems/SelectionSystem.h"

#include "Classes/Modules/PreferencesModule/PreferencesData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/SharedModules/ThemesModule/ThemesModule.h>
#include <TArc/Qt/QtByteArray.h>

#include <Engine/PlatformApiQt.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/KeyedArchive.h>
#include <Logger/Logger.h>
#include <Base/RefPtr.h>

namespace SettingsConverterDetail
{
const DAVA::String versionPropertyKey = "settingsVersion";
const DAVA::uint32 INIT_SETTINGS_VERSION = 1;
const DAVA::uint32 CURRENT_VERSION = INIT_SETTINGS_VERSION;

class OldSettingsConverter
{
public:
    OldSettingsConverter(DAVA::TArc::ContextAccessor* accessor)
        : archive(new DAVA::KeyedArchive())
    {
        DAVA::FileSystem* fs = accessor->GetEngineContext()->fileSystem;
        DAVA::FilePath localPrefrencesPath(fs->GetCurrentDocumentsDirectory() + "QuickEdSettings.archive");
        if (archive->Load(localPrefrencesPath))
        {
            std::function<void(DAVA::KeyedArchive*, DAVA::Vector<DAVA::String> & spacing)> dumpFn = [&dumpFn, this](DAVA::KeyedArchive* ar, DAVA::Vector<DAVA::String>& path)
            {
                const DAVA::KeyedArchive::UnderlyingMap& map = ar->GetArchieveData();
                for (const auto& node : map)
                {
                    if (node.second->GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
                    {
                        DAVA::KeyedArchive* archive = node.second->AsKeyedArchive();
                        path.push_back(node.first);
                        unpackedArchive.emplace(path, archive);
                        dumpFn(archive, path);
                        path.pop_back();
                    }
                }
            };

            DAVA::Vector<DAVA::String> path;
            dumpFn(archive.Get(), path);
        }
    }

    void Do(const DAVA::TArc::PropertiesHolder& rootNode, DAVA::TArc::ContextAccessor* accessor)
    {
        if (unpackedArchive.empty() == true)
        {
            return;
        }

        ControlTransformationSettings* controlTransformation = accessor->GetGlobalContext()->GetData<ControlTransformationSettings>();
        DVASSERT(controlTransformation);

        PreviewWidgetSettings* previewWidgetSettings = accessor->GetGlobalContext()->GetData<PreviewWidgetSettings>();
        DVASSERT(previewWidgetSettings);

        PixelGridPreferences* pixelGridPreferences = accessor->GetGlobalContext()->GetData<PixelGridPreferences>();
        DVASSERT(pixelGridPreferences);

        SelectionSettings* selectionSettings = accessor->GetGlobalContext()->GetData<SelectionSettings>();
        DVASSERT(selectionSettings);

        UserAssetsSettings* userAssetsSettings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();
        DVASSERT(userAssetsSettings);

        PreferencesData* preferencesData = accessor->GetGlobalContext()->GetData<PreferencesData>();
        DVASSERT(preferencesData);

        DAVA::TArc::ThemesSettings* themeSettings = accessor->GetGlobalContext()->GetData<DAVA::TArc::ThemesSettings>();
        DVASSERT(themeSettings);

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "ColorControl" });
            previewWidgetSettings->backgroundColor0 = archive->GetColor("backgroundColor0");
            previewWidgetSettings->backgroundColor1 = archive->GetColor("backgroundColor1");
            previewWidgetSettings->backgroundColor2 = archive->GetColor("backgroundColor2");
            previewWidgetSettings->backgroundColorIndex = archive->GetUInt32("backgroundColorIndex");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "SelectionSystem" });
            selectionSettings->canFindCommonForSelection = archive->GetBool("canFindCommonForSelection");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "GuidesControllerPreferences" });
            userAssetsSettings->guidesColor = archive->GetColor("guideColor");
            userAssetsSettings->previewGuideColor = archive->GetColor("previewGuideColor");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "HUDControlsPreferences" });
            userAssetsSettings->pivotPointPath2 = archive->GetString("pivotPointPath2");
            userAssetsSettings->borderRectPath2 = archive->GetString("borderRectPath2");
            userAssetsSettings->selectionRectColor = archive->GetColor("selectionRectColor");
            userAssetsSettings->cornerRectPath2 = archive->GetString("cornerRectPath2");
            userAssetsSettings->hudRectColor = archive->GetColor("hudRectColor");
            userAssetsSettings->highlightColor = archive->GetColor("highlightColor");
            userAssetsSettings->rotatePath2 = archive->GetString("rotatePath2");
            userAssetsSettings->magnetLinePath2 = archive->GetString("magnetLinePath2");
            userAssetsSettings->magnetRectPath2 = archive->GetString("magnetRectPath2");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "HUDSystem" });
            controlTransformation->minimumSelectionRectSize = archive->GetVector2("minimumSelectionRectSize");
            controlTransformation->showPivot = archive->GetBool("showPivot");
            controlTransformation->showRotate = archive->GetBool("showRotate");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "EditorTransformSystem" });
            controlTransformation->angleSegment = archive->GetFloat("angleSegment");
            controlTransformation->shareOfSizeToMagnetPivot = archive->GetVector2("shareOfSizeToMagnetPivot");
            controlTransformation->canMagnet = archive->GetBool("canMagnet");
            controlTransformation->expandedmoveStepByKeyboard2 = archive->GetVector2("expandedmoveStepByKeyboard2");
            controlTransformation->moveMagnetRange = archive->GetVector2("moveMagnetRange");
            controlTransformation->shiftInverted = archive->GetBool("shiftInverted");
            controlTransformation->moveStepByKeyboard2 = archive->GetVector2("moveStepByKeyboard2");
            controlTransformation->pivotMagnetRange = archive->GetVector2("pivotMagnetRange");
            controlTransformation->resizeMagnetRange = archive->GetVector2("resizeMagnetRange");
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "PixelGridPreferences" });
            if (archive != nullptr)
            {
                pixelGridPreferences->gridColor = archive->GetColor("gridColor");
                pixelGridPreferences->scaleToDisplay = archive->GetFloat("scaleToDisplay");
                pixelGridPreferences->isVisible = archive->GetBool("isVisible");
            }
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "FindFileDialog" });
            accessor->CreatePropertiesNode("FindFileDialog").Set("lastUsedPath", archive->GetString("lastUsedPath"));
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "preferences", "DialogReloadSprites" });
            DAVA::TArc::PropertiesItem item = accessor->CreatePropertiesNode("DialogReloadSprites");
            item.Set("currentGpu", archive->GetVariant("currentGPU")->AsUInt8());
            item.Set("quality", archive->GetUInt32("quality"));
            item.Set("forcreRepack", archive->GetBool("forceRepackEnabled"));
            item.Set("showConsole", archive->GetBool("consoleVisible"));

            DAVA::String consoleStateBase64 = archive->GetString("consoleState");
            QByteArray consoleState = QByteArray::fromBase64(QByteArray(consoleStateBase64.c_str(), static_cast<int>(consoleStateBase64.size())));
            item.Set("consoleState", consoleState);
        }

        {
            DAVA::KeyedArchive* archive = GetArchive({ "unnamed preferences" });
            themeSettings->SetTheme(static_cast<DAVA::TArc::ThemesSettings::eTheme>(archive->GetInt64("ThemeName")), DAVA::PlatformApi::Qt::GetApplication());
        }
    }

    DAVA::KeyedArchive* GetArchive(const DAVA::Vector<DAVA::String>& path)
    {
        auto iter = unpackedArchive.find(path);
        if (iter != unpackedArchive.end())
        {
            return iter->second;
        }
        return nullptr;
    }

private:
    DAVA::RefPtr<DAVA::KeyedArchive> archive;
    DAVA::Map<DAVA::Vector<DAVA::String>, DAVA::KeyedArchive*> unpackedArchive;
};

void ConvertToInitVersion(const DAVA::TArc::PropertiesHolder& rootNode, DAVA::TArc::ContextAccessor* accessor)
{
    OldSettingsConverter converter(accessor);
    converter.Do(rootNode, accessor);
}
} // namespace SettingsConverterDetail

void ConvertSettingsIfNeeded(const DAVA::TArc::PropertiesHolder& rootNode, DAVA::TArc::ContextAccessor* accessor)
{
    using namespace SettingsConverterDetail;
    DAVA::uint32 settingsVersion = 0;
    {
        DAVA::TArc::PropertiesItem verionsInfo = rootNode.CreateSubHolder("VersionsInfo");
        settingsVersion = verionsInfo.Get<DAVA::uint32>("SettingsVersion", 0);
    }
    if (settingsVersion == 0)
    {
        ConvertToInitVersion(rootNode, accessor);
    }

    rootNode.CreateSubHolder("VersionsInfo").Set("SettingsVersion", CURRENT_VERSION);
}
