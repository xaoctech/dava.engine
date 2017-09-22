#include "Classes/Qt/DockParticleEditor/LayerDragForceWidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>

#include "Classes/Qt/DockParticleEditor/ParticleVector3Widget.h"
#include "Classes/Qt/DockParticleEditor/TimeLineWidget.h"
#include "Classes/Qt/DockParticleEditor/WheellIgnorantComboBox.h"
#include "Classes/Qt/Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"
#include "Classes/Commands2/ParticleEditorCommands.h"

#include <Base/Array.h>
#include <Base/Map.h>

namespace LayerDragForceWidgetDetail
{
struct ShapeMap
{
    DAVA::ParticleDragForce::eShape elemType;
    QString name;
};
const DAVA::Array<ShapeMap, 2> shapeMap =
{ {
{ DAVA::ParticleDragForce::eShape::BOX, "Box" },
{ DAVA::ParticleDragForce::eShape::SPHERE, "Sphere" }
} };

struct TimingMap
{
    DAVA::ParticleDragForce::eTimingType elemType;
    QString name;
};
const DAVA::Array<TimingMap, 3> timingMap =
{ {
{ DAVA::ParticleDragForce::eTimingType::CONSTANT, "Constant" },
{ DAVA::ParticleDragForce::eTimingType::OVER_LAYER_LIFE, "Over layer life" },
{ DAVA::ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE, "Over particle life" }
} };

DAVA::Map<DAVA::ParticleDragForce::eType, QString> forceTypes =
{
    { DAVA::ParticleDragForce::eType::DRAG_FORCE, "Drag Force" },
    { DAVA::ParticleDragForce::eType::LORENTZ_FORCE, "Lorentz Force" },
    { DAVA::ParticleDragForce::eType::GRAVITY, "Gravity" },
    { DAVA::ParticleDragForce::eType::WIND, "Wind" },
    { DAVA::ParticleDragForce::eType::POINT_GRAVITY, "Point Gravity" },
    { DAVA::ParticleDragForce::eType::BOX_WRAP, "Box Wrap" }
};

template <typename T, typename U, size_t sz>
int ElementToIndex(T elem, const DAVA::Array<U, sz> map)
{
    for (size_t i = 0; i < map.size(); ++i)
    {
        if (map[i].elemType == elem)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}
}

LayerDragForceWidget::LayerDragForceWidget(QWidget* parent /* = nullptr */)
    : BaseParticleEditorContentWidget(parent)
{
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    BuildCommonSection();
    BuildGravitySection();
    BuildShapeSection();
    BuildTimingSection();
    BuilDirectionSection();
    BuildWindSection();
    BuildPointGravitySection();
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    blockSignals = false;
}

void LayerDragForceWidget::BuildTimingSection()
{
    using namespace LayerDragForceWidgetDetail;

    timingTypeSeparator = new QFrame();
    timingTypeSeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(timingTypeSeparator);

    timingLabel = new QLabel("Timing type:");
    mainLayout->addWidget(timingLabel);

    timingTypeComboBox = new WheellIgnorantComboBox();
    for (size_t i = 0; i < timingMap.size(); ++i)
        timingTypeComboBox->addItem(timingMap[i].name);

    connect(timingTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(OnValueChanged()));

    mainLayout->addWidget(timingTypeComboBox);

    forcePowerLabel = new QLabel("Force power:");
    mainLayout->addWidget(forcePowerLabel);

    forcePowerTimeLine = new TimeLineWidget(this);
    connect(forcePowerTimeLine, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(forcePowerTimeLine);

    forcePower = new ParticleVector3Widget("Force power", DAVA::Vector3::Zero);
    connect(forcePower, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(forcePower);

    forcePowerSpin = new EventFilterDoubleSpinBox();
    SetupSpin(forcePowerSpin);
    mainLayout->addWidget(forcePowerSpin);

    QHBoxLayout* freqLayout = new QHBoxLayout(this);
    windFreqLabel = new QLabel("Wind frequency:");
    windFreqSpin = new EventFilterDoubleSpinBox();
    SetupSpin(windFreqSpin);
    freqLayout->addWidget(windFreqLabel);
    freqLayout->addWidget(windFreqSpin);
    mainLayout->addLayout(freqLayout);

    QHBoxLayout* biasLayout = new QHBoxLayout(this);
    windBiasLabel = new QLabel("Wind bias:");
    windBiasSpin = new EventFilterDoubleSpinBox();
    SetupSpin(windBiasSpin);
    biasLayout->addWidget(windBiasLabel);
    biasLayout->addWidget(windBiasSpin);
    mainLayout->addLayout(biasLayout);
}

void LayerDragForceWidget::BuildShapeSection()
{
    using namespace LayerDragForceWidgetDetail;

    shapeSeparator = new QFrame();
    shapeSeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(shapeSeparator);

    shapeLabel = new QLabel("Shape:");
    mainLayout->addWidget(shapeLabel);

    shapeComboBox = new WheellIgnorantComboBox();
    for (size_t i = 0; i < shapeMap.size(); ++i)
        shapeComboBox->addItem(shapeMap[i].name);
    connect(shapeComboBox, SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(OnValueChanged()));
    mainLayout->addWidget(shapeComboBox);

    boxSize = new ParticleVector3Widget("Box size", DAVA::Vector3::Zero);
    connect(boxSize, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(boxSize);

    radiusWidget = new QWidget();
    mainLayout->addWidget(radiusWidget);
    QHBoxLayout* layout = new QHBoxLayout(radiusWidget);
    QLabel* radiusLabel = new QLabel("Radius");

    radiusSpin = new EventFilterDoubleSpinBox();
    radiusSpin->setSingleStep(0.01);
    radiusSpin->setDecimals(3);
    connect(radiusSpin, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
    radiusSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(radiusLabel);
    layout->addWidget(radiusSpin);
}

void LayerDragForceWidget::BuildCommonSection()
{
    forceTypeLabel = new QLabel("OLOLABEL");
    forceTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    forceTypeLabel->setContentsMargins(0, 15, 0, 15);
    forceTypeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(forceTypeLabel);

    isActive = new QCheckBox("Is active");
    connect(isActive, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(isActive);

    forceNameEdit = new QLineEdit();
    mainLayout->addWidget(forceNameEdit);
    connect(forceNameEdit, SIGNAL(editingFinished()), this, SLOT(OnValueChanged()));

    infinityRange = new QCheckBox("Use infinity range");
    connect(infinityRange, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(infinityRange);
}

void LayerDragForceWidget::UpdateVisibility(DAVA::ParticleDragForce::eShape shape, DAVA::ParticleDragForce::eTimingType timingType, DAVA::ParticleDragForce::eType forceType, bool isInfinityRange)
{
    using Shape = DAVA::ParticleDragForce::eShape;
    using TimingType = DAVA::ParticleDragForce::eTimingType;
    using ForceType = DAVA::ParticleDragForce::eType;
    bool isGravity = forceType == ForceType::GRAVITY;
    bool isWind = forceType == ForceType::WIND;
    bool isDirectionalForce = forceType == ForceType::LORENTZ_FORCE || forceType == ForceType::WIND;
    bool isPointGravity = forceType == ForceType::POINT_GRAVITY;

    boxSize->setVisible(shape == Shape::BOX && !isInfinityRange && !isGravity);
    radiusWidget->setVisible(shape == Shape::SPHERE && !isInfinityRange && !isGravity);
    shapeComboBox->setVisible(!isInfinityRange && !isGravity);
    shapeLabel->setVisible(!isInfinityRange && !isGravity);
    shapeSeparator->setVisible(!isInfinityRange && !isGravity);

    forcePower->setVisible(timingType == TimingType::CONSTANT && !isGravity && !isWind);
    forcePowerTimeLine->setVisible(timingType != TimingType::CONSTANT && !isGravity);
    forcePowerLabel->setVisible((timingType != TimingType::CONSTANT && !isGravity) || (timingType == TimingType::CONSTANT && isWind));
    forcePowerSpin->setVisible(timingType == TimingType::CONSTANT && isWind);

    direction->setVisible(isDirectionalForce);
    directionSeparator->setVisible(isDirectionalForce);

    // Gravity
    infinityRange->setVisible(!isGravity);
    gravitySeparator->setVisible(isGravity);
    gravityLabel->setVisible(isGravity);
    gravitySpin->setVisible(isGravity);
    timingTypeComboBox->setVisible(!isGravity);
    timingTypeSeparator->setVisible(!isGravity);
    timingLabel->setVisible(!isGravity);

    // Wind
    windSeparator->setVisible(isWind);
    windFreqLabel->setVisible(isWind);
    windFreqSpin->setVisible(isWind);
    windTurbLabel->setVisible(timingType == TimingType::CONSTANT && isWind);
    windTurbSpin->setVisible(timingType == TimingType::CONSTANT && isWind);
    windTurbFreqLabel->setVisible(isWind);
    windTurbFreqSpin->setVisible(isWind);
    windBiasLabel->setVisible(isWind);
    windBiasSpin->setVisible(isWind);
    turbulenceTimeLine->setVisible(timingType != TimingType::CONSTANT && isWind);
    backTurbLabel->setVisible(isWind);
    backTurbSpin->setVisible(isWind);

    // PointGravity
    pointGravitySeparator->setVisible(isPointGravity);
    pointGravityRadiusLabel->setVisible(isPointGravity);
    pointGravityRadiusSpin->setVisible(isPointGravity);
    pointGravityUseRnd->setVisible(isPointGravity);
}

void LayerDragForceWidget::SetupSpin(EventFilterDoubleSpinBox* spin, DAVA::float32 singleStep /*= 0.0001*/, DAVA::int32 decimals /*= 4*/)
{
    spin->setMinimum(-100000000000000000000.0);
    spin->setMaximum(100000000000000000000.0);
    spin->setSingleStep(singleStep);
    spin->setDecimals(decimals);
    connect(spin, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
    spin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void LayerDragForceWidget::BuilDirectionSection()
{
    directionSeparator = new QFrame();
    directionSeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(directionSeparator);
    direction = new ParticleVector3Widget("Force direction", DAVA::Vector3::Zero);
    connect(direction, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(direction);
}

void LayerDragForceWidget::BuildGravitySection()
{
    gravityWidget = new QWidget(this);
    gravitySeparator = new QFrame(gravityWidget);
    gravitySeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(gravitySeparator);

    QHBoxLayout* layout = new QHBoxLayout(this);
    gravityLabel = new QLabel("Gravity force:");
    gravitySpin = new EventFilterDoubleSpinBox();
    SetupSpin(gravitySpin);
    layout->addWidget(gravityLabel);
    layout->addWidget(gravitySpin);
    mainLayout->addLayout(layout);
}

void LayerDragForceWidget::BuildWindSection()
{
    windSeparator = new QFrame();
    windSeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(windSeparator);

    QHBoxLayout* turbLayout = new QHBoxLayout(this);
    windTurbLabel = new QLabel("Wind turbulence:");
    windTurbSpin = new EventFilterDoubleSpinBox();
    SetupSpin(windTurbSpin);
    turbLayout->addWidget(windTurbLabel);
    turbLayout->addWidget(windTurbSpin);
    mainLayout->addLayout(turbLayout);

    turbulenceTimeLine = new TimeLineWidget(this);
    connect(turbulenceTimeLine, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(turbulenceTimeLine);

    QHBoxLayout* turbFreqLayout = new QHBoxLayout(this);
    windTurbFreqLabel = new QLabel("Wind turbulence frequency:");
    windTurbFreqSpin = new EventFilterDoubleSpinBox();
    SetupSpin(windTurbFreqSpin);
    turbFreqLayout->addWidget(windTurbFreqLabel);
    turbFreqLayout->addWidget(windTurbFreqSpin);
    mainLayout->addLayout(turbFreqLayout);

    QHBoxLayout* backTurbLayout = new QHBoxLayout(this);
    backTurbLabel = new QLabel("Backward turbulence probability:");
    backTurbSpin = new EventFilterDoubleSpinBox();
    SetupSpin(backTurbSpin, 1, 0);
    backTurbLayout->addWidget(backTurbLabel);
    backTurbLayout->addWidget(backTurbSpin);
    mainLayout->addLayout(backTurbLayout);
}

void LayerDragForceWidget::BuildPointGravitySection()
{
    pointGravitySeparator = new QFrame();
    pointGravitySeparator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(pointGravitySeparator);

    QHBoxLayout* pointGravityRadLayout = new QHBoxLayout(this);
    pointGravityRadiusLabel = new QLabel("Point gravity radius:");
    pointGravityRadiusSpin = new EventFilterDoubleSpinBox();
    SetupSpin(pointGravityRadiusSpin);
    pointGravityRadLayout->addWidget(pointGravityRadiusLabel);
    pointGravityRadLayout->addWidget(pointGravityRadiusSpin);
    mainLayout->addLayout(pointGravityRadLayout);
    pointGravityUseRnd = new QCheckBox("Random points on sphere:");
    connect(pointGravityUseRnd, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(pointGravityUseRnd);
}

void LayerDragForceWidget::Init(SceneEditor2* scene, DAVA::ParticleLayer* layer_, DAVA::uint32 forceIndex_, bool updateMinimized)
{
    using namespace DAVA;
    using namespace LayerDragForceWidgetDetail;

    using LineWrapper = PropLineWrapper<Vector3>;
    using LineHelper = PropertyLineHelper;

    if (!layer_ || layer_->GetDragForces().size() <= forceIndex_ || blockSignals)
        return;

    layer = layer_;
    forceIndex = forceIndex_;
    blockSignals = true;

    selectedForce = layer->GetDragForces()[forceIndex];
    infinityRange->setChecked(selectedForce->isInfinityRange);
    isActive->setChecked(selectedForce->isActive);
    boxSize->SetValue(selectedForce->boxSize);
    forcePower->SetValue(selectedForce->forcePower);
    radiusSpin->setValue(selectedForce->radius);
    forceNameEdit->setText(QString::fromStdString(selectedForce->forceName));
    forceTypeLabel->setText(forceTypes[selectedForce->type]);
    direction->SetValue(selectedForce->direction);
    gravitySpin->setValue(selectedForce->forcePower.z);
    windTurbSpin->setValue(selectedForce->windTurbulence);
    windTurbFreqSpin->setValue(selectedForce->windTurbulenceFrequency);
    windFreqSpin->setValue(selectedForce->windFrequency);
    windBiasSpin->setValue(selectedForce->windBias);
    backTurbSpin->setValue(selectedForce->backwardTurbulenceProbability);
    forcePowerSpin->setValue(selectedForce->forcePower.x);
    pointGravityRadiusSpin->setValue(selectedForce->pointGravityRadius);
    pointGravityUseRnd->setChecked(selectedForce->pointGravityUseRandomPointsOnSphere);

    UpdateVisibility(selectedForce->shape, selectedForce->timingType, selectedForce->type, selectedForce->isInfinityRange);

    static const Vector<QColor> colors{ Qt::red, Qt::darkGreen, Qt::blue };
    static const Vector<QString> legends{ "Force x", "Force y", "Force z" };
    static const Vector<QString> windLegends{ "Wind force", "none", "none" };
    const Vector<QString>* currLegends = nullptr;
    if (selectedForce->type == ParticleDragForce::eType::WIND)
        currLegends = &windLegends;
    else
        currLegends = &legends;

    forcePowerTimeLine->Init(0, 1, updateMinimized, true, false);
    forcePowerTimeLine->AddLines(LineWrapper(LineHelper::GetValueLine(selectedForce->forcePowerLine)).GetProps(), colors, *currLegends);
    forcePowerTimeLine->EnableLock(true);

    turbulenceTimeLine->Init(0, 1, updateMinimized);
    turbulenceTimeLine->AddLine(0, PropLineWrapper<float32>(LineHelper::GetValueLine(selectedForce->turbulenceLine)).GetProps(), Qt::red, "Turbulence");
    turbulenceTimeLine->EnableLock(true);

    shapeComboBox->setCurrentIndex(ElementToIndex(selectedForce->shape, shapeMap));
    timingTypeComboBox->setCurrentIndex(ElementToIndex(selectedForce->timingType, timingMap));

    blockSignals = false;
}

void LayerDragForceWidget::Update()
{
    Init(GetActiveScene(), layer, forceIndex, false);
}

void LayerDragForceWidget::StoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;

    DAVA::KeyedArchive* props = new DAVA::KeyedArchive();
    forcePowerTimeLine->GetVisualState(props);
    visualStateProps->SetArchive("FORCE_PROPS", props);

    props->DeleteAllKeys();
    turbulenceTimeLine->SetVisualState(props);
    visualStateProps->SetArchive("TURB_PROPS", props);
    DAVA::SafeRelease(props);
}

void LayerDragForceWidget::RestoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;
    forcePowerTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_PROPS"));
    turbulenceTimeLine->SetVisualState(visualStateProps->GetArchive("TURB_PROPS"));
}

void LayerDragForceWidget::OnValueChanged()
{
    using namespace DAVA;
    using namespace LayerDragForceWidgetDetail;
    using Shape = ParticleDragForce::eShape;
    using TimingType = ParticleDragForce::eTimingType;
    using ForceType = ParticleDragForce::eType;

    if (blockSignals)
        return;

    Shape shape = shapeMap[shapeComboBox->currentIndex()].elemType;
    TimingType timingType = timingMap[timingTypeComboBox->currentIndex()].elemType;

    PropLineWrapper<Vector3> propForce;
    forcePowerTimeLine->GetValues(propForce.GetPropsPtr());

    PropLineWrapper<float32> propTurb;
    turbulenceTimeLine->GetValue(0, propTurb.GetPropsPtr());

    CommandUpdateParticleDragForce::ForceParams params;
    params.isActive = isActive->isChecked();
    params.forceName = forceNameEdit->text().toStdString();
    params.shape = shape;
    params.timingType = timingType;
    params.boxSize = boxSize->GetValue();
    if (selectedForce->type == ForceType::WIND)
        params.forcePower = Vector3(forcePowerSpin->value(), 0.0f, 0.0f);
    else
        params.forcePower = forcePower->GetValue();

    params.direction = direction->GetValue();
    params.useInfinityRange = infinityRange->isChecked();
    params.radius = radiusSpin->value();
    params.forcePowerLine = propForce.GetPropLine();
    params.turbulenceLine = propTurb.GetPropLine();
    params.windFrequency = windFreqSpin->value();
    params.windTurbulence = windTurbSpin->value();
    params.windTurbulenceFrequency = windTurbFreqSpin->value();
    params.windBias = windBiasSpin->value();
    params.backwardTurbulenceProbability = static_cast<uint32>(Clamp(backTurbSpin->value(), 0.0, 100.0));
    params.pointGravityRadius = pointGravityRadiusSpin->value();
    params.pointGravityUseRandomPointsOnSphere = pointGravityUseRnd->isChecked();

    backTurbSpin->setValue(params.backwardTurbulenceProbability);

    if (selectedForce->type == ForceType::GRAVITY)
        params.forcePower.z = gravitySpin->value();

    UpdateVisibility(shape, timingType, selectedForce->type, params.useInfinityRange);

    shapeComboBox->setCurrentIndex(ElementToIndex(shape, shapeMap));
    timingTypeComboBox->setCurrentIndex(ElementToIndex(timingType, timingMap));

    std::unique_ptr<CommandUpdateParticleDragForce> updateDragForceCmd(new CommandUpdateParticleDragForce(layer, forceIndex, std::move(params)));

    SceneEditor2* activeScene = GetActiveScene();
    DVASSERT(activeScene != nullptr);
    activeScene->Exec(std::move(updateDragForceCmd));
    activeScene->MarkAsChanged();

    emit ValueChanged();
}
