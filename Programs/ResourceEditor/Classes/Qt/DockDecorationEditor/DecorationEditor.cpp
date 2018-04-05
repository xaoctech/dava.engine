#include "Classes/Qt/DockDecorationEditor/DecorationEditor.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"

#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectionData.h>

#include <TArc/Core/Deprecated.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/FilePathEdit.h>

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/Landscape.h>
#include <Scene3D/Components/ComponentHelpers.h>

#include <QtWidgets/QtWidgets>

class DensityTableItemDelegate;

DAVA_REFLECTION_IMPL(DecorationEditor)
{
    DAVA::ReflectionRegistrator<DecorationEditor>::Begin()
    .Field("decorationPath", &DecorationEditor::GetDecorationPath, &DecorationEditor::SetDecorationPath)[DAVA::Meta<SceneFileMeta, DAVA::Metas::File>("All(*.sc2);; SC2(*.sc2);")]
    .Field("hasData", &DecorationEditor::HasDecorationData, nullptr)
    .End();
}

DecorationEditor::DecorationEditor(DAVA::ContextAccessor* contextAccessor_, DAVA::UI* ui_, QWidget* parent)
    : QWidget(parent)
    , contextAccessor(contextAccessor_)
    , ui(ui_)
{
    selectionFieldBinder.reset(new DAVA::FieldBinder(contextAccessor));

    DAVA::FieldDescriptor fieldDescr;
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<DAVA::SelectionData>();
    fieldDescr.fieldName = DAVA::FastName(DAVA::SelectionData::selectionPropertyName);
    selectionFieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &DecorationEditor::OnSelectionChanged));

    SetupUI();

    connect(layerListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DecorationEditor::OnLayerSelectionChanged);
    connect(variationListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DecorationEditor::OnVariationSelectionChanged);

    connect(baseLevelEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DecorationEditor::OnSpinBoxChanged);
    connect(levelCountEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DecorationEditor::OnSpinBoxChanged);

    connect(cullfaceCheckBox, &QCheckBox::stateChanged, this, &DecorationEditor::OnCheckBoxChanged);
    connect(orientCheckbox, &QCheckBox::stateChanged, this, &DecorationEditor::OnCheckBoxChanged);
    connect(tintCheckBox, &QCheckBox::stateChanged, this, &DecorationEditor::OnCheckBoxChanged);
    connect(collisionCheckBox, &QCheckBox::stateChanged, this, &DecorationEditor::OnCheckBoxChanged);
    connect(enabledCheckBox, &QCheckBox::stateChanged, this, &DecorationEditor::OnCheckBoxChanged);

    connect(layerIndexEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DecorationEditor::OnSpinBoxChanged);
    connect(orientEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(tintHeightEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(scaleMinEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(scaleMaxEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(pitchMaxEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(collisionGroupEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DecorationEditor::OnSpinBoxChanged);
    connect(collisionRadiusEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);
    connect(densityEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DecorationEditor::OnDoubleSpinBoxChanged);

    connect(densityTableModel, &QStandardItemModel::dataChanged, this, &DecorationEditor::OnDensityTableChanged);

    UpdateDecorationSettings();
    UpdateLayerSettings();
    UpdateVariationSettings();
}

void DecorationEditor::OnSelectionChanged(const DAVA::Any& selection)
{
    decorationData = nullptr;

    if (selection.CanGet<DAVA::SelectableGroup>())
    {
        const DAVA::SelectableGroup& selectionGroup = selection.Get<DAVA::SelectableGroup>();
        if (selectionGroup.GetSize() == 1)
        {
            const auto& obj = selectionGroup.GetFirst();
            if (obj.CanBeCastedTo<DAVA::Entity>())
            {
                DAVA::Landscape* landscape = DAVA::GetLandscape(obj.AsEntity());
                if (landscape)
                {
                    decorationData = landscape->GetDecorationData();
                    subdivision = landscape->GetSubdivision();
                }
            }
        }
    }

    UpdateLevelDistances();
    UpdateDecorationSettings();
}

DAVA::FilePath DecorationEditor::GetDecorationPath() const
{
    return (decorationData != nullptr) ? decorationData->GetDecorationPath() : DAVA::FilePath();
}

void DecorationEditor::SetDecorationPath(const DAVA::FilePath& decorationPath)
{
    DVASSERT(decorationData != nullptr);
    decorationData->SetDecorationPath(decorationPath);
    layerListWidget->selectionModel()->clearSelection();

    UpdateDecorationSettings();
    MarkSceneChanged();
}

bool DecorationEditor::HasDecorationData() const
{
    return decorationData != nullptr;
}

void DecorationEditor::MarkSceneChanged()
{
    DAVA::DataContext* activeContext = contextAccessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        activeContext->GetData<DAVA::SceneData>()->GetScene()->SetChanged();
    }
}

void DecorationEditor::OnLayerSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    selectedLayer = layerListWidget->selectionModel()->selectedIndexes();
    variationListWidget->selectionModel()->clearSelection();

    UpdateLayerSettings();
    UpdateVariationSettings();
}

void DecorationEditor::OnVariationSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    selectedVariations = variationListWidget->selectionModel()->selectedIndexes();
    UpdateVariationSettings();
}

void DecorationEditor::OnSpinBoxChanged(int value)
{
    if (decorationData == nullptr)
        return;

    if (uiUpdateState)
        return;

    QObject* obj = sender();
    if (obj == baseLevelEdit)
    {
        decorationData->SetBaseLevel(DAVA::uint32(value));
        UpdateDecorationSettings();
    }
    else if (obj == levelCountEdit)
    {
        decorationData->SetLevelCount(DAVA::uint32(value));
        UpdateLevelDistances();
        UpdateVariationSettings();
        UpdateDecorationSettings();
    }
    else if (obj == layerIndexEdit)
    {
        DAVA::uint32 layerIndex = selectedLayer.last().row();
        decorationData->SetLayerMaskIndex(layerIndex, DAVA::uint8(value));

        UpdateLayerSettings();
        MarkSceneChanged();
    }
    else if (obj == collisionGroupEdit)
    {
        DAVA::uint32 layerIndex = selectedLayer.last().row();
        for (QModelIndex& index : selectedVariations)
        {
            DAVA::uint32 varIndex = DAVA::uint32(index.row());
            decorationData->SetVariationCollisionGroup(layerIndex, varIndex, value);
        }
        UpdateVariationSettings();
    }

    MarkSceneChanged();
}

void DecorationEditor::OnCheckBoxChanged(int state)
{
    if (decorationData == nullptr)
        return;

    if (selectedLayer.empty())
        return;

    if (uiUpdateState)
        return;

    DAVA::uint32 layerIndex = selectedLayer.last().row();
    bool value = (state == Qt::Checked);

    QObject* obj = sender();
    if (obj == tintCheckBox)
    {
        decorationData->SetLayerTint(layerIndex, value);
    }
    else if (obj == collisionCheckBox)
    {
        decorationData->SetLayerCollisionDetection(layerIndex, value);
    }
    else if (obj == cullfaceCheckBox)
    {
        decorationData->SetLayerCullface(layerIndex, value);
    }
    else if (obj == orientCheckbox)
    {
        decorationData->SetLayerOrientOnLandscape(layerIndex, value);
    }
    else if (obj == enabledCheckBox)
    {
        for (QModelIndex& index : selectedVariations)
            decorationData->SetVariationEnabled(layerIndex, DAVA::uint32(index.row()), value);
    }

    UpdateLayerSettings();
    MarkSceneChanged();
}

void DecorationEditor::OnDoubleSpinBoxChanged(double valueDouble)
{
    if (decorationData == nullptr)
        return;

    if (selectedLayer.empty())
        return;

    if (uiUpdateState)
        return;

    QObject* obj = sender();
    DAVA::float32 value = DAVA::float32(valueDouble);

    DAVA::uint32 layerIndex = selectedLayer.last().row();
    if (obj == orientEdit)
    {
        decorationData->SetLayerOrientValue(layerIndex, value);
        UpdateLayerSettings();
    }
    else if (obj == tintHeightEdit)
    {
        decorationData->SetLayerTintHeight(layerIndex, value);
        UpdateLayerSettings();
    }
    else
    {
        for (QModelIndex& index : selectedVariations)
        {
            DAVA::uint32 varIndex = DAVA::uint32(index.row());

            if (obj == densityEdit)
                decorationData->SetVariationDensity(layerIndex, varIndex, value);
            else if (obj == collisionRadiusEdit)
                decorationData->SetVariationCollisionRadius(layerIndex, varIndex, value);
            else if (obj == scaleMinEdit)
                decorationData->SetVariationScaleMin(layerIndex, varIndex, value);
            else if (obj == scaleMaxEdit)
                decorationData->SetVariationScaleMax(layerIndex, varIndex, value);
            else if (obj == pitchMaxEdit)
                decorationData->SetVariationPitchMax(layerIndex, varIndex, value);
        }
        UpdateVariationSettings();
    }

    MarkSceneChanged();
}

void DecorationEditor::OnDensityTableChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    if (decorationData == nullptr)
        return;

    if (selectedLayer.empty())
        return;

    if (bottomRight.column() != 1)
        return;

    if (uiUpdateState)
        return;

    DAVA::uint32 layerIndex = selectedLayer.last().row();

    for (QModelIndex& index : selectedVariations)
    {
        DAVA::uint32 varIndex = DAVA::uint32(index.row());
        DAVA::uint32 level = DAVA::uint32(bottomRight.row());
        DAVA::float32 value = densityTableModel->data(bottomRight, Qt::UserRole).toFloat();

        decorationData->SetLevelDensity(layerIndex, varIndex, level, value);
    }

    UpdateVariationSettings();
    MarkSceneChanged();
}

