#include "UI/Preview/Guides/GuidesController.h"
#include "UI/Preview/Guides/GuideLabel.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/EditorCanvasData.h"
#include "Modules/DocumentsModule/CentralWidgetData.h"

#include "Modules/PreferencesModule/PreferencesData.h"

#include "QECommands/SetGuidesCommand.h"

#include <TArc/Core/FieldBinder.h>

#include <QtTools/Updaters/LazyUpdater.h>

#include <Reflection/ReflectedTypeDB.h>
#include <Logger/Logger.h>
#include <Preferences/PreferencesStorage.h>
#include <Preferences/PreferencesRegistrator.h>

GuidesController::GuidesController(DAVA::Vector2::eAxis orientation_, DAVA::TArc::ContextAccessor* accessor_, QWidget* container_)
    : orientation(orientation_)
    , accessor(accessor_)
    , fieldBinder(new DAVA::TArc::FieldBinder(accessor))
    , container(container_)
{
    updater.SetCallback(DAVA::MakeFunction(this, &GuidesController::SyncGuidesWithValues));

    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    preferencesDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreferencesData>());

    BindFields();

    preferences.guidesColorChanged.Connect(this, &GuidesController::OnGuidesColorChanged);
    preferences.previewGuideColorChanged.Connect(this, &GuidesController::OnPreviewGuideColorChanged);

    CreatePreviewGuide();
}

void GuidesController::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = DocumentData::displayedRootControlsPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnRootControlsChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = DocumentData::guidesPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<PreferencesData>();
        fieldDescr.fieldName = PreferencesData::guidesEnabledPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = EditorCanvasData::startValuePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = EditorCanvasData::lastValuePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = EditorCanvasData::scalePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<CentralWidgetData>();
        fieldDescr.fieldName = CentralWidgetData::guidesPosPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<CentralWidgetData>();
        fieldDescr.fieldName = CentralWidgetData::guidesSizePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<CentralWidgetData>();
        fieldDescr.fieldName = CentralWidgetData::guidesRelativePosPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnVisualPropertiesChanged));
    }
}

void GuidesController::OnVisualPropertiesChanged(const DAVA::Any&)
{
    updater.MarkDirty();

    SetDisplayState(NO_DISPLAY);
    DisableDrag();

}

void GuidesController::OnMousePress(DAVA::float32 position)
{
    if (IsEnabled() == false || displayState == NO_DISPLAY)
    {
        return;
    }

    //if guides was disabled - enable guides and create new one
    if (IsGuidesEnabled() == false)
    {
        SetGuidesEnabled(true);
        SetDisplayState(DISPLAY_PREVIEW);
    }

    if (displayState == DISPLAY_PREVIEW)
    {
        PackageNode::AxisGuides values = GetValues();

        values.push_back(PositionToValue(position));

        SetValues(values);

        SyncGuidesWithValues();

        previewGuide.Raise();

        SetDisplayState(DISPLAY_DRAG);
    }

    EnableDrag(position);
}

void GuidesController::OnMouseMove(DAVA::float32 position)
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }

    if (dragState == DRAG)
    {
        DragGuide(position);
    }
    else
    {
        PackageNode::AxisGuides::iterator valuePtr = GetNearestValuePtr(position);
        if (valuePtr != cachedValues.end())
        {
            SetDisplayState(DISPLAY_DRAG);
        }
        else
        {
            SetDisplayState(DISPLAY_PREVIEW);
            SetupPreviewGuide(position);
        }
    }
}

void GuidesController::OnMouseRelease(DAVA::float32 position)
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }

    //copy  pointer to dragged item to remove it if we drag outside of screen
    DisableDrag();

    if (displayState == DISPLAY_REMOVE)
    {
        RemoveGuide(GetValues().back());
        SetDisplayState(NO_DISPLAY);
    }
}

void GuidesController::OnMouseLeave()
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }
    SetDisplayState(NO_DISPLAY);
}

