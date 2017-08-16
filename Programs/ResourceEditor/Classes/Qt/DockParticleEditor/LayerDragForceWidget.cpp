#include "Classes/Qt/DockParticleEditor/LayerDragForceWidget.h"

#include "Classes/Qt/DockParticleEditor/ParticleVector3Widget.h"
#include "Classes/Qt/DockParticleEditor/WheellIgnorantComboBox.h"

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
}

LayerDragForceWidget::LayerDragForceWidget(QWidget* parent /* = nullptr */)
    : BaseParticleEditorContentWidget(parent)
{
    mainLayout = new QVBoxLayout();
    
    setLayout(mainLayout);
    shapeComboBox = new WheellIgnorantComboBox();
    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);
    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        shapeComboBox->addItem(LayerDragForceWidgetDetail::shapeMap[i].shapeName);

    connect(shapeComboBox, SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(OnValueChanged()));
    mainLayout->addWidget(shapeComboBox);

    boxSize = new ParticleVector3Widget("Box size", Vector3::Zero);
    connect(boxSize, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));

    forcePower = new ParticleVector3Widget("Force power", Vector3::Zero);
    connect(forcePower, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));

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

    mainLayout->addWidget(boxSize);
    mainLayout->addWidget(forcePower);

    infinityRange = new QCheckBox("Use infinity range");
    connect(infinityRange, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));
    mainLayout->addWidget(infinityRange);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    blockSignals = false;
}

void LayerDragForceWidget::Init(SceneEditor2* scene, DAVA::ParticleLayer* layer_, DAVA::uint32 forceIndex_, bool updateMinimized)
{
    if (!layer_ || layer_->GetDragForces().size() <= forceIndex_ || blockSignals)
        return;

    layer = layer_;
    forceIndex = forceIndex_;
    blockSignals = true;

    ParticleDragForce* currForce = layer->GetDragForces()[forceIndex];
    infinityRange->setChecked(currForce->infinityRange);
    boxSize->SetValue(currForce->boxSize);
    forcePower->SetValue(currForce->forcePower);
    radiusSpin->setValue(currForce->radius);

    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);
    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        if (LayerDragForceWidgetDetail::shapeMap[i].shape == currForce->shape)
            shapeComboBox->setCurrentIndex(i);

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
}

void LayerDragForceWidget::RestoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;
}

void LayerDragForceWidget::OnValueChanged()
{
    if (blockSignals)
        return;

    DAVA::ParticleDragForce::eShape shape = LayerDragForceWidgetDetail::shapeMap[shapeComboBox->currentIndex()].shape;
    CommandUpdateParticleDragForce::ForceParams params;
    params.shape = shape;
    params.boxSize = boxSize->GetValue();
    params.forcePower = forcePower->GetValue();
    params.useInfinityRange = infinityRange->isChecked();
    params.radius = radiusSpin->value();

    boxSize->setVisible(shape == ParticleDragForce::eShape::BOX);
    radiusWidget->setVisible(shape == ParticleDragForce::eShape::SPHERE);
    DAVA::int32 shapeTypes = sizeof(LayerDragForceWidgetDetail::shapeMap) / sizeof(*LayerDragForceWidgetDetail::shapeMap);

    for (DAVA::int32 i = 0; i < shapeTypes; ++i)
        if (LayerDragForceWidgetDetail::shapeMap[i].shape == shape)
            shapeComboBox->setCurrentIndex(i);

    std::unique_ptr<CommandUpdateParticleDragForce> updateDragForceCmd(new CommandUpdateParticleDragForce(layer, forceIndex, std::move(params)));

    SceneEditor2* activeScene = GetActiveScene();
    DVASSERT(activeScene != nullptr);
    activeScene->Exec(std::move(updateDragForceCmd));
    activeScene->MarkAsChanged();
    //Init(activeScene, layer, forceIndex, false);
    emit ValueChanged();
}
