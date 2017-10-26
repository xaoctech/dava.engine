#include "DockSceneTree/SceneTree.h"
#include "Main/mainwindow.h"
#include "StringConstants.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QMenu>
#include <QDebug>
#include <QSignalBlocker>

#include "Classes/Application/RESettings.h"
#include "Deprecated/SceneValidator.h"
#include "Main/QtUtils.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneImageGraber.h"
#include "Qt/GlobalOperations.h"
#include "Qt/Tools/PathDescriptor/PathDescriptor.h"

#include "QtTools/FileDialogs/FileDialog.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

// commands
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "Commands2/Base/RECommandNotificationObject.h"

#include "Classes/Qt/Actions/SaveEntityAsAction.h"
#include "Classes/Qt/Scene/SceneHelper.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"

#include <QtTools/ConsoleWidget/PointerSerializer.h>
#include <QtTools/Updaters/LazyUpdater.h>

#include <TArc/Utils/Utils.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/ScopedValueGuard.h>
#include <TArc/Core/FieldBinder.h>

#include <FileSystem/VariantType.h>

#include <QShortcut>
#include "SlotSupportModule/Private/EditorSlotSystem.h"
#include "SlotSupportModule/SlotSystemSettings.h"

namespace SceneTreeDetails
{
QString GetParticlesConfigPath()
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    if (data == nullptr)
        return QString("");

    return QString::fromStdString(data->GetParticlesConfigPath().GetAbsolutePathname());
}

DAVA::FilePath GetDataSourcePath()
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    if (data == nullptr)
        return DAVA::FilePath();

    return data->GetDataSource3DPath();
}

void SaveEmitter(SceneEditor2* scene, DAVA::ParticleEffectComponent* component, DAVA::ParticleEmitter* emitter,
                 bool askFileName, const QString& defaultName, const DAVA::Function<std::unique_ptr<DAVA::Command>(const DAVA::FilePath&)>& commandCreator)
{
    askFileName |= emitter->configPath.IsEmpty();

    DAVA::FilePath yamlPath = emitter->configPath;
    if (askFileName)
    {
        CommonInternalSettings* settings = REGlobal::GetGlobalContext()->GetData<CommonInternalSettings>();
        DAVA::FilePath defaultPath = settings->emitterSaveDir;
        QString particlesPath = defaultPath.IsEmpty() ? GetParticlesConfigPath() : QString::fromStdString(defaultPath.GetAbsolutePathname());

        DAVA::FileSystem::Instance()->CreateDirectory(DAVA::FilePath(particlesPath.toStdString()), true); //to ensure that folder is created

        QString emitterPathname = particlesPath + defaultName;
        QString title = QStringLiteral("Save Particle Emitter ") + QString(emitter->name.c_str());
        QString filePath = FileDialog::getSaveFileName(nullptr, title, emitterPathname, QStringLiteral("YAML File (*.yaml)"));

        if (filePath.isEmpty())
        {
            return;
        }

        yamlPath = DAVA::FilePath(filePath.toStdString());
        settings->emitterSaveDir = yamlPath.GetDirectory();
    }

    scene->Exec(commandCreator(yamlPath));
    if (askFileName)
    {
        scene->SetChanged();
    }
}
}

class SceneTree::BaseContextMenu : public QObject
{
public:
    BaseContextMenu(SceneTree* treeWidget_)
        : treeWidget(treeWidget_)
    {
    }

    void Show(const QPoint& pos)
    {
        if (!IsValidForShow())
            return;

        QMenu menu;
        FillActions(menu);
        menu.exec(pos);
    }

    bool IsStructureChanged() const
    {
        return isStructureChanged;
    }

protected:
    virtual bool IsValidForShow() const
    {
        return GetScene() != nullptr;
    }

    virtual void FillActions(QMenu& menu) = 0;

    template <typename Func>
    void Connect(QAction* action, const typename QtPrivate::FunctionPointer<Func>::Object* receiver, Func slot)
    {
        const auto connectResult = QObject::connect(action, &QAction::triggered, receiver, slot);
        DVASSERT(connectResult);
    }

    template <typename Func>
    void Connect(QAction* action, Func slot)
    {
        const auto connectResult = QObject::connect(action, &QAction::triggered, slot);
        DVASSERT(connectResult);
    }

    void MarkStructureChanged()
    {
        GetScene()->SetChanged();
        isStructureChanged = true;
    }

    SceneEditor2* GetScene() const
    {
        return GetSceneModel()->GetScene();
    }

    SceneTreeModel* GetSceneModel() const
    {
        return treeWidget->treeModel;
    }

    SceneTreeFilteringModel* GetFilteredModel() const
    {
        return treeWidget->filteringProxyModel;
    }

    struct RemoveInfo
    {
        RemoveInfo(std::unique_ptr<DAVA::Command>&& command_, const DAVA::Any& selectedObject_)
            : command(std::move(command_))
            , selectedObject(selectedObject_)
        {
        }

        RemoveInfo(RemoveInfo&& info)
            : command(std::move(info.command))
            , selectedObject(info.selectedObject)
        {
        }

        std::unique_ptr<DAVA::Command> command;
        DAVA::Any selectedObject;
    };

    void RemoveCommandsHelper(const DAVA::String& text, SceneTreeItem::eItemType type, const DAVA::Function<RemoveInfo(SceneTreeItem*)>& callback)
    {
        DAVA::Vector<std::unique_ptr<DAVA::Command>> commands;
        commands.reserve(GetSelectedItemsCount());

        QSet<QModelIndex> selectedIndecies;
        foreach (QModelIndex index, treeWidget->selectionModel()->selectedRows())
        {
            selectedIndecies.insert(treeWidget->filteringProxyModel->mapToSource(index));
        }

        foreach (QModelIndex srcIndex, selectedIndecies)
        {
            SceneTreeItem* item = treeWidget->treeModel->GetItem(srcIndex);
            DVASSERT(item != nullptr);

            bool parentSelected = false;

            QModelIndex srcIndexParent = srcIndex.parent();

            while (srcIndexParent.isValid() && !parentSelected)
            {
                SceneTreeItem* itemParent = treeWidget->treeModel->GetItem(srcIndexParent);

                if (selectedIndecies.contains(srcIndexParent) && itemParent->ItemType() == type)
                {
                    parentSelected = true;
                    break;
                }

                srcIndexParent = srcIndexParent.parent();
            }

            if (!parentSelected && static_cast<SceneTreeItem::eItemType>(item->ItemType()) == type)
            {
                RemoveInfo info = callback(item);
                commands.push_back(std::move(info.command));
            }
        }

        if (!commands.empty())
        {
            SceneEditor2* sceneEditor = GetScene();
            sceneEditor->BeginBatch(text, static_cast<DAVA::uint32>(commands.size()));

            static_cast<SceneTree*>(GetParentWidget())->SyncSelectionToTree();
            for (std::unique_ptr<DAVA::Command>& command : commands)
            {
                sceneEditor->Exec(std::move(command));
            }
            MarkStructureChanged();
            sceneEditor->EndBatch();
        }

        treeWidget->SyncSelectionFromTree();
    }