void DecorationEditor::UpdateDecorationSettings()
{
    if (decorationData)
    {
        uiUpdateState = true;

        editorContent->setVisible(true);

        DAVA::uint32 baseLevel = decorationData->GetBaseLevel();
        DAVA::uint32 levelCount = decorationData->GetLevelCount();

        baseLevelEdit->setValue(baseLevel);
        levelCountEdit->setValue(levelCount);

        DAVA::uint32 minLevel = (baseLevel < levelCount) ? 0 : (baseLevel - levelCount);
        DAVA::float32 maxDistance = subdivision->GetSubdivisionDistance(minLevel);
        maxDistanceLabel->setText(QString("Max Distance: ~%1m").arg(int(maxDistance)));

        DAVA::uint32 layersCount = decorationData->GetLayersCount();
        layerListModel->setRowCount(layersCount);
        for (DAVA::uint32 l = 0; l < layersCount; ++l)
        {
            DAVA::FastName name = decorationData->GetLayerName(l);
            QString layerName = name.IsValid() ? QString::fromStdString(name.c_str()) : QStringLiteral("#noname-layer");
            layerListModel->setData(layerListModel->index(l, 0), layerName, Qt::DisplayRole);
        }

        uiUpdateState = false;
    }
    else
    {
        editorContent->setVisible(false);
    }
}

