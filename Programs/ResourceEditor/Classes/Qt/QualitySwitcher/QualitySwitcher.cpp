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
QString ApplyButtonName = "ApplyButton";

QualitySwitcher::QualitySwitcher()
    : QDialog(DAVA::Deprecated::GetUI()->GetWindow(DAVA::mainWindowKey), Qt::Dialog | Qt::WindowStaysOnTopHint) //https://bugreports.qt.io/browse/QTBUG-34767
{
    using namespace DAVA;

    const int spacing = 5;
    const int minColumnW = 150;
    int currentRow = 1;

    QualitySettingsSystem* qs = QualitySettingsSystem::Instance();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QGroupBox* generalGroup = new QGroupBox(this);
    QGroupBox* optionsGroup = new QGroupBox(this);
    QWidget* buttonsGroup = new QWidget(this);

    QGridLayout* generalLayout = new QGridLayout(generalGroup);
    generalLayout->setColumnMinimumWidth(0, minColumnW);
    generalLayout->setColumnMinimumWidth(1, minColumnW);

    generalGroup->setTitle("General");
    generalGroup->setLayout(generalLayout);

    for (uint32 i = 0; i < QualityGroup::Count; ++i)
    {
        QualityGroup group = static_cast<QualityGroup>(i);
        const FastName& qualityGroupName = qs->GetQualityGroupName(group);
        const Vector<FastName>& qualityGroupValues = qs->GetAvailableQualitiesForGroup(group);

        if (!qualityGroupValues.empty())
        {
            QLabel* label = new QLabel(qualityGroupName.c_str(), generalGroup);
            generalLayout->addWidget(label, currentRow, 0);

            QComboBox* combo = new QComboBox(generalGroup);
            combo->setObjectName(qualityGroupName.c_str());
            generalLayout->addWidget(combo, currentRow, 1);

            FastName currentQuality = qs->GetCurrentQualityForGroup(group);

            for (const FastName& val : qualityGroupValues)
            {
                bool isValidValue = (i == QualityGroup::RenderFlowType) ?
                Renderer::IsRenderFlowAllowed(qs->GetQualityValue<QualityGroup::RenderFlowType>(val)) :
                true;

                if (isValidValue)
                {
                    combo->addItem(val.c_str());

                    if (currentQuality == val)
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

        int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
        for (int32 i = 0; i < optionsCount; ++i)
        {
            FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);

            QLabel* labOp = new QLabel(QString(optionName.c_str()) + ":", optionsGroup);
            optionsLayout->addWidget(labOp, i, 0);

            QCheckBox* checkOp = new QCheckBox(optionsGroup);
            checkOp->setObjectName(QString(optionName.c_str()) + "CheckBox");
            checkOp->setChecked(QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
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
    using namespace DAVA;

    bool changedQualities[QualityGroup::Count] = {};

    bool materialSettingsChanged = false;
    bool optionSettingsChanged = false;
    {
        for (DAVA::uint32 i = 0; i < DAVA::QualityGroup::Count; ++i)
        {
            QualityGroup group = static_cast<QualityGroup>(i);
            const FastName& qualityGroupName = QualitySettingsSystem::Instance()->GetQualityGroupName(group);
            QComboBox* combo = findChild<QComboBox*>(qualityGroupName.c_str());
            if (combo == nullptr)
                continue;

            FastName selectedQuality(combo->currentText().toLatin1());
            if (selectedQuality != QualitySettingsSystem::Instance()->GetCurrentQualityForGroup(group))
            {
                QualitySettingsSystem::Instance()->SetCurrentQualityForGroup(group, selectedQuality);
                changedQualities[group] = true;
            }
        }
    }

    // options
    {
        int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
        for (int32 i = 0; i < optionsCount; ++i)
        {
            FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);
            QCheckBox* checkBox = findChild<QCheckBox*>(QString(optionName.c_str()) + "CheckBox");
            if (nullptr != checkBox)
            {
                FastName optionName(checkBox->property("qualityOptionName").toString().toStdString().c_str());
                bool checked = checkBox->isChecked();
                if (QualitySettingsSystem::Instance()->IsOptionEnabled(optionName) != checked)
                {
                    QualitySettingsSystem::Instance()->EnableOption(optionName, checked);
                    optionSettingsChanged = true;
                }
            }
        }
    }

    if (changedQualities[QualityGroup::RenderFlowType])
    {
        RenderFlow currentFlow = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::RenderFlowType>();

        ContextAccessor* accessor = Deprecated::GetAccessor();
        PropertiesItem item = accessor->CreatePropertiesNode("renderFlow");
        item.Set("renderFlow", currentFlow);

        Renderer::SetRenderFlow(currentFlow);
    }

    if (changedQualities[QualityGroup::Shadow])
    {
        ShadowQuality shadowQuality = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Shadow>();
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_CASCADES, std::min(uint32(MAX_SHADOW_CASCADES), shadowQuality.cascadesCount));
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_PCF, shadowQuality.enablePCF ? 1 : 0);
        Renderer::GetRuntimeTextures().InvalidateTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
        materialSettingsChanged = true;
    }

    if (changedQualities[DAVA::QualityGroup::Scattering])
    {
        DAVA::ScatteringQuality scatteringQuality = DAVA::QualitySettingsSystem::Instance()->GetCurrentQualityValue<DAVA::QualityGroup::Scattering>();
        DAVA::Renderer::GetRuntimeFlags().SetFlag(DAVA::RuntimeFlags::Flag::ATMOSPHERE_SCATTERING_SAMPLES, scatteringQuality.scatteringSamples);
        materialSettingsChanged = true;
    }

    if (materialSettingsChanged)
    {
        Deprecated::GetInvoker()->Invoke(ReloadShaders.ID);
    }

    if (materialSettingsChanged || optionSettingsChanged)
    {
        Deprecated::GetAccessor()->ForEachContext([&](const DataContext& ctx) {
            SceneEditor2* scene = ctx.GetData<SceneData>()->GetScene().Get();
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