    void ForEachSelectedByType(SceneTreeItem::eItemType type, const DAVA::Function<void(SceneTreeItem*)>& callback)
    {
        foreach (QModelIndex index, treeWidget->selectionModel()->selectedRows())
        {
            QModelIndex srcIndex = treeWidget->filteringProxyModel->mapToSource(index);
            SceneTreeItem* item = treeWidget->treeModel->GetItem(srcIndex);
            DVASSERT(item != nullptr);

            if (static_cast<SceneTreeItem::eItemType>(item->ItemType()) == type)
            {
                callback(item);
            }
        }
    }

    DAVA::uint32 GetSelectedItemsCount() const
    {
        return treeWidget->selectionModel()->selectedRows().size();
    }

    SceneTree* GetParentWidget()
    {
        return treeWidget;
    }

private:
    SceneTree* treeWidget;
    bool isStructureChanged = false;
};

class SceneTree::EntityContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    EntityContextMenu(SceneTreeItemEntity* item, SceneTree* treeWidget)
        : TBase(treeWidget)
        , entityItem(item)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && entityItem != nullptr;
    }

    void FillActions(QMenu& menu) override
    {
        using namespace DAVA::TArc;
        SceneEditor2* scene = GetScene();

        const SelectableGroup& selection = Selection::GetSelection();
        DAVA::uint32 selectionSize = selection.GetSize();

        DAVA::Entity* entity = entityItem->GetEntity();
        DAVA::Camera* camera = GetCamera(entityItem->GetEntity());

        if (GetSceneModel()->GetCustomFlags(entityItem->index()) & SceneTreeModel::CF_Disabled)
        {
            if (selectionSize == 1 && camera != nullptr)
            {
                FillCameraActions(menu);
                menu.addSeparator();
            }

            if ((camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
            {
                Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), QStringLiteral("Remove entity")), [scene] { ::RemoveSelection(scene); });
            }
        }
        else
        {
            Connect(menu.addAction(SharedIcon(":/QtIcons/zoom.png"), QStringLiteral("Look at")), [scene] { LookAtSelection(scene); });
            if (camera != nullptr)
            {
                FillCameraActions(menu);
            }

            menu.addSeparator();
            if (entity->GetLocked() == false && (camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
            {
                Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), QStringLiteral("Remove entity")), [scene] { ::RemoveSelection(scene); });
            }

            menu.addSeparator();
            QAction* lockAction = menu.addAction(SharedIcon(":/QtIcons/lock_add.png"), QStringLiteral("Lock"));
            QAction* unlockAction = menu.addAction(SharedIcon(":/QtIcons/lock_delete.png"), QStringLiteral("Unlock"));
            Connect(lockAction, [scene] { LockTransform(scene); });
            Connect(unlockAction, [scene] { UnlockTransform(scene); });
            if (entity->GetLocked())
            {
                lockAction->setDisabled(true);
            }
            else
            {
                unlockAction->setDisabled(true);
            }

            menu.addSeparator();
            Connect(menu.addAction(SharedIcon(":/QtIcons/save_as.png"), QStringLiteral("Save Entity As...")), this, &EntityContextMenu::SaveEntityAs);

            DAVA::KeyedArchive* customProp = GetCustomPropertiesArchieve(entity);
            bool isConstReference = false;
            if (nullptr != customProp)
            {
                DAVA::FilePath ownerRef = customProp->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
                if (!ownerRef.IsEmpty())
                {
                    if (selectionSize == 1)
                    {
                        Connect(menu.addAction(QStringLiteral("Edit Model")), this, &EntityContextMenu::EditModel);
                    }

                    Connect(menu.addAction(QStringLiteral("Reload Model...")), this, &EntityContextMenu::ReloadModel);
                }

                isConstReference = customProp->GetBool(ResourceEditor::EDITOR_CONST_REFERENCE, false);
            }

            if (isConstReference != true)
            {
                Connect(menu.addAction(QStringLiteral("Reload Model As...")), this, &EntityContextMenu::ReloadModelAs);
            }

            // particle effect
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
            if (nullptr != effect)
            {
                menu.addSeparator();
                QMenu* particleEffectMenu = menu.addMenu("Particle Effect");

                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/emitter_particle.png"), QStringLiteral("Add Emitter")), this, &EntityContextMenu::AddEmitter);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Effect Emitters")), this, &EntityContextMenu::SaveEffectEmitters);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Effect Emitters As...")), this, &EntityContextMenu::SaveEffectEmittersAs);
                particleEffectMenu->addSeparator();
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/play.png"), QStringLiteral("Start")), this, &EntityContextMenu::StartEffect);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/stop.png"), QStringLiteral("Stop")), this, &EntityContextMenu::StopEffect);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/restart.png"), QStringLiteral("Restart")), this, &EntityContextMenu::RestartEffect);
            }

            if (selectionSize)
            {
                DAVA::TexturesMap textures;

                for (auto selectEntity : selection.ObjectsOfType<DAVA::Entity>())
                {
                    SceneHelper::TextureCollector collector;
                    SceneHelper::EnumerateEntityTextures(scene, selectEntity, collector);
                    DAVA::TexturesMap& collectorTextures = collector.GetTextures();

                    textures.insert(collectorTextures.begin(), collectorTextures.end());
                }

                if (textures.empty() == false)
                {
                    Connect(menu.addAction(QStringLiteral("Reload Textures...")), [textures]
                            {
                                DAVA::Vector<DAVA::Texture*> reloadTextures;

                                for (auto& tex : textures)
                                {
                                    reloadTextures.push_back(tex.second);
                                }

                                REGlobal::GetInvoker()->Invoke(REGlobal::ReloadTextures.ID, reloadTextures);

                            });
                }
            }

            if (selectionSize == 1)
            {
                menu.addSeparator();
                Connect(menu.addAction(SharedIcon(":/QtIconsTextureDialog/filter.png"), QStringLiteral("Set name as filter")), this, &EntityContextMenu::SetEntityNameAsFilter);
            }

            menu.addSeparator();
            {
                Connect(menu.addAction(SharedIcon(":/QtIcons/openscene.png"), QStringLiteral("Load slots preset")), this, &EntityContextMenu::LoadSlotsPreset);
            }
            {
                std::function<bool(DAVA::Entity*)> checkSlotComponent = [&checkSlotComponent](DAVA::Entity* entity) {
                    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0)
                        return true;

                    for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
                    {
                        if (checkSlotComponent(entity->GetChild(i)) == true)
                            return true;
                    }

                    return false;
                };
                bool hasSlotComponent = checkSlotComponent(entity);
                QAction* action = menu.addAction(SharedIcon(":/QtIcons/save_as.png"), QStringLiteral("Save slots preset"));
                action->setEnabled(hasSlotComponent);
                Connect(action, this, &EntityContextMenu::SaveSlotsPreset);
            }
        }
    }

