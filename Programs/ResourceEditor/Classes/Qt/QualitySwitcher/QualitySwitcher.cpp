#include "QualitySwitcher.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/Settings/RESettings.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/Systems/EditorMaterialSystem.h>
#include <REPlatform/Commands/ParticleEditorCommands.h>
#include <REPlatform/Scene/SceneEditor2.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/Deprecated.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/Set.h>
#include <Render/RenderBase.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Material/NMaterial.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

QualitySwitcher* QualitySwitcher::switcherDialog = nullptr;
const QString AnisotropyComboName = "AnisotropyCombo";
const QString MultiSamplingComboName = "MSAACombo";
const QString ShadowComboName = "ShadowCombo";
const QString RenderFlowComboName = "RenderFlowCombo";
const QString ApplyButtonName = "ApplyButton";

QualitySwitcher::QualitySwitcher()
    : QDialog(DAVA::Deprecated::GetUI()->GetWindow(DAVA::mainWindowKey), Qt::Dialog | Qt::WindowStaysOnTopHint) //https://bugreports.qt.io/browse/QTBUG-34767
{
    const int spacing = 5;
    const int minColumnW = 150;
    int currentRow = 1;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QGroupBox* generalGroup = new QGroupBox(this);
    QGroupBox* optionsGroup = new QGroupBox(this);
    QWidget* buttonsGroup = new QWidget(this);

    QGridLayout* generalLayout = new QGridLayout(generalGroup);
    generalLayout->setColumnMinimumWidth(0, minColumnW);
    generalLayout->setColumnMinimumWidth(1, minColumnW);

    generalGroup->setTitle("General");
    generalGroup->setLayout(generalLayout);

    {
        QLabel* labRenderFlow = new QLabel("Render Flow:", generalGroup);
        generalLayout->addWidget(labRenderFlow, currentRow, 0);

        QComboBox* comboFlow = new QComboBox(generalGroup);
        comboFlow->addItem("LDR Forward", QVariant(int(DAVA::RenderFlow::LDRForward)));
        comboFlow->addItem("HDR Forward", QVariant(int(DAVA::RenderFlow::HDRForward)));
        comboFlow->addItem("HDR Deferred", QVariant(int(DAVA::RenderFlow::HDRDeferred)));
        if (rhi::DeviceCaps().isFramebufferFetchSupported)
        {
            comboFlow->addItem("Tile Based HDR Forward", QVariant(int(DAVA::RenderFlow::TileBasedHDRForward)));
            comboFlow->addItem("Tiled Based HDR Deferred", QVariant(int(DAVA::RenderFlow::TileBasedHDRDeferred)));
        }
        comboFlow->setObjectName(RenderFlowComboName);
        comboFlow->setCurrentIndex(static_cast<int>(DAVA::Renderer::GetCurrentRenderFlow()) - 1);
        generalLayout->addWidget(comboFlow, currentRow, 1);
        QObject::connect(comboFlow, SIGNAL(activated(int)), this, SLOT(OnSetSettingsDirty(int)));

        ++currentRow;
    }

    for (uint32 i = 0; i < QualityGroup::Count; ++i)
    {
        QualityGroup group = static_cast<QualityGroup>(i);
        const FastName& qualityGroupName = DAVA::QualitySettingsSystem::Instance()->GetQualityGroupName(group);
        const Vector<FastName>& qualityGroupValues = DAVA::QualitySettingsSystem::Instance()->GetAvailableQualitiesForGroup(group);

        if (!qualityGroupValues.empty())
        {
            QLabel* label = new QLabel(qualityGroupName.c_str(), generalGroup);
            generalLayout->addWidget(label, currentRow, 0);

            QComboBox* combo = new QComboBox(generalGroup);
            combo->setObjectName(qualityGroupName.c_str());
            generalLayout->addWidget(combo, currentRow, 1);

            FastName currentQuality = DAVA::QualitySettingsSystem::Instance()->GetCurrentQualityForGroup(group);
            for (const DAVA::FastName& i : qualityGroupValues)
            {
                combo->addItem(i.c_str());
                if (currentQuality == i)
                {
                    combo->setCurrentIndex(combo->count() - 1);
                }
            }

            QObject::connect(combo, SIGNAL(activated(int)), this, SLOT(OnSetSettingsDirty(int)));
            ++currentRow;
        }
    }

    // quality options
    {
        QGridLayout* optionsLayout = new QGridLayout(optionsGroup);
        optionsLayout->setColumnMinimumWidth(0, minColumnW);
        optionsLayout->setColumnMinimumWidth(1, minColumnW);

        optionsGroup->setTitle("Options");
        optionsGroup->setLayout(optionsLayout);

        DAVA::int32 optionsCount = DAVA::QualitySettingsSystem::Instance()->GetOptionsCount();
        for (DAVA::int32 i = 0; i < optionsCount; ++i)
        {
            DAVA::FastName optionName = DAVA::QualitySettingsSystem::Instance()->GetOptionName(i);

            QLabel* labOp = new QLabel(QString(optionName.c_str()) + ":", optionsGroup);
            optionsLayout->addWidget(labOp, i, 0);

            QCheckBox* checkOp = new QCheckBox(optionsGroup);
            checkOp->setObjectName(QString(optionName.c_str()) + "CheckBox");
            checkOp->setChecked(DAVA::QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
            checkOp->setProperty("qualityOptionName", QVariant(optionName.c_str()));
            optionsLayout->addWidget(checkOp, i, 1);
            QObject::connect(checkOp, SIGNAL(clicked(bool)), this, SLOT(OnOptionClick(bool)));
        }
    }

    // buttons
    {
        QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsGroup);
        buttonsGroup->setLayout(buttonsLayout);

        QPushButton* buttonOk = new QPushButton("OK", buttonsGroup);
        connect(buttonOk, &QPushButton::released, this, &QualitySwitcher::OnOkPressed);

        QPushButton* buttonCancel = new QPushButton("Cancel", buttonsGroup);
        connect(buttonCancel, &QPushButton::released, this, &QualitySwitcher::OnCancelPressed);

        QPushButton* buttonApply = new QPushButton("Apply", buttonsGroup);
        buttonApply->setObjectName(ApplyButtonName);
        connect(buttonApply, &QPushButton::released, this, &QualitySwitcher::OnApplyPressed);

        buttonsLayout->addStretch();
        buttonsLayout->addWidget(buttonOk);
        buttonsLayout->addWidget(buttonCancel);
        buttonsLayout->addWidget(buttonApply);
        buttonsLayout->setMargin(5);
        buttonsLayout->setSpacing(spacing);
    }

    mainLayout->addWidget(generalGroup);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonsGroup);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(spacing);

    setLayout(mainLayout);

    SetSettingsDirty(false);

    adjustSize();
}

