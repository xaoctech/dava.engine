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
    { DAVA::ParticleDragForce::eType::POINT_GRAVITY, "Point Gravity" },
    { DAVA::ParticleDragForce::eType::BOX_WRAP, "Box Wrap" }
};

template <typename T, typename U, size_t sz>
int ElementToIndex(T elem, const Array<U, sz> map)
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
    BuildShapeSection();
    BuildTimingSection();
    BuilDirectionSection();
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    blockSignals = false;
}

void LayerDragForceWidget::BuildTimingSection()
{
    using namespace LayerDragForceWidgetDetail;

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    QLabel* timingLabel = new QLabel("Timing type:");
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

void LayerDragForceWidget::UpdateVisibility(DAVA::ParticleDragForce::eShape shape, DAVA::ParticleDragForce::eTimingType timingType, bool isInfinityRange)
{
    using Shape = DAVA::ParticleDragForce::eShape;
    using TimingType = DAVA::ParticleDragForce::eTimingType;

    boxSize->setVisible(shape == Shape::BOX && !isInfinityRange);
    radiusWidget->setVisible(shape == Shape::SPHERE && !isInfinityRange);
    shapeComboBox->setVisible(!isInfinityRange);
    shapeLabel->setVisible(!isInfinityRange);
    shapeSeparator->setVisible(!isInfinityRange);

    forcePower->setVisible(timingType == TimingType::CONSTANT);
    forcePowerTimeLine->setVisible(timingType != TimingType::CONSTANT);
    forcePowerLabel->setVisible(timingType != TimingType::CONSTANT);
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

    ParticleDragForce* currForce = layer->GetDragForces()[forceIndex];
    infinityRange->setChecked(currForce->isInfinityRange);
    isActive->setChecked(currForce->isActive);
    boxSize->SetValue(currForce->boxSize);
    forcePower->SetValue(currForce->forcePower);
    radiusSpin->setValue(currForce->radius);
    forceNameEdit->setText(QString::fromStdString(currForce->forceName));
    forceTypeLabel->setText(forceTypes[currForce->type]);
    direction->SetValue(currForce->direction);

    UpdateVisibility(currForce->shape, currForce->timingType, currForce->isInfinityRange);

    static const Vector<QColor> colors{ Qt::red, Qt::darkGreen, Qt::blue };
    static const Vector<QString> legends{ "Force x", "Force y", "Force z" };

    forcePowerTimeLine->Init(0, 1, updateMinimized, true, false);
    forcePowerTimeLine->AddLines(LineWrapper(LineHelper::GetValueLine(currForce->forcePowerLine)).GetProps(), colors, legends);
    forcePowerTimeLine->EnableLock(true);

    shapeComboBox->setCurrentIndex(ElementToIndex(currForce->shape, shapeMap));
    timingTypeComboBox->setCurrentIndex(ElementToIndex(currForce->timingType, timingMap));

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
    using namespace DAVA;
    using namespace LayerDragForceWidgetDetail;
    using Shape = ParticleDragForce::eShape;
    using TimingType = ParticleDragForce::eTimingType;

    if (blockSignals)
        return;

    Shape shape = shapeMap[shapeComboBox->currentIndex()].elemType;
    TimingType timingType = timingMap[timingTypeComboBox->currentIndex()].elemType;

    PropLineWrapper<Vector3> propForce;
    forcePowerTimeLine->GetValues(propForce.GetPropsPtr());

    CommandUpdateParticleDragForce::ForceParams params;
    params.isActive = isActive->isChecked();
    params.forceName = forceNameEdit->text().toStdString();
    params.shape = shape;
    params.timingType = timingType;
    params.boxSize = boxSize->GetValue();
    params.forcePower = forcePower->GetValue();
    params.direction = direction->GetValue();
    params.useInfinityRange = infinityRange->isChecked();
    params.radius = radiusSpin->value();
    params.forcePowerLine = propForce.GetPropLine();

    UpdateVisibility(shape, timingType, params.useInfinityRange);

    shapeComboBox->setCurrentIndex(ElementToIndex(shape, shapeMap));
    timingTypeComboBox->setCurrentIndex(ElementToIndex(timingType, timingMap));

    std::unique_ptr<CommandUpdateParticleDragForce> updateDragForceCmd(new CommandUpdateParticleDragForce(layer, forceIndex, std::move(params)));

    SceneEditor2* activeScene = GetActiveScene();
    DVASSERT(activeScene != nullptr);
    activeScene->Exec(std::move(updateDragForceCmd));
    activeScene->MarkAsChanged();

    emit ValueChanged();
}