private:
    void FillCameraActions(QMenu& menu)
    {
        using namespace DAVA::TArc;
        Connect(menu.addAction(SharedIcon(":/QtIcons/eye.png"), QStringLiteral("Look from")), this, &EntityContextMenu::SetCurrentCamera);
        Connect(menu.addAction(SharedIcon(":/QtIcons/camera.png"), QStringLiteral("Set custom draw camera")), this, &EntityContextMenu::SetCustomDrawCamera);
        Connect(menu.addAction(SharedIcon(":/QtIcons/grab-image.png"), QStringLiteral("Grab image")), this, &EntityContextMenu::GrabImage);
    }

    void SaveEntityAs()
    {
        const SelectableGroup& selection = Selection::GetSelection();
        if (selection.IsEmpty())
            return;

        SceneEditor2* scene = GetScene();
        DAVA::FilePath scenePath = scene->GetScenePath().GetDirectory();
        if (!DAVA::FileSystem::Instance()->Exists(scenePath) || !scene->IsLoaded())
        {
            scenePath = SceneTreeDetails::GetDataSourcePath();
        }

        QString baseDir(scenePath.GetDirectory().GetAbsolutePathname().c_str());
        QString filePath = FileDialog::getSaveFileName(nullptr, QStringLiteral("Save scene file"), baseDir, QStringLiteral("DAVA SceneV2 (*.sc2)"));
        if (!filePath.isEmpty())
        {
            SaveEntityAsAction saver(&selection, filePath.toStdString());
            saver.Run();
        }
    }

    void EditModel()
    {
        const SelectableGroup& selection = Selection::GetSelection();
        for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
        {
            DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
            if (archive)
            {
                DAVA::FilePath entityRefPath = archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
                if (DAVA::FileSystem::Instance()->Exists(entityRefPath))
                {
                    std::shared_ptr<GlobalOperations>& globalOperations = GetParentWidget()->globalOperations;
                    DVASSERT(globalOperations != nullptr);
                    globalOperations->CallAction(GlobalOperations::OpenScene, DAVA::Any(DAVA::String(entityRefPath.GetAbsolutePathname().c_str())));
                }
                else
                {
                    DAVA::Logger::Error((ResourceEditor::SCENE_TREE_WRONG_REF_TO_OWNER + entityRefPath.GetAbsolutePathname()).c_str());
                }
            }
        }
    }

    void ReloadModel()
    {
        SceneEditor2* sceneEditor = GetScene();
        QDialog dlg(GetParentWidget());

        QVBoxLayout* dlgLayout = new QVBoxLayout();
        dlgLayout->setMargin(10);

        dlg.setWindowTitle("Reload Model options");
        dlg.setLayout(dlgLayout);

        QCheckBox* lightmapsChBox = new QCheckBox(QStringLiteral("Leave lightmap settings"), &dlg);
        dlgLayout->addWidget(lightmapsChBox);
        lightmapsChBox->setCheckState(Qt::Checked);

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
        dlgLayout->addWidget(buttons);

        QObject::connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
        QObject::connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));

        if (QDialog::Accepted == dlg.exec())
        {
            DAVA::String wrongPathes;
            const SelectableGroup& selection = Selection::GetSelection();
            for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
            {
                DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
                if (archive)
                {
                    DAVA::FilePath pathToReload(archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER));
                    if (!DAVA::FileSystem::Instance()->Exists(pathToReload))
                    {
                        wrongPathes += DAVA::Format("\r\n%s : %s", entity->GetName().c_str(),
                                                    pathToReload.GetAbsolutePathname().c_str());
                    }
                }
            }
            if (!wrongPathes.empty())
            {
                DAVA::Logger::Error((ResourceEditor::SCENE_TREE_WRONG_REF_TO_OWNER + wrongPathes).c_str());
            }
            SelectableGroup newSelection = sceneEditor->structureSystem->ReloadEntities(selection, lightmapsChBox->isChecked());
            Selection::SetSelection(newSelection);
        }
    }

    void ReloadModelAs()
    {
        SceneEditor2* sceneEditor = GetScene();

        const SelectableGroup& selection = Selection::GetSelection();
        DAVA::Entity* entity = selection.GetContent().front().AsEntity();
        DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
        if (archive != nullptr)
        {
            DAVA::String ownerPath = archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
            if (ownerPath.empty())
            {
                DAVA::FilePath p = sceneEditor->GetScenePath().GetDirectory();
                if (DAVA::FileSystem::Instance()->Exists(p) && sceneEditor->IsLoaded())
                {
                    ownerPath = p.GetAbsolutePathname();
                }
                else
                {
                    ownerPath = SceneTreeDetails::GetDataSourcePath().GetAbsolutePathname();
                }
            }

            QString filePath = FileDialog::getOpenFileName(GetParentWidget(), QStringLiteral("Open scene file"), ownerPath.c_str(), QStringLiteral("DAVA SceneV2 (*.sc2)"));
            if (!filePath.isEmpty())
            {
                SelectableGroup newSelection = sceneEditor->structureSystem->ReloadEntitiesAs(selection, filePath.toStdString());
                Selection::SetSelection(newSelection);
            }
        }
    }

    void AddEmitter()
    {
        DAVA::Entity* entity = entityItem->GetEntity();
        DVASSERT(DAVA::GetEffectComponent(entity) != nullptr);

        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleEmitter(entity)));
        MarkStructureChanged();
    }

    void SaveEffectEmitters()
    {
        PerformSaveEffectEmitters(false);
    }

    void SaveEffectEmittersAs()
    {
        PerformSaveEffectEmitters(true);
    }

    void GrabImage()
    {
        SceneEditor2* scene = GetScene();
        DAVA::FilePath scenePath = scene->GetScenePath();
        QString filePath = FileDialog::getSaveFileName(GetParentWidget()->globalOperations->GetGlobalParentWidget(),
                                                       "Save Scene Image",
                                                       scenePath.GetDirectory().GetAbsolutePathname().c_str(),
                                                       PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);

        if (filePath.isEmpty())
            return;

        SceneImageGrabber::Params params;
        params.scene = scene;
        params.cameraToGrab = GetCamera(entityItem->GetEntity());
        DVASSERT(params.cameraToGrab.Get() != nullptr);
        GlobalSceneSettings* settings = REGlobal::GetGlobalContext()->GetData<GlobalSceneSettings>();
        params.imageSize = DAVA::Size2i(settings->grabSizeWidth, settings->grabSizeHeight);
        params.outputFile = filePath.toStdString();

        SceneImageGrabber::GrabImage(params);
    }

    void PerformSaveEffectEmitters(bool forceAskFileName)
    {
        SceneEditor2* sceneEditor = GetScene();
        DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entityItem->GetEntity());
        DVASSERT(effect != nullptr);

        QString effectName(entityItem->GetEntity()->GetName().c_str());
        for (DAVA::int32 i = 0, sz = effect->GetEmittersCount(); i != sz; ++i)
        {
            DAVA::ParticleEmitterInstance* instance = effect->GetEmitterInstance(i);
            QString defName = effectName + "_" + QString::number(i + 1) + "_" + QString(instance->GetEmitter()->name.c_str()) + ".yaml";
            SceneTreeDetails::SaveEmitter(sceneEditor, effect, instance->GetEmitter(), forceAskFileName, defName, [&](const DAVA::FilePath& path) {
                return std::unique_ptr<DAVA::Command>(new CommandSaveParticleEmitterToYaml(effect, instance, path));
            });
        }
    }

    template <typename CMD, typename... Arg>
    void ExecuteCommandForEffect(Arg&&... args)
    {
        SceneEditor2* sceneEditor = GetScene();
        const SelectableGroup& selection = Selection::GetSelection();
        for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
        {
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
            if (nullptr != effect)
            {
                sceneEditor->Exec(std::unique_ptr<CMD>(new CMD(entity, std::forward<Arg>(args)...)));
            }
        }
    }

    void StartEffect()
    {
        ExecuteCommandForEffect<CommandStartStopParticleEffect>(true);
    }

    void StopEffect()
    {
        ExecuteCommandForEffect<CommandStartStopParticleEffect>(false);
    }

    void RestartEffect()
    {
        ExecuteCommandForEffect<CommandRestartParticleEffect>();
    }

    void SetEntityNameAsFilter()
    {
        const SelectableGroup& selection = Selection::GetSelection();
        DVASSERT(selection.GetSize() == 1);
        DVASSERT(selection.GetFirst().CanBeCastedTo<DAVA::Entity>());

        std::shared_ptr<GlobalOperations>& globalOperations = GetParentWidget()->globalOperations;
        DVASSERT(globalOperations != nullptr);
        globalOperations->CallAction(GlobalOperations::SetNameAsFilter, DAVA::Any(DAVA::String(selection.GetFirst().AsEntity()->GetName().c_str())));
    }

    void SetCurrentCamera()
    {
        DAVA::Camera* camera = GetCamera(entityItem->GetEntity());
        DVASSERT(camera != nullptr);
        GetScene()->SetCurrentCamera(camera);
    }

    void SetCustomDrawCamera()
    {
        DAVA::Camera* camera = GetCamera(entityItem->GetEntity());
        DVASSERT(camera != nullptr);
        GetScene()->SetCustomDrawCamera(camera);
    }

    void SaveSlotsPreset()
    {
        SceneEditor2* scene = GetScene();
        SlotSystemSettings* settings = REGlobal::GetGlobalContext()->GetData<SlotSystemSettings>();

        if (scene->IsLoaded() == false)
        {
            DAVA::Logger::Error("[SaveSlotsPreset] Sorry, you should save scene first");
            return;
        }

        QString selectedPath = FileDialog::getSaveFileName(nullptr, QStringLiteral("Save slot preset"), settings->lastPresetSaveLoadPath, QStringLiteral("Slots preset file (*.spreset)"));
        if (selectedPath.isEmpty())
        {
            return;
        }

        settings->lastPresetSaveLoadPath = selectedPath;
        EditorSlotSystem* system = scene->LookupEditorSystem<EditorSlotSystem>();
        DAVA::RefPtr<DAVA::KeyedArchive> archive = system->SaveSlotsPreset(entityItem->GetEntity());
        if (archive->Save(selectedPath.toStdString()) == false)
        {
            DAVA::Logger::Error("Slots Preset was not saved");
        }
    }

    void LoadSlotsPreset()
    {
        SlotSystemSettings* settings = REGlobal::GetGlobalContext()->GetData<SlotSystemSettings>();
        QString selectedPath = FileDialog::getOpenFileName(nullptr, QStringLiteral("Save slot preset"), settings->lastPresetSaveLoadPath, QStringLiteral("Slots preset file (*.spreset)"));
        if (selectedPath.isEmpty())
        {
            return;
        }

        settings->lastPresetSaveLoadPath = selectedPath;
        DAVA::RefPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive());
        if (archive->Load(selectedPath.toStdString()) == false)
        {
            DAVA::Logger::Error("Can't read selected slots preset");
            return;
        }

        EditorSlotSystem* system = GetScene()->LookupEditorSystem<EditorSlotSystem>();
        system->LoadSlotsPreset(entityItem->GetEntity(), archive);
    }

