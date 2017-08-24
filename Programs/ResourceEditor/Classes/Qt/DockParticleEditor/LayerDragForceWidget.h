#pragma once

#include <DAVAEngine.h>

#include <QWidget>

#include <Particles/ParticleDragForce.h>
#include "BaseParticleEditorContentWidget.h"

class ParticleVector3Widget;
class TimeLineWidget;
class QVBoxLayout;
class QCheckBox;
class WheellIgnorantComboBox;
class EventFilterDoubleSpinBox;
class QLabel;
class QLineEdit;
class QFrame;

class LayerDragForceWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit LayerDragForceWidget(QWidget* parent = nullptr);

    void BuildTimingSection();

    ~LayerDragForceWidget() = default;

    void Init(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::uint32 forceIndex, bool updateMinimized);

    DAVA::ParticleLayer* GetLayer() const;
    DAVA::int32 GetForceIndex() const;

    void Update();

    void StoreVisualState(DAVA::KeyedArchive* visualStateProps) override;
    void RestoreVisualState(DAVA::KeyedArchive* visualStateProps) override;

signals:
    void ValueChanged();

protected slots:
    void OnValueChanged();

private:
    void BuildShapeSection();
    void BuildCommonSection();
    void UpdateVisibility(DAVA::ParticleDragForce::eShape shape, DAVA::ParticleDragForce::eTimingType timingType, bool isInfinityRange);

    QVBoxLayout* mainLayout = nullptr;
    QLineEdit* forceNameEdit = nullptr;
    QCheckBox* isActive = nullptr;
    QCheckBox* infinityRange = nullptr;

    QFrame* shapeSeparator = nullptr;
    QLabel* shapeLabel = nullptr;
    WheellIgnorantComboBox* shapeComboBox = nullptr;
    ParticleVector3Widget* boxSize = nullptr;
    QWidget* radiusWidget = nullptr;
    EventFilterDoubleSpinBox* radiusSpin = nullptr;

    WheellIgnorantComboBox* timingTypeComboBox = nullptr;
    QLabel* forcePowerLabel = nullptr;
    ParticleVector3Widget* forcePower = nullptr;
    TimeLineWidget* forcePowerTimeLine = nullptr;

    DAVA::ParticleLayer* layer = nullptr;
    DAVA::int32 forceIndex = -1;

    bool blockSignals = false;
};

inline DAVA::ParticleLayer* LayerDragForceWidget::GetLayer() const
{
    return layer;
}

inline DAVA::int32 LayerDragForceWidget::GetForceIndex() const
{
    return forceIndex;
}