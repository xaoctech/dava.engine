#pragma once

#include <DAVAEngine.h>

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

class ParticleVector3Widget;
class TimeLineWidget;
class QVBoxLayout;
class QCheckBox;

class LayerDragForceWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit LayerDragForceWidget(QWidget* parent = nullptr);
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
    QVBoxLayout* mainLayout = nullptr;
    DAVA::ParticleLayer* layer = nullptr;
    ParticleVector3Widget* position = nullptr;
    ParticleVector3Widget* rotation = nullptr;
    QCheckBox* infinityRange = nullptr;
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