QList<QAction*> GuidesController::GetActions(DAVA::float32 position, QObject* parent)
{
    //we can call context menu when we dragging guide
    DisableDrag();

    QList<QAction*> actions;

    QAction* guidesEnabledAction = new QAction("Show Alignment Guides", parent);
    guidesEnabledAction->setCheckable(true);
    guidesEnabledAction->setChecked(IsGuidesEnabled());
    connect(guidesEnabledAction, &QAction::toggled, this, &GuidesController::SetGuidesEnabled);

    actions << guidesEnabledAction;

    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return actions;
    }

    PackageNode::AxisGuides::iterator valuePtr = GetNearestValuePtr(position);
    if (valuePtr != cachedValues.end())
    {
        QAction* removeGuideAction = new QAction("Remove Guide", parent);
        connect(removeGuideAction, &QAction::triggered, std::bind(&GuidesController::RemoveGuide, this, *valuePtr));
        actions << removeGuideAction;
    }

    QAction* removeAllGuidesAction = new QAction(QString("Remove All %1 Guides").arg(orientation == DAVA::Vector2::AXIS_X ? "Vertical" : "Horizontal"), parent);
    connect(removeAllGuidesAction, &QAction::triggered, this, &GuidesController::RemoveAllGuides);
    if (GetValues().empty())
    {
        removeAllGuidesAction->setEnabled(false);
    }
    actions << removeAllGuidesAction;

    return actions;
}

void GuidesController::SetDisplayState(eDisplayState state)
{
    if (displayState == state)
    {
        return;
    }

    //leave current state
    switch (displayState)
    {
    case DISPLAY_PREVIEW:
        previewGuide.Hide();
        break;
    case DISPLAY_DRAG:
    case DISPLAY_REMOVE:
        container->unsetCursor();
        break;
    default:
        break;
    }

    displayState = state;

    switch (displayState)
    {
    case DISPLAY_PREVIEW:
        previewGuide.Show();
        previewGuide.Raise();
        break;
    case DISPLAY_DRAG:
        container->setCursor(orientation == DAVA::Vector2::AXIS_X ? Qt::SplitHCursor : Qt::SplitVCursor);
        break;
    case DISPLAY_REMOVE:
        container->setCursor(QCursor(QPixmap(":/Cursors/trashCursor.png")));
    default:
        break;
    }
}

void GuidesController::EnableDrag(DAVA::float32 position)
{
    DVASSERT(IsEnabled());
    if (dragState == DRAG)
    {
        return;
    }

    valuePtr = GetNearestValuePtr(position);

    DVASSERT(valuePtr != cachedValues.end());
    if (valuePtr != cachedValues.end())
    {
        dragState = DRAG;

        DAVA::TArc::DataContext* active = accessor->GetActiveContext();
        DocumentData* data = active->GetData<DocumentData>();
        data->BeginBatch("Dragging guide");

        //move dragged guide to the end of list to display it under all other guides
        cachedValues.splice(cachedValues.end(), cachedValues, valuePtr);
    }
}

void GuidesController::DisableDrag()
{
    if (dragState != DRAG)
    {
        return;
    }

    DAVA::TArc::DataContext* active = accessor->GetActiveContext();
    DocumentData* data = active->GetData<DocumentData>();
    data->EndBatch();

    dragState = NO_DRAG;
}

void GuidesController::OnRootControlsChanged(const DAVA::Any& rootControls)
{
    //this is not good situation, but we can reload or close document by shortcut while we dragging guide
    SetDisplayState(NO_DISPLAY);
    DisableDrag();
    SyncGuidesWithValues();
}

void GuidesController::SyncGuidesWithValues()
{
    using namespace DAVA;

    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        while (guides.empty() == false)
        {
            RemoveLastGuideWidget();
        }
        return;
    }

    EditorCanvasData* canvasData = GetCanvasData();
    float32 scale = canvasData->GetScale();
    float32 minValue = canvasData->GetStartValue()[orientation] / scale;
    float32 maxValue = canvasData->GetLastValue()[orientation] / scale;

    PackageNode::AxisGuides values = GetValues();
    PackageNode::AxisGuides visibleValues;
    for (float32 value : values)
    {
        if (value > minValue && value < maxValue)
        {
            visibleValues.push_back(value);
        }
    }

    std::size_t size = visibleValues.size();

    while (guides.size() > size)
    {
        RemoveLastGuideWidget();
    }
    while (guides.size() < size)
    {
        Guide guide = CreateGuide(preferences.GetGuidesColor());
        guides.append(guide);
    }

    DVASSERT(size == guides.size());
    int index = 0;
    for (float32 value : visibleValues)
    {
        Guide& guide = guides[index++];
        ResizeGuide(guide);
        guide.text->SetValue(value);
        MoveGuide(value, guide);

        delayedExecutor.DelayedExecute([&guide]() {
            guide.Show();
            guide.Raise();
        });
    }
}

