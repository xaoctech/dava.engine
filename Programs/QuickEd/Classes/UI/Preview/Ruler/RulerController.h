#pragma once

#include "UI/Preview/Ruler/RulerSettings.h"

#include <QObject>
#include <QPoint>

#include <memory>

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

class RulerController : public QObject
{
    Q_OBJECT

public:
    // Construction/destruction.
    RulerController(DAVA::TArc::ContextAccessor* accessor, QObject* parent = nullptr);
    ~RulerController() override;

public slots:
    // Update the ruler markers with the mouse position.
    void UpdateRulerMarkers(QPoint curMousePos);

signals:
    // Horizontal/Vertical ruler settings are changed.
    void HorisontalRulerSettingsChanged(const RulerSettings& settings);
    void VerticalRulerSettingsChanged(const RulerSettings& settings);

    // Horizontal/Vertical mark positions are changed.
    void HorisontalRulerMarkPositionChanged(int position);
    void VerticalRulerMarkPositionChanged(int position);

protected:
    // Update the rulers by sending "settings changed" signal to them.
    void UpdateRulers();

    void SetupInitialRulerSettings(RulerSettings& settings);

    // Recalculate the ruler settings depending on position/zoom level and emit signals.
    void RecalculateRulerSettings();

private:
    void InitFieldBinder();
    void OnStartValueChanged(const DAVA::Any& startValue);
    void OnScaleChanged(const DAVA::Any& scaleValue);

    // Screen view pos and scale.
    QPoint viewPos;
    float screenScale = 1.0f;

    // Ruler settings.
    RulerSettings horisontalRulerSettings;
    RulerSettings verticalRulerSettings;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
