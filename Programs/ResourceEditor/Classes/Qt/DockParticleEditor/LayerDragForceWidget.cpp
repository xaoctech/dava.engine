#include "Classes/Qt/DockParticleEditor/LayerDragForceWidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>

#include "Classes/Qt/DockParticleEditor/ParticleVector3Widget.h"
#include "Classes/Qt/DockParticleEditor/WheellIgnorantComboBox.h"
#include "Classes/Qt/Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"
#include "Classes/Commands2/ParticleEditorCommands.h"

#include <Particles/ParticleDragForce.h>

namespace LayerDragForceWidgetDetail
{
struct ShapeMap
{
    DAVA::ParticleDragForce::eShape shape;
    QString shapeName;
};
const ShapeMap shapeMap[] =
{
    { DAVA::ParticleDragForce::eShape::BOX, "Box" },
    { DAVA::ParticleDragForce::eShape::SPHERE, "Sphere" }
};

struct TimingMap
{
    DAVA::ParticleDragForce::eTimingType timingType;
    QString name;
};
const TimingMap timingMap[] =
{
    { DAVA::ParticleDragForce::eTimingType::CONSTANT, "Constant" },
    { DAVA::ParticleDragForce::eTimingType::OVER_LAYER_LIFE, "Over layer life" },
    { DAVA::ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE, "Over particle life"}
};
}

LayerDragForceWidget::LayerDragForceWidget(QWidget* parent /* = nullptr */)
    : BaseParticleEditorContentWidget(parent)
{
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    isActive = new QCheckBox("Is active");
    connect(isActive, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(isActive);

    forceNameEdit = new QLineEdit();
    mainLayout->addWidget(forceNameEdit);
    connect(forceNameEdit, SIGNAL(editingFinished()), this, SLOT(OnValueChanged()));

    infinityRange = new QCheckBox("Use infinity range");
    connect(infinityRange, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(infinityRange);

    shapeSeparator = new QFrame();
    shapeSeparator->setFrameShape(QFrame::HLine);

    //line->setFrameShadow(QFrame::Raised);
    mainLayout->addWidget(shapeSeparator);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);

    shapeLabel = new QLabel("Shape:");
    mainLayout->addWidget(shapeLabel);

    shapeComboBox = new WheellIgnorantComboBox();
    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);
    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        shapeComboBox->addItem(LayerDragForceWidgetDetail::shapeMap[i].shapeName);

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
    radiusSpin = new EventFilterDoubleSpinBox();
    QLabel* radiusLabel = new QLabel("Radius");

    layout->addWidget(radiusLabel);
    layout->addWidget(radiusSpin);

    radiusSpin->setSingleStep(0.01);
    radiusSpin->setDecimals(3);
    connect(radiusSpin, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
    radiusSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    mainLayout->addWidget(line);

    QLabel* timingLabel = new QLabel("Timing type:");
    mainLayout->addWidget(timingLabel);
    timingTypeComboBox = new WheellIgnorantComboBox();
    DAVA::int32 timingTypes = sizeof(LayerDragForceWidgetDetail::timingMap) / sizeof(*LayerDragForceWidgetDetail::timingMap);
    for (DAVA::int32 i = 0; i < timingTypes; ++i)
        timingTypeComboBox->addItem(LayerDragForceWidgetDetail::timingMap[i].name);
    connect(timingTypeComboBox, SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(OnValueChanged()));

    mainLayout->addWidget(timingTypeComboBox);

    forcePower = new ParticleVector3Widget("Force power", DAVA::Vector3::Zero);
    connect(forcePower, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));

    forcePowerLabel = new QLabel("Force power:");
    mainLayout->addWidget(forcePowerLabel);
    forcePowerTimeLine = new TimeLineWidget(this);
    mainLayout->addWidget(forcePowerTimeLine);
    connect(forcePowerTimeLine, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));

    mainLayout->addWidget(forcePower);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    blockSignals = false;
}