QualitySwitcher::~QualitySwitcher()
{
    switcherDialog = nullptr;
}

void QualitySwitcher::UpdateEntitiesToQuality(DAVA::Entity* e)
{
    DAVA::QualitySettingsSystem::Instance()->UpdateEntityVisibility(e);
    for (DAVA::int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        UpdateEntitiesToQuality(e->GetChild(i));
    }
}

void QualitySwitcher::UpdateParticlesToQuality()
{
    SceneSignals* sceneSignals = SceneSignals::Instance();
    DAVA::Deprecated::GetAccessor()->ForEachContext([sceneSignals, this](const DAVA::DataContext& ctx) {
        DAVA::SceneEditor2* scene = ctx.GetData<DAVA::SceneData>()->GetScene().Get();
        scene->BeginBatch("Switch particle quality");
        ReloadEntityEmitters(scene, scene);
        sceneSignals->EmitStructureChanged(scene, nullptr);
        scene->EndBatch();
    });
}

void QualitySwitcher::ReloadEntityEmitters(DAVA::SceneEditor2* scene, DAVA::Entity* e)
{
    DAVA::ParticleEffectComponent* comp = GetEffectComponent(e);
    if (comp)
    {
        scene->Exec(std::make_unique<DAVA::CommandReloadEmitters>(comp));
    }

    for (DAVA::int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        ReloadEntityEmitters(scene, e->GetChild(i));
    }
}

void QualitySwitcher::SetSettingsDirty(bool dirty)
{
    settingsDirty = dirty;
    QPushButton* applyButton = findChild<QPushButton*>(ApplyButtonName);
    applyButton->setEnabled(settingsDirty);
}