private:
    SceneTreeItemEntity* entityItem;
};

class SceneTree::ParticleLayerContextMenu : public BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleLayerContextMenu(SceneTreeItemParticleLayer* layerItem_, SceneTree* treeWidget)
        : TBase(treeWidget)
        , layerItem(layerItem_)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && layerItem != nullptr;
    }

    void FillActions(QMenu& menu) override
    {
        using namespace DAVA::TArc;
        Connect(menu.addAction(SharedIcon(":/QtIcons/clone.png"), QStringLiteral("Clone Layer")), this, &ParticleLayerContextMenu::CloneLayer);
        QString removeLayerText = GetSelectedItemsCount() < 2 ? QStringLiteral("Remove Layer") : QStringLiteral("Remove Layers");
        Connect(menu.addAction(SharedIcon(":/QtIcons/remove_layer.png"), removeLayerText), this, &ParticleLayerContextMenu::RemoveLayer);
        menu.addSeparator();
        Connect(menu.addAction(SharedIcon(":/QtIcons/force.png"), QStringLiteral("Add Force")), this, &ParticleLayerContextMenu::AddForce);
        Connect(menu.addAction(SharedIcon(":/QtIcons/turtle.png"), QStringLiteral("Add Drag")), this, &ParticleLayerContextMenu::AddDrag);
        Connect(menu.addAction(SharedIcon(":/QtIcons/vortex_ico.png"), QStringLiteral("Add Vortex")), this, &ParticleLayerContextMenu::AddVortex);
        Connect(menu.addAction(SharedIcon(":/QtIcons/gravity.png"), QStringLiteral("Add Gravity")), this, &ParticleLayerContextMenu::AddGravity);
        Connect(menu.addAction(SharedIcon(":/QtIcons/wind_p.png"), QStringLiteral("Add Wind")), this, &ParticleLayerContextMenu::AddWind);
        Connect(menu.addAction(SharedIcon(":/QtIcons/pointGravity.png"), QStringLiteral("Add Point Gravity")), this, &ParticleLayerContextMenu::AddPointGravity);
        Connect(menu.addAction(SharedIcon(":/QtIcons/plane_coll.png"), QStringLiteral("Add Plane Collision")), this, &ParticleLayerContextMenu::AddPlaneCollision);
    }

private:
    void CloneLayer()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandCloneParticleEmitterLayer(layerItem->emitterInstance, layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void RemoveLayer()
    {
        RemoveCommandsHelper("Remove layers", SceneTreeItem::EIT_Layer, [](SceneTreeItem* item) -> RemoveInfo
                             {
                                 SceneTreeItemParticleLayer* layerItem = static_cast<SceneTreeItemParticleLayer*>(item);
                                 DAVA::ParticleLayer* layer = layerItem->GetLayer();
                                 return RemoveInfo(std::unique_ptr<DAVA::Command>(new CommandRemoveParticleEmitterLayer(layerItem->emitterInstance, layer)), layer);
                             });
    }

    void AddForce()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleEmitterSimplifiedForce(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddDrag()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleDrag(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddVortex()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleVortex(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddGravity()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleGravity(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddWind()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleWind(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddPointGravity()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticlePointGravity(layerItem->GetLayer())));
        MarkStructureChanged();
    }

    void AddPlaneCollision()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticlePlaneCollision(layerItem->GetLayer())));
        MarkStructureChanged();
    }

private:
    SceneTreeItemParticleLayer* layerItem;
};

class SceneTree::ParticleSimplifiedForceContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleSimplifiedForceContextMenu(SceneTree* treeWidget)
        : TBase(treeWidget)
    {
    }

protected:
    void FillActions(QMenu& menu) override
    {
        using namespace DAVA::TArc;
        QString removeForce = GetSelectedItemsCount() < 2 ? QStringLiteral("Remove Force") : QStringLiteral("Remove Forces");
        Connect(menu.addAction(SharedIcon(":/QtIcons/remove_force.png"), removeForce), this, &ParticleSimplifiedForceContextMenu::RemoveForce);
    }

