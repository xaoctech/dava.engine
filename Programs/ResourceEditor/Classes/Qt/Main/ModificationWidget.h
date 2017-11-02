#pragma once

#include <REPlatform/DataNodes/Selectable.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/SceneTypes.h>
#include <QWidget>
#include <QAbstractSpinBox>
#include <QLabel>

namespace DAVA
{
class FieldBinder;
class RECommandNotificationObject;
}

class DAVAFloat32SpinBox;
class ModificationWidget : public QWidget
{
    Q_OBJECT

public:
    enum PivotMode : DAVA::uint32
    {
        PivotAbsolute,
        PivotRelative,
    };

    explicit ModificationWidget(QWidget* parent = nullptr);
    ~ModificationWidget() override;

    void SetPivotMode(PivotMode pivotMode);
    void SetTransformType(DAVA::Selectable::TransformType modifMode);

    void ReloadValues();

private slots:
    void OnSceneActivated(DAVA::SceneEditor2* scene);
    void OnSceneDeactivated(DAVA::SceneEditor2* scene);
    void OnSceneCommand(DAVA::SceneEditor2* scene, const DAVA::RECommandNotificationObject& commandNotification);

    void OnXChanged();
    void OnYChanged();
    void OnZChanged();

private:
    void ApplyValues(DAVA::ST_Axis axis);

    std::unique_ptr<DAVA::FieldBinder> selectionFieldBinder;

    QLabel* xLabel = nullptr;
    QLabel* yLabel = nullptr;
    QLabel* zLabel = nullptr;
    DAVAFloat32SpinBox* xAxisModify = nullptr;
    DAVAFloat32SpinBox* yAxisModify = nullptr;
    DAVAFloat32SpinBox* zAxisModify = nullptr;
    DAVA::SceneEditor2* curScene = nullptr;
    PivotMode pivotMode = PivotMode::PivotAbsolute;
    DAVA::Selectable::TransformType modifMode = DAVA::Selectable::TransformType::Disabled;
    bool groupMode = false;
};

class DAVAFloat32SpinBox : public QAbstractSpinBox
{
    Q_OBJECT

public:
    explicit DAVAFloat32SpinBox(QWidget* parent = nullptr);

    void showButtons(bool show);

    DAVA::float32 value() const;
    void setValue(DAVA::float32 val);
    void stepBy(int steps) override;

signals:
    void valueEdited();

public slots:
    void clear() override;

protected slots:
    void textEditingFinished();

private:
    bool eventFilter(QObject* object, QEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    StepEnabled stepEnabled() const override;

    DAVA::float32 originalValue = 0;

    static const int precision = 3;
    static const DAVA::float32 eps;

    bool hasButtons = true;
};
