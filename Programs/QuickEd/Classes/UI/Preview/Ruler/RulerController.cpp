#include "UI/Preview/Ruler/RulerController.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>

#include <cmath>

RulerController::RulerController(DAVA::TArc::ContextAccessor* accessor_, QObject* parent)
    : QObject(parent)
    , screenScale(0.0f)
    , accessor(accessor_)
{
    SetupInitialRulerSettings(horisontalRulerSettings);
    SetupInitialRulerSettings(verticalRulerSettings);

    InitFieldBinder();
}

RulerController::~RulerController() = default;

void RulerController::SetupInitialRulerSettings(RulerSettings& settings)
{
    static const int defaultSmallTicksDelta = 10;
    static const int defaultBigTicksDelta = 50;

    settings.smallTicksDelta = defaultSmallTicksDelta;
    settings.bigTicksDelta = defaultBigTicksDelta;
    settings.startPos = 0;
    settings.zoomLevel = 1.0f;
}

void RulerController::UpdateRulerMarkers(QPoint curMousePos)
{
    emit HorisontalRulerMarkPositionChanged(curMousePos.x());
    emit VerticalRulerMarkPositionChanged(curMousePos.y());
}

void RulerController::UpdateRulers()
{
    emit HorisontalRulerSettingsChanged(horisontalRulerSettings);
    emit VerticalRulerSettingsChanged(verticalRulerSettings);
}

void RulerController::RecalculateRulerSettings()
{
    static const struct
    {
        float scaleLevel;
        int smallTicksDelta;
        int bigTicksDelta;
    } ticksMap[] =
    {
      { 0.1f, 100, 500 },
      { 0.25f, 64, 320 },
      { 0.5f, 40, 200 },
      { 0.75f, 16, 80 },
      { 1.0f, 10, 50 },
      { 2.0f, 10, 50 },
      { 4.0f, 2, 20 },
      { 8.0f, 1, 10 },
      { 12.0f, 1, 10 },
      { 16.0f, 1, 10 }
    };

    // Look for the closest value.
    int closestValueIndex = 0;
    float closestScaleDistance = std::numeric_limits<float>::max();

    for (int i = 0; i < sizeof(ticksMap) / sizeof(ticksMap[0]); i++)
    {
        float curScaleDistance = std::fabs(ticksMap[i].scaleLevel - screenScale);
        if (curScaleDistance < closestScaleDistance)
        {
            closestScaleDistance = curScaleDistance;
            closestValueIndex = i;
        }
    }

    horisontalRulerSettings.smallTicksDelta = ticksMap[closestValueIndex].smallTicksDelta;
    horisontalRulerSettings.bigTicksDelta = ticksMap[closestValueIndex].bigTicksDelta;
    verticalRulerSettings.smallTicksDelta = ticksMap[closestValueIndex].smallTicksDelta;
    verticalRulerSettings.bigTicksDelta = ticksMap[closestValueIndex].bigTicksDelta;

    UpdateRulers();
}

void RulerController::InitFieldBinder()
{
    using namespace DAVA;
    using namespace TArc;
    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = FastName(EditorCanvasData::startValuePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &RulerController::OnStartValueChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = FastName(EditorCanvasData::scalePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &RulerController::OnScaleChanged));
    }
}

void RulerController::OnStartValueChanged(const DAVA::Any& startValue)
{
    QPoint pos(0, 0);
    if (startValue.CanGet<DAVA::Vector2>())
    {
        DAVA::Vector2 davaPos = startValue.Get<DAVA::Vector2>();
        pos.setX(davaPos.x);
        pos.setY(davaPos.y);
    }

    if (viewPos != pos)
    {
        viewPos = pos;
        horisontalRulerSettings.startPos = viewPos.x();
        verticalRulerSettings.startPos = viewPos.y();

        UpdateRulers();
    }
}

void RulerController::OnScaleChanged(const DAVA::Any& scaleValue)
{
    screenScale = 1.0f;
    if (scaleValue.CanGet<DAVA::float32>())
    {
        screenScale = scaleValue.Get<DAVA::float32>();
    }

    horisontalRulerSettings.zoomLevel = screenScale;
    verticalRulerSettings.zoomLevel = screenScale;

    RecalculateRulerSettings();
}