private:
    void RemoveForce()
    {
        RemoveCommandsHelper("Remove simplified forces", SceneTreeItem::EIT_ForceSimplified, [](SceneTreeItem* item)
                             {
                                 SceneTreeItemParticleForceSimplified* forceItem = static_cast<SceneTreeItemParticleForceSimplified*>(item);
                                 DAVA::ParticleForceSimplified* force = forceItem->GetForce();
                                 return RemoveInfo(std::unique_ptr<DAVA::Command>(new CommandRemoveParticleEmitterSimplifiedForce(forceItem->layer, force)), force);
                             });
    }
};

class SceneTree::ParticleForceContextMenu : public SceneTree::BaseContextMenu
{
    //////////////////////////////////////////////////////////////////////////
    using TBase = BaseContextMenu;

public:
    ParticleForceContextMenu(SceneTreeItemParticleForce* force_, SceneTree* treeWidget)
        : TBase(treeWidget)
        , forceItem(force_)
    {
    }

protected:
    void FillActions(QMenu& menu) override
    {
        Connect(menu.addAction(DAVA::TArc::SharedIcon(":/QtIcons/clone.png"), QStringLiteral("Clone Force")), this, &ParticleForceContextMenu::CloneForce);
        QString removeForceHint;
        const QIcon* icon = nullptr;
        if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::DRAG_FORCE)
        {
            removeForceHint = GetSelectedItemsCount() < 2 ? QStringLiteral("Remove Drag Force") : QStringLiteral("Remove Drag Forces");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/remove_turtle.png");
        }
        else if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::VORTEX)
        {
            removeForceHint = GetSelectedItemsCount() < 2 ? QStringLiteral("Remove Vortex") : QStringLiteral("Remove Vortices");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/vortex_ico_remove.png");
        }
        else if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::GRAVITY)
        {
            removeForceHint = QStringLiteral("Remove Gravity");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/gravity_remove.png");
        }
        else if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::WIND)
        {
            removeForceHint = QStringLiteral("Remove Wind");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/wind_p_remove.png");
        }
        else if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::POINT_GRAVITY)
        {
            removeForceHint = QStringLiteral("Remove Point Gravity");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/pointGravity_remove.png");
        }
        else if (forceItem->GetForce()->type == DAVA::ParticleForce::eType::PLANE_COLLISION)
        {
            removeForceHint = QStringLiteral("Remove Plane Collision");
            icon = &DAVA::TArc::SharedIcon(":/QtIcons/plane_coll_remove.png");
        }

        Connect(menu.addAction(*icon, removeForceHint), this, &ParticleForceContextMenu::RemoveForce);
    }

private:
    void CloneForce()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandCloneParticleForce(forceItem->layer, forceItem->GetForce())));
        MarkStructureChanged();
    }

    void RemoveForce()
    {
        DAVA::ParticleForce* force = forceItem->GetForce();
        DAVA::String commandName;
        if (force->type == DAVA::ParticleForce::eType::DRAG_FORCE)
            commandName = "Remove Drag force";
        else if (force->type == DAVA::ParticleForce::eType::VORTEX)
            commandName = "Remove Vortex";
        else if (force->type == DAVA::ParticleForce::eType::GRAVITY)
            commandName = "Remove Gravity";
        else if (force->type == DAVA::ParticleForce::eType::WIND)
            commandName = "Remove Wind";
        else if (force->type == DAVA::ParticleForce::eType::POINT_GRAVITY)
            commandName = "Remove Point Gravity";
        else if (force->type == DAVA::ParticleForce::eType::PLANE_COLLISION)
            commandName = "Remove Plane Collision";

        RemoveCommandsHelper(commandName, SceneTreeItem::EIT_ParticleForce, [](SceneTreeItem* item)
                             {
                                 SceneTreeItemParticleForce* forceItem = static_cast<SceneTreeItemParticleForce*>(item);
                                 DAVA::ParticleForce* force = forceItem->GetForce();
                                 return RemoveInfo(std::unique_ptr<DAVA::Command>(new CommandRemoveParticleForce(forceItem->layer, force)), force);
                             });
    }

    SceneTreeItemParticleForce* forceItem;
};

class SceneTree::ParticleEmitterContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleEmitterContextMenu(SceneTreeItemParticleEmitter* emitter, SceneTree* treeWidget)
        : TBase(treeWidget)
        , emitterItem(emitter)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && emitterItem != nullptr;
    }

    virtual bool IsRemovable() const
    {
        return true;
    }

    void FillActions(QMenu& menu) override
    {
        using namespace DAVA::TArc;
        if (IsRemovable())
        {
            QString removeEmitterText = GetSelectedItemsCount() < 2 ? QStringLiteral("Remove emitter") : QStringLiteral("Remove emitters");
            Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), removeEmitterText), this, &ParticleEmitterContextMenu::RemoveEmitter);
            menu.addSeparator();
        }
        Connect(menu.addAction(SharedIcon(":/QtIcons/layer_particle.png"), QStringLiteral("Add Layer")), this, &ParticleEmitterContextMenu::AddLayer);
        menu.addSeparator();
        Connect(menu.addAction(SharedIcon(":/QtIcons/openscene.png"), QStringLiteral("Load Emitter from Yaml")), this, &ParticleEmitterContextMenu::LoadEmitterFromYaml);
        Connect(menu.addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Emitter to Yaml")), this, &ParticleEmitterContextMenu::SaveEmitterToYaml);
        Connect(menu.addAction(SharedIcon(":/QtIcons/save_as.png"), QStringLiteral("Save Emitter to Yaml As...")), this, &ParticleEmitterContextMenu::SaveEmitterToYamlAs);
    }

    void RemoveEmitter()
    {
        RemoveCommandsHelper("Remove Emitters", SceneTreeItem::EIT_Emitter, [](SceneTreeItem* item)
                             {
                                 SceneTreeItemParticleEmitter* emitterItem = static_cast<SceneTreeItemParticleEmitter*>(item);
                                 DAVA::ParticleEmitterInstance* emitterInstance = emitterItem->GetEmitterInstance();
                                 return RemoveInfo(std::unique_ptr<DAVA::Command>(new CommandRemoveParticleEmitter(emitterItem->effect, emitterInstance)), emitterInstance);
                             });
    }

    void AddLayer()
    {
        GetScene()->Exec(std::unique_ptr<DAVA::Command>(new CommandAddParticleEmitterLayer(emitterItem->GetEmitterInstance())));
        MarkStructureChanged();
    }

    void LoadEmitterFromYaml()
    {
        CommonInternalSettings* settings = REGlobal::GetGlobalContext()->GetData<CommonInternalSettings>();
        DAVA::FilePath defaultPath = settings->emitterLoadDir;
        QString particlesPath = defaultPath.IsEmpty() ? SceneTreeDetails::GetParticlesConfigPath() : QString::fromStdString(defaultPath.GetAbsolutePathname());

        QString selectedPath = FileDialog::getOpenFileName(nullptr, QStringLiteral("Open Particle Emitter Yaml file"), particlesPath, QStringLiteral("YAML File (*.yaml)"));

        if (selectedPath.isEmpty() == false)
        {
            DAVA::FilePath yamlPath = selectedPath.toStdString();
            settings->emitterLoadDir = yamlPath.GetDirectory();

            GetScene()->Exec(CreateLoadCommand(yamlPath));
            MarkStructureChanged();
        }
    }

    void SaveEmitterToYaml()
    {
        SaveEmitter(false);
    }

    void SaveEmitterToYamlAs()
    {
        SaveEmitter(true);
    }

    void SaveEmitter(bool forceAskFileName)
    {
        SceneTreeDetails::SaveEmitter(GetScene(), emitterItem->effect, emitterItem->GetEmitterInstance()->GetEmitter(),
                                      forceAskFileName, QString(),
                                      DAVA::MakeFunction(this, &ParticleEmitterContextMenu::CreateSaveCommand));
    }

    virtual std::unique_ptr<DAVA::Command> CreateLoadCommand(const DAVA::FilePath& path)
    {
        return std::unique_ptr<DAVA::Command>(new CommandLoadParticleEmitterFromYaml(emitterItem->effect, emitterItem->GetEmitterInstance(), path));
    }

    virtual std::unique_ptr<DAVA::Command> CreateSaveCommand(const DAVA::FilePath& path)
    {
        return std::unique_ptr<DAVA::Command>(new CommandSaveParticleEmitterToYaml(emitterItem->effect, emitterItem->GetEmitterInstance(), path));
    }