void LayerDragForceWidget::Init(SceneEditor2* scene, DAVA::ParticleLayer* layer_, DAVA::uint32 forceIndex_, bool updateMinimized)
{
    using LineWrapper = DAVA::PropLineWrapper<DAVA::Vector3>;
    using LineHelper = DAVA::PropertyLineHelper;

    if (!layer_ || layer_->GetDragForces().size() <= forceIndex_ || blockSignals)
        return;

    layer = layer_;
    forceIndex = forceIndex_;
    blockSignals = true;

    DAVA::ParticleDragForce* currForce = layer->GetDragForces()[forceIndex];
    infinityRange->setChecked(currForce->isInfinityRange);
    isActive->setChecked(currForce->isActive);
    boxSize->SetValue(currForce->boxSize);
    forcePower->SetValue(currForce->forcePower);
    radiusSpin->setValue(currForce->radius);
    boxSize->setVisible(currForce->shape == DAVA::ParticleDragForce::eShape::BOX && !currForce->isInfinityRange);
    radiusWidget->setVisible(currForce->shape == DAVA::ParticleDragForce::eShape::SPHERE && !currForce->isInfinityRange);
    shapeComboBox->setVisible(!currForce->isInfinityRange);
    shapeLabel->setVisible(!currForce->isInfinityRange);
    shapeSeparator->setVisible(!currForce->isInfinityRange);
    forceNameEdit->setText(QString::fromStdString(currForce->forceName));

    forcePower->setVisible(currForce->timingType == DAVA::ParticleDragForce::eTimingType::CONSTANT);
    forcePowerTimeLine->setVisible(currForce->timingType != DAVA::ParticleDragForce::eTimingType::CONSTANT);
    forcePowerLabel->setVisible(currForce->timingType != DAVA::ParticleDragForce::eTimingType::CONSTANT);

    static const DAVA::Vector<QColor> colors{ Qt::red, Qt::darkGreen, Qt::blue };
    static const DAVA::Vector<QString> legends{ "Force x", "Force y", "Force z" };

    forcePowerTimeLine->Init(0, 1, updateMinimized, true, false);
    forcePowerTimeLine->AddLines(LineWrapper(LineHelper::GetValueLine(currForce->forcePowerLine)).GetProps(), colors, legends);
    forcePowerTimeLine->EnableLock(true);

    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);
    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        if (LayerDragForceWidgetDetail::shapeMap[i].shape == currForce->shape)
        {
            shapeComboBox->setCurrentIndex(i);
            break;
        }

    DAVA::int32 timingTypes = sizeof(LayerDragForceWidgetDetail::timingMap) / sizeof(*LayerDragForceWidgetDetail::timingMap);
    for (DAVA::int32 i = 0; i < timingTypes; ++i)
        if (LayerDragForceWidgetDetail::timingMap[i].timingType == currForce->timingType)
        {
            timingTypeComboBox->setCurrentIndex(i);
            break;
        }

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
    DAVA::SafeRelease(props);
}

void LayerDragForceWidget::RestoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;
    forcePowerTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_PROPS"));
}

void LayerDragForceWidget::OnValueChanged()
{
    if (blockSignals)
        return;

    DAVA::ParticleDragForce::eShape shape = LayerDragForceWidgetDetail::shapeMap[shapeComboBox->currentIndex()].shape;
    DAVA::ParticleDragForce::eTimingType timingType = LayerDragForceWidgetDetail::timingMap[timingTypeComboBox->currentIndex()].timingType;

    DAVA::PropLineWrapper<DAVA::Vector3> propForce;
    forcePowerTimeLine->GetValues(propForce.GetPropsPtr());

    CommandUpdateParticleDragForce::ForceParams params;
    params.isActive = isActive->isChecked();
    params.forceName = forceNameEdit->text().toStdString();
    params.shape = shape;
    params.timingType = timingType;
    params.boxSize = boxSize->GetValue();
    params.forcePower = forcePower->GetValue();
    params.useInfinityRange = infinityRange->isChecked();
    params.radius = radiusSpin->value();
    params.forcePowerLine = propForce.GetPropLine();

    boxSize->setVisible(shape == DAVA::ParticleDragForce::eShape::BOX && !params.useInfinityRange);
    radiusWidget->setVisible(shape == DAVA::ParticleDragForce::eShape::SPHERE && !params.useInfinityRange);
    shapeComboBox->setVisible(!params.useInfinityRange);
    shapeLabel->setVisible(!params.useInfinityRange);
    shapeSeparator->setVisible(!params.useInfinityRange);
    forcePower->setVisible(timingType == DAVA::ParticleDragForce::eTimingType::CONSTANT);
    forcePowerTimeLine->setVisible(timingType != DAVA::ParticleDragForce::eTimingType::CONSTANT);
    forcePowerLabel->setVisible(timingType != DAVA::ParticleDragForce::eTimingType::CONSTANT);

    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);

    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        if (LayerDragForceWidgetDetail::shapeMap[i].shape == shape)
        {
            shapeComboBox->setCurrentIndex(i);
            break;
        }

    DAVA::int32 timingTypes = sizeof(LayerDragForceWidgetDetail::timingMap) / sizeof(*LayerDragForceWidgetDetail::timingMap);
    for (DAVA::int32 i = 0; i < timingTypes; ++i)
        if (LayerDragForceWidgetDetail::timingMap[i].timingType == timingType)
        {
            timingTypeComboBox->setCurrentIndex(i);
            break;
        }

    std::unique_ptr<CommandUpdateParticleDragForce> updateDragForceCmd(new CommandUpdateParticleDragForce(layer, forceIndex, std::move(params)));

    SceneEditor2* activeScene = GetActiveScene();
    DVASSERT(activeScene != nullptr);
    activeScene->Exec(std::move(updateDragForceCmd));
    activeScene->MarkAsChanged();
    //Init(activeScene, layer, forceIndex, false);
    emit ValueChanged();
}