void QualitySwitcher::ApplySettings()
{
    bool changedQualities[QualityGroup::Count] = {};

    bool materialSettingsChanged = false;
    bool optionSettingsChanged = false;
    {
        for (uint32 i = 0; i < QualityGroup::Count; ++i)
        {
            QualityGroup group = static_cast<QualityGroup>(i);
            const FastName& qualityGroupName = DAVA::QualitySettingsSystem::Instance()->GetQualityGroupName(group);
            QComboBox* combo = findChild<QComboBox*>(qualityGroupName.c_str());
            if (combo == nullptr)
                continue;

            // const Vector<FastName>& qualityGroupValues = DAVA::QualitySettingsSystem::Instance()->GetAvailableQualitiesForGroup(group);
            DAVA::FastName selectedQuality(combo->currentText().toLatin1());
            if (selectedQuality != DAVA::QualitySettingsSystem::Instance()->GetCurrentQualityForGroup(group))
            {
                DAVA::QualitySettingsSystem::Instance()->SetCurrentQualityForGroup(group, selectedQuality);
                changedQualities[group] = true;
            }
        }

        QComboBox* combo = findChild<QComboBox*>(RenderFlowComboName);
        if (combo != nullptr)
        {
            DAVA::RenderFlow currentFlow = static_cast<DAVA::RenderFlow>(combo->currentData().toInt());
            DAVA::Renderer::SetRenderFlow(currentFlow);
        }
    }

    // options
    {
        DAVA::int32 optionsCount = DAVA::QualitySettingsSystem::Instance()->GetOptionsCount();
        for (DAVA::int32 i = 0; i < optionsCount; ++i)
        {
            DAVA::FastName optionName = DAVA::QualitySettingsSystem::Instance()->GetOptionName(i);
            QCheckBox* checkBox = findChild<QCheckBox*>(QString(optionName.c_str()) + "CheckBox");
            if (nullptr != checkBox)
            {
                DAVA::FastName optionName(checkBox->property("qualityOptionName").toString().toStdString().c_str());
                bool checked = checkBox->isChecked();
                if (DAVA::QualitySettingsSystem::Instance()->IsOptionEnabled(optionName) != checked)
                {
                    DAVA::QualitySettingsSystem::Instance()->EnableOption(optionName, checked);
                    optionSettingsChanged = true;
                }
            }
        }
    }

    if (changedQualities[QualityGroup::Shadow])
    {
        ShadowQuality shadowQuality = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Shadow>();
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_CASCADES, shadowQuality.cascadesCount);
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_PCF, shadowQuality.enablePCF ? 1 : 0);
        Renderer::GetRuntimeTextures().InvalidateTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
        materialSettingsChanged = true;
    }

    if (changedQualities[QualityGroup::Scattering])
    {
        ScatteringQuality scatteringQuality = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Scattering>();
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::ATMOSPHERE_SCATTERING_SAMPLES, scatteringQuality.scatteringSamples);
        materialSettingsChanged = true;
    }

    if (materialSettingsChanged)
    {
        DAVA::Deprecated::GetInvoker()->Invoke(DAVA::ReloadShaders.ID);
    }

    if (materialSettingsChanged || optionSettingsChanged)
    {
        DAVA::Deprecated::GetAccessor()->ForEachContext([&](const DAVA::DataContext& ctx) {
            DAVA::SceneEditor2* scene = ctx.GetData<DAVA::SceneData>()->GetScene().Get();
            UpdateEntitiesToQuality(scene);
            scene->foliageSystem->SyncFoliageWithLandscape();
        });
        SceneSignals::Instance()->EmitQualityChanged();
    }
}

void QualitySwitcher::ShowDialog()
{
    if (switcherDialog == nullptr)
    {
        //we don't need synchronization because of working in UI thread
        switcherDialog = new QualitySwitcher();
        switcherDialog->setAttribute(Qt::WA_DeleteOnClose, true);

        switcherDialog->show();
    }

    switcherDialog->raise();
    switcherDialog->activateWindow();
}

void QualitySwitcher::OnSetSettingsDirty(int index)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnParticlesTagsCloudChanged(const QString& text)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnOptionClick(bool checked)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnOkPressed()
{
    if (settingsDirty)
    {
        ApplySettings();
    }

    accept();
}

void QualitySwitcher::OnCancelPressed()
{
    reject();
}

void QualitySwitcher::OnApplyPressed()
{
    SetSettingsDirty(false);

    ApplySettings();
}
