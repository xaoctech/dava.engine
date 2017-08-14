#include "Classes/Qt/DockParticleEditor/LayerDragForceWidget.h"

#include "Classes/Qt/DockParticleEditor/ParticleVector3Widget.h"

LayerDragForceWidget::LayerDragForceWidget(QWidget* parent /* = nullptr */)
    : BaseParticleEditorContentWidget(parent)
{
    mainLayout = new QVBoxLayout();
    
    setLayout(mainLayout);
    position = new ParticleVector3Widget("Position", Vector3::Zero);
    connect(position, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));

    rotation = new ParticleVector3Widget("Rotation", Vector3::Zero);
    connect(rotation, SIGNAL(valueChanged()), this, SLOT(OnValueChanged()));

    mainLayout->addWidget(position);
    mainLayout->addWidget(rotation);

    infinityRange = new QCheckBox("Use infinity range");
    mainLayout->addWidget(infinityRange);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    blockSignals = false;
}

void LayerDragForceWidget::Init(SceneEditor2* scene, DAVA::ParticleLayer* layer_, DAVA::uint32 forceIndex_, bool updateMinimized)
{
    if (!layer_ || layer_->GetDragForces().size() <= forceIndex)
        return;

    layer = layer_;
    forceIndex = forceIndex_;
    blockSignals = true;

    ParticleDragForce* currForce = layer->GetDragForces()[forceIndex];


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

    std::unique_ptr<CommandUpdateParticleDragForce> updateDragForceCmd(new CommandUpdateParticleDragForce(layer, forceIndex));
    updateDragForceCmd->Init(position->GetValue(), rotation->GetValue(), infinityRange->isChecked());

    SceneEditor2* activeScene = GetActiveScene();
    DVASSERT(activeScene != nullptr);
    activeScene->Exec(std::move(updateDragForceCmd));
    activeScene->MarkAsChanged();
    Init(activeScene, layer, forceIndex, false);
    emit ValueChanged();
}