private:
    SceneTreeItemParticleEmitter* emitterItem;
};

class SceneTree::ParticleInnerEmitterContextMenu : public SceneTree::ParticleEmitterContextMenu
{
    using TBase = ParticleEmitterContextMenu;

public:
    ParticleInnerEmitterContextMenu(SceneTreeItemParticleInnerEmitter* item, SceneTree* treeWidget)
        : TBase(item, treeWidget)
        , emitterItem(item)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && emitterItem != nullptr;
    }

    bool IsRemovable() const override
    {
        return false;
    }

    std::unique_ptr<DAVA::Command> CreateLoadCommand(const DAVA::FilePath& path) override
    {
        emitterItem->parent->innerEmitterPath = path;
        return std::unique_ptr<DAVA::Command>(new CommandLoadInnerParticleEmitterFromYaml(emitterItem->GetEmitterInstance(), path));
    }

    std::unique_ptr<DAVA::Command> CreateSaveCommand(const DAVA::FilePath& path) override
    {
        emitterItem->parent->innerEmitterPath = path;
        return std::unique_ptr<DAVA::Command>(new CommandSaveInnerParticleEmitterToYaml(emitterItem->GetEmitterInstance(), path));
    }

private:
    SceneTreeItemParticleInnerEmitter* emitterItem = nullptr;
};

SceneTree::SceneTree(QWidget* parent /*= 0*/)
    : QTreeView(parent)
    , treeModel(new SceneTreeModel())
    , filteringProxyModel(new SceneTreeFilteringModel(treeModel))
{
    DAVA::Function<void()> fn(this, &SceneTree::UpdateTree);
    treeUpdater = new LazyUpdater(fn, this);

    setModel(filteringProxyModel);

    treeDelegate = new SceneTreeDelegate();
    setItemDelegate(treeDelegate);

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // scene signals
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &SceneTree::SceneActivated);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &SceneTree::SceneDeactivated);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::StructureChanged, this, &SceneTree::SceneStructureChanged);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &SceneTree::CommandExecuted);

    selectionFieldBinder.reset(new DAVA::TArc::FieldBinder(REGlobal::GetAccessor()));
    {
        DAVA::TArc::FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<SelectionData>();
        fieldDescr.fieldName = DAVA::FastName(SelectionData::selectionPropertyName);
        selectionFieldBinder->BindField(fieldDescr, [this](const DAVA::Any&) { SyncSelectionToTree(); });
    }

    // particles signals
    QObject::connect(SceneSignals::Instance(), &SceneSignals::ParticleLayerValueChanged, this, &SceneTree::ParticleLayerValueChanged);

    // this widget signals
    QObject::connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &SceneTree::TreeSelectionChanged);
    QObject::connect(this, &QTreeView::doubleClicked, this, &SceneTree::TreeItemDoubleClicked);
    QObject::connect(this, &QTreeView::collapsed, this, &SceneTree::TreeItemCollapsed);
    QObject::connect(this, &QTreeView::expanded, this, &SceneTree::TreeItemExpanded);
    QObject::connect(new QShortcut(QKeySequence(Qt::Key_X), this), &QShortcut::activated, this, &SceneTree::CollapseSwitch);

    QObject::connect(this, &QTreeView::customContextMenuRequested, this, &SceneTree::ShowContextMenu);

    QAction* deleteSelection = new QAction(tr("Delete Selection"), this);
    deleteSelection->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete << Qt::CTRL + Qt::Key_Backspace);
    deleteSelection->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteSelection, &QAction::triggered, this, &SceneTree::RemoveSelection);
    addAction(deleteSelection);
}

SceneTree::~SceneTree()
{
    delete filteringProxyModel;
    delete treeModel;
}

void SceneTree::Init(const std::shared_ptr<GlobalOperations>& globalOperations_)
{
    globalOperations = globalOperations_;
}

void SceneTree::SetFilter(const QString& filter)
{
    treeModel->SetFilter(filter);
    filteringProxyModel->invalidate();
    SyncSelectionToTree();

    if (!filter.isEmpty())
    {
        ExpandFilteredItems();
    }
}

void SceneTree::RemoveSelection()
{
    ::RemoveSelection(treeModel->GetScene());
}

void SceneTree::GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col)
{
    row = -1;
    col = -1;
    index = indexAt(pos);
    if (!visualRect(index).contains(pos))
    {
        index = QModelIndex();
        return;
    }

    switch (dropIndicatorPosition())
    {
    case QAbstractItemView::AboveItem:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::BelowItem:
        row = index.row() + 1;
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::OnItem:
    case QAbstractItemView::OnViewport:
        break;
    }
}

void SceneTree::dropEvent(QDropEvent* event)
{
    QTreeView::dropEvent(event);

    if (treeModel->DropAccepted())
    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        expand(parent);
    }

    // after processing don't allow this event to go higher
    // so no body will decide to remove/insert drag&dropped items into treeview
    // except our model. Model will do this when scene entity remove/move signals catched
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void SceneTree::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeView::dragMoveEvent(event);

    GlobalSceneSettings* settings = REGlobal::GetGlobalContext()->GetData<GlobalSceneSettings>();
    if (settings->dragAndDropWithShift == true && ((event->keyboardModifiers() & Qt::SHIFT) != Qt::SHIFT))
    {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
        return;
    }

    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        if (treeModel->DropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, filteringProxyModel->mapToSource(parent)))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }
}

void SceneTree::dragEnterEvent(QDragEnterEvent* event)
{
    QTreeView::dragEnterEvent(event);

    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        if (treeModel->DropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, filteringProxyModel->mapToSource(parent)))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }
}

void SceneTree::SceneActivated(SceneEditor2* scene)
{
    treeModel->SetScene(scene);
    selectionModel()->clear();
    SyncSelectionToTree();
    filteringProxyModel->invalidate();

    PropagateSolidFlag();
}