PackageNode::AxisGuides::iterator GuidesController::GetNearestValuePtr(DAVA::float32 position)
{
    using namespace DAVA;

    float32 range = 1;

    float32 scale = GetCanvasData()->GetScale();
    if (scale < 1.0f)
    {
        range = 3 / scale;
    }

    cachedValues = GetValues();

    if (cachedValues.empty())
    {
        return cachedValues.end();
    }

    float32 value = PositionToValue(position);

    PackageNode::AxisGuides::iterator iter = std::min_element(cachedValues.begin(), cachedValues.end(), [value](float32 left, float32 right)
                                                              {
                                                                  return std::abs(left - value) < std::abs(right - value);
                                                              });
    if (std::fabs(*iter - value) >= range)
    {
        return cachedValues.end();
    }
    return iter;
}

bool GuidesController::IsEnabled() const
{
    if (documentDataWrapper.HasData())
    {
        SortedControlNodeSet displayedRootControls = documentDataWrapper.GetFieldValue(DocumentData::displayedRootControlsPropertyName).Cast<SortedControlNodeSet>(SortedControlNodeSet());
        if (displayedRootControls.size() == 1)
        {
            return true;
        }
    }
    return false;
}

PackageNode::AxisGuides GuidesController::GetValues() const
{
    PackageNode::Guides guides = documentDataWrapper.GetFieldValue(DocumentData::guidesPropertyName).Cast<PackageNode::Guides>(PackageNode::Guides());
    return guides[orientation];
}

void GuidesController::SetValues(const PackageNode::AxisGuides& values)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* data = activeContext->GetData<DocumentData>();
    const SortedControlNodeSet& rootControls = data->GetDisplayedRootControls();
    Q_ASSERT(rootControls.size() == 1);
    DAVA::String name = (*rootControls.begin())->GetName();

    data->ExecCommand<SetGuidesCommand>(name, orientation, values);
}

void GuidesController::CreatePreviewGuide()
{
    DVASSERT(previewGuide.line == nullptr && previewGuide.text == nullptr);
    previewGuide = CreateGuide(preferences.GetPreviewGuideColor());
    previewGuide.Hide();
}

void GuidesController::SetupPreviewGuide(DAVA::float32 position)
{
    DAVA::float32 value = PositionToValue(position);
    ResizeGuide(previewGuide);
    previewGuide.text->SetValue(value);
    MoveGuide(value, previewGuide);
}

Guide GuidesController::CreateGuide(const DAVA::Color& color) const
{
    Guide guide;
    guide.line = new QWidget(container);
    guide.text = new GuideLabel(orientation, container);

    guide.line->setAttribute(Qt::WA_TransparentForMouseEvents);
    guide.text->setAttribute(Qt::WA_TransparentForMouseEvents);
    SetGuideColor(guide.line, color);
    return guide;
}

void GuidesController::DragGuide(DAVA::float32 position)
{
    using namespace DAVA;

    DVASSERT(IsEnabled());
    DVASSERT(dragState == DRAG);

    float32 value = PositionToValue(position);

    EditorCanvasData* canvasData = GetCanvasData();
    float32 scale = canvasData->GetScale();
    float32 minValue = canvasData->GetStartValue()[orientation] / scale;
    float32 maxValue = canvasData->GetLastValue()[orientation] / scale;
    SetDisplayState((value < minValue || value > maxValue) ? DISPLAY_REMOVE : DISPLAY_DRAG);

    *valuePtr = PositionToValue(position);
    SetValues(cachedValues);

    SyncGuidesWithValues();
}

void GuidesController::RemoveGuide(DAVA::float32 value)
{
    cachedValues = GetValues();
    cachedValues.remove(value);
    SetValues(cachedValues);
}

void GuidesController::RemoveAllGuides()
{
    SetValues(PackageNode::AxisGuides());
}

bool GuidesController::IsGuidesEnabled() const
{
    return preferencesDataWrapper.GetFieldValue(PreferencesData::guidesEnabledPropertyName).Cast<bool>(true);
}

void GuidesController::SetGuidesEnabled(bool enabled)
{
    preferencesDataWrapper.SetFieldValue(PreferencesData::guidesEnabledPropertyName, enabled);
    if (enabled == false)
    {
        DisableDrag();
        SetDisplayState(NO_DISPLAY);
    }
}