void DecorationEditor::UpdateLayerSettings()
{
    if (decorationData && selectedLayer.size())
    {
        uiUpdateState = true;

        DAVA::uint32 layerIndex = selectedLayer.last().row();

        cullfaceCheckBox->setChecked(decorationData->GetLayerCullface(layerIndex));
        orientCheckbox->setChecked(decorationData->GetLayerOrientOnLandscape(layerIndex));
        collisionCheckBox->setChecked(decorationData->GetLayerCollisionDetection(layerIndex));
        orientEdit->setValue(decorationData->GetLayerOrientValue(layerIndex));

        tintCheckBox->setChecked(decorationData->GetLayerTint(layerIndex));
        tintHeightEdit->setValue(decorationData->GetLayerTintHeight(layerIndex));
        layerIndexEdit->setValue(decorationData->GetLayerMaskIndex(layerIndex));

        DAVA::uint32 variationCount = decorationData->GetVariationCount(layerIndex);
        variationListModel->setRowCount(variationCount);
        for (DAVA::uint32 v = 0; v < variationCount; ++v)
        {
            DAVA::FastName name = decorationData->GetVariationName(layerIndex, v);
            QString varName = name.IsValid() ? QString::fromStdString(name.c_str()) : QStringLiteral("#noname-variation");
            variationListModel->setData(variationListModel->index(v, 0), varName, Qt::DisplayRole);
        }

        layerSettingsWidget->setVisible(true);

        uiUpdateState = false;
    }
    else
    {
        layerSettingsWidget->setVisible(false);
    }
}