void SceneTree::SceneDeactivated(SceneEditor2* scene)
{
    if (treeModel->GetScene() == scene)
    {
        selectionModel()->clear();
        treeModel->SetScene(nullptr);
    }
}

void SceneTree::SceneStructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
{
    if (scene == treeModel->GetScene())
    {
        UpdateModel();
    }
}

void SceneTree::CommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification)
{
    static const DAVA::Vector<DAVA::uint32> idsForUpdate =
    { {
    CMDID_COMPONENT_ADD,
    CMDID_COMPONENT_REMOVE,
    CMDID_INSP_MEMBER_MODIFY,
    CMDID_INSP_DYNAMIC_MODIFY,
    CMDID_ENTITY_LOCK,
    CMDID_PARTICLE_EMITTER_ADD,
    CMDID_PARTICLE_EMITTER_MOVE,
    CMDID_PARTICLE_LAYER_REMOVE,
    CMDID_PARTICLE_LAYER_MOVE,
    CMDID_PARTICLE_FORCE_REMOVE,
    CMDID_PARTICLE_SIMPLIFIED_FORCE_MOVE,
    CMDID_PARTICLE_FORCE_MOVE,
    CMDID_META_OBJ_MODIFY,
    CMDID_PARTICLE_EMITTER_LAYER_ADD,
    CMDID_PARTICLE_EMITTER_LAYER_REMOVE,
    CMDID_PARTICLE_EMITTER_LAYER_CLONE,
    CMDID_PARTICLE_EMITTER_SIMPLIFIED_FORCE_ADD,
    CMDID_PARTICLE_EMITTER_SIMPLIFIED_FORCE_REMOVE,
    CMDID_PARTICLE_EMITTER_DRAG_ADD,
    CMDID_PARTICLE_EMITTER_VORTEX_ADD,
    CMDID_PARTICLE_EMITTER_GRAVITY_ADD,
    CMDID_PARTICLE_EMITTER_WIND_ADD,
    CMDID_PARTICLE_EMITTER_POINT_GRAVITY_ADD,
    CMDID_PARTICLE_EMITTER_PLANE_COLLISION_ADD,
    CMDID_PARTICLE_EMITTER_FORCE_REMOVE,
    CMDID_PARTICLE_EFFECT_EMITTER_REMOVE,
    CMDID_REFLECTED_FIELD_MODIFY,
    } };
    static const DAVA::Vector<DAVA::uint32> idsForTreeUpdate =
    { {
    CMDID_PARTICLE_FORCE_UPDATE,
    CMDID_PARTICLE_SIMPLIFIED_FORCE_UPDATE
    } };

    if (commandNotification.MatchCommandIDs(idsForUpdate))
    {
        UpdateModel();
        treeUpdater->Update();
    }

    if (commandNotification.MatchCommandIDs(idsForTreeUpdate))
    {
        treeUpdater->Update();
    }
}

void SceneTree::TreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (isInSelectionSync)
        return;

    SyncSelectionFromTree();
}

void SceneTree::ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer)
{
    QModelIndexList indexList = selectionModel()->selection().indexes();
    if (indexList.empty())
    {
        return;
    }

    QModelIndex realIndex = filteringProxyModel->mapToSource(indexList[0]);
    SceneTreeItem* item = treeModel->GetItem(realIndex);
    if (item->ItemType() != SceneTreeItem::EIT_Layer)
    {
        return;
    }

    DAVA::ParticleLayer* selectedLayer = SceneTreeItemParticleLayer::GetLayer(item);
    if (selectedLayer != layer)
    {
        return;
    }

    // Update the "isEnabled" flag, if it is changed.
    bool sceneTreeItemChecked = item->checkState() == Qt::Checked;
    if (layer->isDisabled == sceneTreeItemChecked)
    {
        const QSignalBlocker guard(this);
        item->setCheckState(sceneTreeItemChecked ? Qt::Unchecked : Qt::Checked);
    }

    //check if we need to resync tree for superemmiter
    SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)item;
    bool needEmmiter = selectedLayer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES;
    if (itemLayer->hasInnerEmmiter != needEmmiter)
    {
        itemLayer->hasInnerEmmiter = needEmmiter;
        UpdateModel();
    }
}

void SceneTree::TreeItemDoubleClicked(const QModelIndex& index)
{
    LookAtSelection(treeModel->GetScene());
}

void SceneTree::ShowContextMenu(const QPoint& pos)
{
    const SelectableGroup& selection = Selection::GetSelection();
    if (selection.IsEmpty())
    {
        return;
    }

    QModelIndex index = filteringProxyModel->mapToSource(indexAt(pos));
    if (!index.isValid())
    {
        return;
    }

    SceneTreeItem* item = treeModel->GetItem(index);
    DVASSERT(item != nullptr);

    QPoint globalPos = mapToGlobal(pos);

    auto showMenuFn = [&](BaseContextMenu&& menu)
    {
        menu.Show(globalPos);
        if (menu.IsStructureChanged())
        {
            UpdateModel();
        }
    };

    switch (item->ItemType())
    {
    case SceneTreeItem::EIT_Entity:
        showMenuFn(EntityContextMenu(static_cast<SceneTreeItemEntity*>(item), this));
        break;
    case SceneTreeItem::EIT_Emitter:
        showMenuFn(ParticleEmitterContextMenu(static_cast<SceneTreeItemParticleEmitter*>(item), this));
        break;
    case SceneTreeItem::EIT_Layer:
        showMenuFn(ParticleLayerContextMenu(static_cast<SceneTreeItemParticleLayer*>(item), this));
        break;
    case SceneTreeItem::EIT_ForceSimplified:
        showMenuFn(ParticleSimplifiedForceContextMenu(this));
        break;
    case SceneTreeItem::EIT_InnerEmitter:
        showMenuFn(ParticleInnerEmitterContextMenu(static_cast<SceneTreeItemParticleInnerEmitter*>(item), this));
        break;
    case SceneTreeItem::EIT_ParticleForce:
        showMenuFn(ParticleForceContextMenu(static_cast<SceneTreeItemParticleForce*>(item), this));
        break;
    default:
        break;
    };
}

void SceneTree::CollapseSwitch()
{
    QModelIndexList indexList = selectionModel()->selection().indexes();
    for (int i = 0; i < indexList.size(); ++i)
    {
        QModelIndex index = indexList.at(i);

        if (isExpanded(index))
        {
            collapse(index);
        }
        else
        {
            expand(index);
        }
    }
}

void SceneTree::ReloadSelectedEntitiesTextures()
{
    using namespace DAVA::TArc;
    const SelectableGroup& selection = Selection::GetSelection();
    DAVA::Vector<DAVA::Texture*> reloadTextures;

    for (auto selectEntity : selection.ObjectsOfType<DAVA::Entity>())
    {
        SceneHelper::TextureCollector collector;
        SceneHelper::EnumerateEntityTextures(selectEntity->GetScene(), selectEntity, collector);
        DAVA::TexturesMap& textures = collector.GetTextures();

        for (auto& tex : textures)
        {
            auto found = std::find(reloadTextures.begin(), reloadTextures.end(), tex.second);

            if (found == reloadTextures.end())
            {
                reloadTextures.push_back(tex.second);
            }
        }
    }

    REGlobal::GetInvoker()->Invoke(REGlobal::ReloadTextures.ID, reloadTextures);
}