void GuidesController::OnGuidesColorChanged(const DAVA::Color& color)
{
    for (Guide& guide : guides)
    {
        SetGuideColor(guide.line, color);
    }
}

void GuidesController::OnPreviewGuideColorChanged(const DAVA::Color& color)
{
    SetGuideColor(previewGuide.line, color);
}

void GuidesController::SetGuideColor(QWidget* guide, const DAVA::Color& color) const
{
    QString colorString = QString("rgba(%1, %2, %3, %4)")
                          .arg(color.r * 255.0f)
                          .arg(color.g * 255.0f)
                          .arg(color.b * 255.0f)
                          .arg(color.a * 255.0f);

    guide->setStyleSheet(QString("QWidget { background-color: %1; }").arg(colorString));
}

void GuidesController::RemoveLastGuideWidget()
{
    Guide& guide = guides.last();
    delete guide.line;
    delete guide.text;
    guides.removeLast();
}

EditorCanvasData* GuidesController::GetCanvasData() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    return activeContext->GetData<EditorCanvasData>();
}

CentralWidgetData* GuidesController::GetCentralWidgetData() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* globalContext = accessor->GetGlobalContext();
    return globalContext->GetData<CentralWidgetData>();
}

DocumentData* GuidesController::GetDocumentData() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    return activeContext->GetData<DocumentData>();
}

DAVA::float32 GuidesController::PositionToValue(DAVA::float32 position) const
{
    using namespace DAVA;
    EditorCanvasData* canvasData = GetCanvasData();
    float32 minValue = canvasData->GetStartValue()[orientation];
    float32 scale = canvasData->GetScale();
    return std::round((minValue + position) / scale);
}

DAVA::float32 GuidesController::ValueToPosition(DAVA::float32 value) const
{
    using namespace DAVA;
    EditorCanvasData* canvasData = GetCanvasData();
    float32 relativePos = GetCentralWidgetData()->GetGuidesRelativePos()[orientation];
    float32 minValue = canvasData->GetStartValue()[orientation];
    float32 scale = canvasData->GetScale();
    return relativePos + value * scale - minValue;
}

void GuidesController::ResizeGuide(Guide& guide) const
{
    using namespace DAVA;
    float32 size = GetCentralWidgetData()->GetGuidesSize()[orientation];
    if (orientation == Vector2::AXIS_X)
    {
        guide.line->resize(1, size);
        guide.text->resize(30, 15);
    }
    else
    {
        guide.line->resize(size, 1);
        guide.text->resize(15, 30);
    }
}

void GuidesController::MoveGuide(DAVA::float32 value, Guide& guide) const
{
    using namespace DAVA;
    float32 startPos = GetCentralWidgetData()->GetGuidesPos()[orientation];
    float32 tmp = ValueToPosition(value);
    int32 position = static_cast<int32>(std::round(ValueToPosition(value)));
    if (orientation == Vector2::AXIS_X)
    {
        guide.line->move(position, startPos);
        guide.text->move(position + 5, startPos);
    }
    else
    {
        guide.line->move(startPos, position);
        guide.text->move(startPos, position - guide.text->height() - 5);
    }
}

GuidesControllerPreferences::GuidesControllerPreferences()
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

GuidesControllerPreferences::~GuidesControllerPreferences()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

const DAVA::Color& GuidesControllerPreferences::GetGuidesColor() const
{
    return guidesColor;
}

void GuidesControllerPreferences::SetGuidesColor(const DAVA::Color& color)
{
    guidesColor = color;
    guidesColorChanged.Emit(color);
}

const DAVA::Color& GuidesControllerPreferences::GetPreviewGuideColor() const
{
    return previewGuideColor;
}

void GuidesControllerPreferences::SetPreviewGuideColor(const DAVA::Color& color)
{
    previewGuideColor = color;
    previewGuideColorChanged.Emit(color);
}

REGISTER_PREFERENCES_ON_START(GuidesControllerPreferences,
                              PREF_ARG("detectGuideDistance", DAVA::float32(3.0f)),
                              PREF_ARG("guideColor", DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f)),
                              PREF_ARG("previewGuideColor", DAVA::Color(1.0f, 0.0f, 0.0f, 0.5f))
                              )