void DecorationEditor::UpdateVariationSettings()
{
    if (decorationData && selectedLayer.size() && selectedVariations.size())
    {
        uiUpdateState = true;

        DAVA::uint32 layerIndex = selectedLayer.last().row();
        DAVA::uint32 varIndex = selectedVariations.last().row();

        enabledCheckBox->setChecked(decorationData->GetVariationEnabled(layerIndex, varIndex));
        scaleMinEdit->setValue(decorationData->GetVariationScaleMin(layerIndex, varIndex));
        scaleMaxEdit->setValue(decorationData->GetVariationScaleMax(layerIndex, varIndex));
        pitchMaxEdit->setValue(decorationData->GetVariationPitchMax(layerIndex, varIndex));
        collisionGroupEdit->setValue(int(decorationData->GetVariationCollisionGroup(layerIndex, varIndex)));
        collisionRadiusEdit->setValue(decorationData->GetVariationCollisionRadius(layerIndex, varIndex));
        densityEdit->setValue(decorationData->GetVariationDensity(layerIndex, varIndex));

        DAVA::uint32 levelCount = decorationData->GetLevelCount();
        densityTableModel->setRowCount(levelCount);
        for (DAVA::uint32 l = 0; l < levelCount; ++l)
        {
            densityTableModel->setData(densityTableModel->index(l, 0), QString("Level #%1 (~%2m)").arg(l).arg(int(levelDistances[l])), Qt::DisplayRole);

            DAVA::float32 levelDensity = decorationData->GetLevelDensity(layerIndex, varIndex, l);
            densityTableModel->setData(densityTableModel->index(l, 1), levelDensity, Qt::UserRole);
            densityTableModel->setData(densityTableModel->index(l, 1), QString("%1%").arg(int(levelDensity * 100)), Qt::DisplayRole);
        }

        variationSettingsWidget->setVisible(true);

        uiUpdateState = false;
    }
    else
    {
        variationSettingsWidget->setVisible(false);
    }
}

void DecorationEditor::UpdateLevelDistances()
{
    if (decorationData && subdivision)
    {
        DAVA::uint32 baseLevel = decorationData->GetBaseLevel();
        DAVA::uint32 levelCount = decorationData->GetLevelCount();

        levelDistances.resize(levelCount);
        for (DAVA::uint32 l = 0; l < levelCount; ++l)
        {
            DAVA::uint32 level0 = (baseLevel < l) ? 0 : baseLevel - l;
            DAVA::uint32 level1 = (level0 != 0) ? (level0 - 1) : level0;

            DAVA::float32 distance0 = subdivision->GetSubdivisionDistance(level0);
            DAVA::float32 distance1 = subdivision->GetSubdivisionDistance(level1);
            levelDistances[l] = (distance0 + distance1) / 2.f;
        }
    }
}