void SceneTree::CollapseAll()
{
    QTreeView::collapseAll();
    bool needSync = false;
    {
        DAVA::TArc::ScopedValueGuard<bool> guard(isInSelectionSync, true);

        QModelIndexList indexList = selectionModel()->selection().indexes();
        for (int i = 0; i < indexList.size(); ++i)
        {
            QModelIndex childIndex = indexList[i];
            if (childIndex.parent().isValid())
            {
                selectionModel()->select(childIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                needSync = true;
            }
        }
    }

    if (needSync)
    {
        TreeSelectionChanged(selectionModel()->selection(), QItemSelection());
    }

    PropagateSolidFlag();
}

void SceneTree::TreeItemCollapsed(const QModelIndex& index)
{
    treeModel->SetSolid(filteringProxyModel->mapToSource(index), true);

    bool needSync = false;
    {
        DAVA::TArc::ScopedValueGuard<bool> guard(isInSelectionSync, true);

        // if selected items were inside collapsed item, remove them from selection
        QModelIndexList indexList = selectionModel()->selection().indexes();
        for (int i = 0; i < indexList.size(); ++i)
        {
            QModelIndex childIndex = indexList[i];
            QModelIndex childParent = childIndex.parent();
            while (childParent.isValid())
            {
                if (childParent == index)
                {
                    selectionModel()->select(childIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                    needSync = true;
                    break;
                }

                childParent = childParent.parent();
            }
        }
    }

    if (needSync)
    {
        TreeSelectionChanged(selectionModel()->selection(), QItemSelection());
    }
}

void SceneTree::TreeItemExpanded(const QModelIndex& index)
{
    QModelIndex mappedIndex = filteringProxyModel->mapToSource(index);
    treeModel->SetSolid(mappedIndex, false);
    QStandardItem* item = treeModel->itemFromIndex(mappedIndex);
    PropagateSolidFlagRecursive(item);
}

void SceneTree::SyncSelectionToTree()
{
    SCOPED_VALUE_GUARD(bool, isInSelectionSync, true, void());

    SceneEditor2* curScene = treeModel->GetScene();
    if (curScene == nullptr)
    {
        return;
    }

    using TSelectionMap = DAVA::Map<QModelIndex, DAVA::Vector<QModelIndex>>;
    TSelectionMap toSelect;

    QModelIndex lastValidIndex;
    const SelectableGroup& selection = Selection::GetSelection();
    for (const auto& item : selection.GetContent())
    {
        QModelIndex sIndex = filteringProxyModel->mapFromSource(treeModel->GetIndex(item.GetContainedObject()));
        if (sIndex.isValid())
        {
            lastValidIndex = sIndex;
            toSelect[sIndex.parent()].push_back(sIndex);
        }
    }

    QItemSelectionModel* selectModel = selectionModel();
    selectModel->clear();

    if (toSelect.empty())
        return;

    QItemSelectionModel::SelectionFlags selectionMode = QItemSelectionModel::Current | QItemSelectionModel::Select | QItemSelectionModel::Rows;
    QItemSelection itemSelection;

    for (TSelectionMap::value_type& selectionNode : toSelect)
    {
        DAVA::Vector<QModelIndex>& indexes = selectionNode.second;
        sort(indexes.begin(), indexes.end(), [](const QModelIndex& left, const QModelIndex& right) {
            DVASSERT(left.parent() == right.parent());
            return left.row() < right.row();
        });

        int startIndex = 0;
        int lastIndex = startIndex;
        int lastRow = indexes[lastIndex].row();
        for (size_t i = 1; i < indexes.size(); ++i)
        {
            int currentRow = indexes[i].row();
            if (currentRow - lastRow < 2)
            {
                DVASSERT(currentRow - lastRow > 0);
                lastRow = currentRow;
                lastIndex = static_cast<DAVA::int32>(i);
            }
            else
            {
                QItemSelection subRange(indexes[startIndex], indexes[lastIndex]);
                itemSelection.merge(subRange, selectionMode);
                startIndex = static_cast<DAVA::int32>(i);
                lastIndex = startIndex;
                lastRow = indexes[lastIndex].row();
            }
        }
        QItemSelection subRange(indexes[startIndex], indexes[lastIndex]);
        itemSelection.merge(subRange, selectionMode);
    }

    selectModel->select(itemSelection, selectionMode);
    if (lastValidIndex.isValid())
    {
        selectModel->setCurrentIndex(lastValidIndex, QItemSelectionModel::Current);
        scrollTo(lastValidIndex, QAbstractItemView::EnsureVisible);
    }
}

void SceneTree::SyncSelectionFromTree()
{
    SCOPED_VALUE_GUARD(bool, isInSelectionSync, true, void());

    SceneEditor2* curScene = treeModel->GetScene();
    if (nullptr != curScene)
    {
        // select items in scene
        SelectableGroup group;
        QModelIndexList indexList = selectionModel()->selection().indexes();
        for (int i = 0; i < indexList.size(); ++i)
        {
            SceneTreeItem* item = treeModel->GetItem(filteringProxyModel->mapToSource(indexList[i]));
            DAVA::Any object(item->GetItemObject());
            group.Add(object, curScene->collisionSystem->GetUntransformedBoundingBox(object));
        }
        Selection::SetSelection(group);
    }
}

void SceneTree::ExpandFilteredItems()
{
    QSet<QModelIndex> indexSet;
    BuildExpandItemsSet(indexSet);

    for (auto i = indexSet.begin(); i != indexSet.end(); ++i)
    {
        expand(*i);
    }
}

void SceneTree::BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent)
{
    const int n = filteringProxyModel->rowCount(parent);
    for (int i = 0; i < n; i++)
    {
        const QModelIndex _index = filteringProxyModel->index(i, 0, parent);
        SceneTreeItem* item = treeModel->GetItem(filteringProxyModel->mapToSource(_index));
        if (item->IsHighlighed())
        {
            indexSet << _index.parent();
        }
        BuildExpandItemsSet(indexSet, _index);
    }
}

void SceneTree::UpdateTree()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void SceneTree::UpdateModel()
{
    const QSignalBlocker guard(selectionModel());
    treeModel->ResyncStructure(treeModel->invisibleRootItem(), treeModel->GetScene());
    treeModel->ReloadFilter();
    filteringProxyModel->invalidate();

    SyncSelectionToTree();

    if (treeModel->IsFilterSet())
    {
        ExpandFilteredItems();
    }
}

void SceneTree::PropagateSolidFlag()
{
    QStandardItem* root = treeModel->invisibleRootItem();
    for (int i = 0; i < root->rowCount(); ++i)
    {
        PropagateSolidFlagRecursive(root->child(i));
    }
}

void SceneTree::PropagateSolidFlagRecursive(QStandardItem* root)
{
    DVASSERT(root != nullptr);
    QModelIndex rootIndex = root->index();
    DVASSERT(rootIndex.isValid());
    QModelIndex filteredIndex = filteringProxyModel->mapFromSource(rootIndex);
    if (isExpanded(filteredIndex))
    {
        treeModel->SetSolid(rootIndex, false);
        for (int i = 0; i < root->rowCount(); ++i)
        {
            PropagateSolidFlagRecursive(root->child(i));
        }
    }
    else
    {
        treeModel->SetSolid(rootIndex, true);
    }
}