void DecorationEditor::SetupUI()
{
    using namespace DAVA;
    setWindowTitle(QStringLiteral("Decoration Editor"));

    QVBoxLayout* editorLayout = new QVBoxLayout(this);
    editorLayout->setMargin(0);

    QScrollArea* editorScrollArea = new QScrollArea(this);
    editorScrollArea->setWidgetResizable(true);
    editorLayout->addWidget(editorScrollArea);

    editorContent = new QWidget(editorScrollArea);
    editorScrollArea->setWidget(editorContent);
    QVBoxLayout* contentLayout = new QVBoxLayout(editorContent);

    QtHBoxLayout* pathBoxLayout = new QtHBoxLayout();
    {
        pathBoxLayout->addWidget(new QLabel(QStringLiteral("Decoration path:"), editorContent));

        FilePathEdit::Params params(contextAccessor, ui, DAVA::mainWindowKey);
        params.fields[FilePathEdit::Fields::IsEnabled] = "hasData";
        params.fields[FilePathEdit::Fields::Value] = "decorationPath";

        pathBoxLayout->AddControl(new FilePathEdit(params, contextAccessor, Reflection::Create(ReflectedObject(this)), editorContent));
    }
    contentLayout->addLayout(pathBoxLayout);

    decorationSettingsWidget = new QGroupBox(editorContent);
    decorationSettingsWidget->setTitle(QStringLiteral("Decoration Settings"));
    {
        QVBoxLayout* decorationSettingsLayout = new QVBoxLayout(decorationSettingsWidget);

        QHBoxLayout* baseLevelLayout = new QHBoxLayout();
        {
            baseLevelLayout->addWidget(new QLabel(QStringLiteral("Base Level:"), decorationSettingsWidget));

            baseLevelLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            baseLevelEdit = new QSpinBox(decorationSettingsWidget);
            baseLevelEdit->setMinimumSize(QSize(60, 0));
            baseLevelLayout->addWidget(baseLevelEdit);
        }
        decorationSettingsLayout->addLayout(baseLevelLayout);

        QHBoxLayout* levelCountLayout = new QHBoxLayout();
        {
            levelCountLayout->addWidget(new QLabel(QStringLiteral("Level Count:"), decorationSettingsWidget));
            levelCountLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            levelCountEdit = new QSpinBox(decorationSettingsWidget);
            levelCountEdit->setMinimumSize(QSize(60, 0));
            levelCountLayout->addWidget(levelCountEdit);
        }
        decorationSettingsLayout->addLayout(levelCountLayout);

        maxDistanceLabel = new QLabel(decorationSettingsWidget);
        decorationSettingsLayout->addWidget(maxDistanceLabel);

        decorationSettingsLayout->addWidget(new QLabel(QStringLiteral("Layers:"), decorationSettingsWidget));

        layerListWidget = new QListView(decorationSettingsWidget);
        layerListModel = new QStandardItemModel(layerListWidget);
        layerListModel->setColumnCount(1);
        layerListWidget->setModel(layerListModel);
        layerListWidget->setSelectionModel(new QItemSelectionModel());
        layerListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        layerListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        layerListWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        decorationSettingsLayout->addWidget(layerListWidget);
    }
    contentLayout->addWidget(decorationSettingsWidget);

    layerSettingsWidget = new QGroupBox(editorContent);
    layerSettingsWidget->setTitle(QStringLiteral("Layer Settings"));
    {
        QVBoxLayout* layerSettingsLayout = new QVBoxLayout(layerSettingsWidget);

        QHBoxLayout* maskSettingLayout = new QHBoxLayout();
        {
            maskSettingLayout->addWidget(new QLabel(QStringLiteral("Layer Index:"), layerSettingsWidget));
            maskSettingLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            layerIndexEdit = new QSpinBox(layerSettingsWidget);
            layerIndexEdit->setSingleStep(1);
            layerIndexEdit->setRange(0, 255);
            maskSettingLayout->addWidget(layerIndexEdit);
        }
        layerSettingsLayout->addLayout(maskSettingLayout);

        cullfaceCheckBox = new QCheckBox(QStringLiteral("Cull backface"), layerSettingsWidget);
        layerSettingsLayout->addWidget(cullfaceCheckBox);

        collisionCheckBox = new QCheckBox(QStringLiteral("Collision Detection"), layerSettingsWidget);
        layerSettingsLayout->addWidget(collisionCheckBox);

        QHBoxLayout* orientSettingLayout = new QHBoxLayout();
        {
            orientCheckbox = new QCheckBox(QStringLiteral("Orient on Landscape"), layerSettingsWidget);
            orientSettingLayout->addWidget(orientCheckbox);

            orientEdit = new QDoubleSpinBox(layerSettingsWidget);
            orientEdit->setMinimumSize(QSize(60, 0));
            orientEdit->setDecimals(2);
            orientEdit->setSingleStep(0.01);
            orientEdit->setRange(0.0, 1.0);
            orientSettingLayout->addWidget(orientEdit);
        }
        layerSettingsLayout->addLayout(orientSettingLayout);

        QHBoxLayout* tintLayout = new QHBoxLayout();
        {
            tintCheckBox = new QCheckBox(QStringLiteral("Tint with Landscape material"), layerSettingsWidget);
            tintLayout->addWidget(tintCheckBox);

            tintHeightEdit = new QDoubleSpinBox(layerSettingsWidget);
            tintHeightEdit->setMinimumSize(QSize(60, 0));
            tintHeightEdit->setDecimals(2);
            tintHeightEdit->setSingleStep(0.01);
            tintHeightEdit->setMinimum(0.0);
            tintLayout->addWidget(tintHeightEdit);
        }
        layerSettingsLayout->addLayout(tintLayout);

        layerSettingsLayout->addWidget(new QLabel(QStringLiteral("Variations:"), layerSettingsWidget));

        variationListWidget = new QListView(layerSettingsWidget);
        variationListModel = new QStandardItemModel();
        variationListModel->setColumnCount(1);
        variationListWidget->setModel(variationListModel);
        variationListWidget->setSelectionModel(new QItemSelectionModel());
        variationListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        variationListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        variationListWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        layerSettingsLayout->addWidget(variationListWidget);
    }
    contentLayout->addWidget(layerSettingsWidget);

    variationSettingsWidget = new QGroupBox(editorContent);
    variationSettingsWidget->setTitle(QStringLiteral("Variation Settings"));
    {
        QVBoxLayout* variationSettingsLayout = new QVBoxLayout(variationSettingsWidget);

        enabledCheckBox = new QCheckBox(QStringLiteral("Show"), variationSettingsWidget);
        variationSettingsLayout->addWidget(enabledCheckBox);

        QHBoxLayout* scaleLayout = new QHBoxLayout();
        {
            scaleLayout->addWidget(new QLabel(QStringLiteral("Scale:"), variationSettingsWidget));
            scaleLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            scaleLayout->addWidget(new QLabel(QStringLiteral("min:"), variationSettingsWidget));
            scaleMinEdit = new QDoubleSpinBox(variationSettingsWidget);
            scaleMinEdit->setMinimumSize(QSize(60, 0));
            scaleMinEdit->setDecimals(2);
            scaleMinEdit->setSingleStep(0.01);
            scaleMinEdit->setRange(0.0, 10.0);
            scaleLayout->addWidget(scaleMinEdit);

            scaleLayout->addWidget(new QLabel(QStringLiteral(" max:"), variationSettingsWidget));
            scaleMaxEdit = new QDoubleSpinBox(variationSettingsWidget);
            scaleMaxEdit->setMinimumSize(QSize(60, 0));
            scaleMaxEdit->setDecimals(2);
            scaleMaxEdit->setSingleStep(0.01);
            scaleMaxEdit->setRange(0.0, 10.0);
            scaleLayout->addWidget(scaleMaxEdit);
        }
        variationSettingsLayout->addLayout(scaleLayout);

        QHBoxLayout* pitchLayout = new QHBoxLayout();
        {
            pitchLayout->addWidget(new QLabel(QStringLiteral("Pitch:"), variationSettingsWidget));
            pitchLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            pitchLayout->addWidget(new QLabel(QStringLiteral(" max:"), variationSettingsWidget));
            pitchMaxEdit = new QDoubleSpinBox(variationSettingsWidget);
            pitchMaxEdit->setMinimumSize(QSize(60, 0));
            pitchMaxEdit->setDecimals(1);
            pitchMaxEdit->setRange(0.0, 90.0);
            pitchMaxEdit->setSingleStep(1.0);
            pitchMaxEdit->setSuffix(QStringLiteral("\260")); //degrees suffix
            pitchLayout->addWidget(pitchMaxEdit);
        }
        variationSettingsLayout->addLayout(pitchLayout);

        QHBoxLayout* collisionGroupLayout = new QHBoxLayout();
        {
            collisionGroupLayout->addWidget(new QLabel(QStringLiteral("Collision Group:"), variationSettingsWidget));
            collisionGroupLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            collisionGroupEdit = new QSpinBox(variationSettingsWidget);
            collisionGroupEdit->setSingleStep(1);
            collisionGroupEdit->setRange(0, 100);
            collisionGroupLayout->addWidget(collisionGroupEdit);
        }
        variationSettingsLayout->addLayout(collisionGroupLayout);

        QHBoxLayout* collisionRadiusLayout = new QHBoxLayout();
        {
            collisionRadiusLayout->addWidget(new QLabel(QStringLiteral("Item Radius:"), variationSettingsWidget));
            collisionRadiusLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            collisionRadiusEdit = new QDoubleSpinBox(variationSettingsWidget);
            collisionRadiusEdit->setDecimals(3);
            collisionRadiusEdit->setSingleStep(0.001);
            collisionRadiusLayout->addWidget(collisionRadiusEdit);
        }
        variationSettingsLayout->addLayout(collisionRadiusLayout);

        QHBoxLayout* densityLayout = new QHBoxLayout();
        {
            densityLayout->addWidget(new QLabel(QStringLiteral("Density (per m\262):"), variationSettingsWidget));
            densityLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            densityEdit = new QDoubleSpinBox(variationSettingsWidget);
            densityEdit->setDecimals(3);
            densityEdit->setSingleStep(0.001);
            densityEdit->setRange(0.0, 1000.0);
            densityLayout->addWidget(densityEdit);
        }
        variationSettingsLayout->addLayout(densityLayout);

        variationSettingsLayout->addWidget(new QLabel(QStringLiteral("Density Distribution:"), variationSettingsWidget));

        densityTableWidget = new QTableView(variationSettingsWidget);
        {
            densityTableModel = new QStandardItemModel();
            densityTableModel->setColumnCount(2);
            densityTableWidget->setModel(densityTableModel);
            densityTableWidget->setItemDelegate(new DensityTableItemDelegate(densityTableWidget));
            densityTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
            densityTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

            densityTableWidget->horizontalHeader()->setVisible(false);
            densityTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            densityTableWidget->verticalHeader()->setVisible(false);
            densityTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
        variationSettingsLayout->addWidget(densityTableWidget);
    }
    contentLayout->addWidget(variationSettingsWidget);
    contentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

//////////////////////////////////////////////////////////////////////////

QWidget* DensityTableItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget* editor = nullptr;
    if (index.column() == 1)
    {
        QSpinBox* spinbox = new QSpinBox(parent);
        spinbox->setRange(0, 100);
        spinbox->setSingleStep(1);
        spinbox->setSuffix(QStringLiteral("%"));
        spinbox->setEnabled(index.row() != 0);
        editor = spinbox;
    }
    return editor;
}

void DensityTableItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == 1)
    {
        bool isOk = false;
        DAVA::float32 value = index.model()->data(index, Qt::UserRole).toFloat(&isOk);
        QSpinBox* spinbox = static_cast<QSpinBox*>(editor);
        if (isOk)
            spinbox->setValue(int(value * 100));
        else
            spinbox->clear();
    }
}

void DensityTableItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() == 1)
    {
        QSpinBox* spinbox = static_cast<QSpinBox*>(editor);
        spinbox->interpretText();
        int value = spinbox->value();
        model->setData(index, QString("%1%").arg(value), Qt::DisplayRole);
        model->setData(index, DAVA::float32(value) / 100.f, Qt::UserRole);
    }
}

void DensityTableItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    editor->setGeometry(option.rect);
}

//////////////////////////////////////////////////////////////////////////

DensityTableItemDelegate::DensityTableItemDelegate(QObject* parent)
    :
    QItemDelegate(parent)
{
}

void DecorationEditorModule::PostInit()
{
    DecorationEditor* widget = new DecorationEditor(GetAccessor(), GetUI(), nullptr);

    DAVA::DockPanelInfo dockInfo;
    dockInfo.title = "Decoration Editor";
    DAVA::PanelKey panelKey(QStringLiteral("DecorationDock"), dockInfo);
    GetUI()->AddView(DAVA::mainWindowKey, panelKey, widget);
}

DAVA_VIRTUAL_REFLECTION_IMPL(DecorationEditorModule)
{
    DAVA::ReflectionRegistrator<DecorationEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(DecorationEditorModule);
