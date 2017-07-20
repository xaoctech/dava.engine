#pragma once

#include "UI/Preview/Guides/IRulerListener.h"
#include "UI/Preview/Guides/Guide.h"
#include "UI/Preview/Data/CanvasDataAdapter.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/Any.h>
#include <Base/Introspection.h>
#include <Math/Vector.h>
#include <Math/Color.h>

#include <QWidget>
#include <QMap>
#include <QObject>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

class CanvasData;
class CentralWidgetData;
class DocumentData;

class QWidget;
class QPropertyAnimation;

class GuidesControllerPreferences : public DAVA::InspBase
{
public:
    GuidesControllerPreferences();
    ~GuidesControllerPreferences();

    const DAVA::Color& GetGuidesColor() const;
    void SetGuidesColor(const DAVA::Color& color);

    const DAVA::Color& GetPreviewGuideColor() const;
    void SetPreviewGuideColor(const DAVA::Color& color);

    DAVA::Signal<const DAVA::Color&> previewGuideColorChanged;
    DAVA::Signal<const DAVA::Color&> guidesColorChanged;

private:
    DAVA::Color guidesColor;
    DAVA::Color previewGuideColor;

public:
    INTROSPECTION(GuidesControllerPreferences,
                  PROPERTY("guideColor", "User graphic/guide color", GetGuidesColor, SetGuidesColor, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("previewGuideColor", "User graphic/preview guide color", GetPreviewGuideColor, SetPreviewGuideColor, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};

//this class realize Behavior pattern to have different behaviors for vertical and for horizontal guides
class GuidesController : public QObject, public IRulerListener, DAVA::TArc::DataListener
{
    Q_OBJECT

public:
    GuidesController(DAVA::Vector2::eAxis orientation, DAVA::TArc::ContextAccessor* accessor, QWidget* container);

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void BindFields();
    void OnCanvasParametersChanged(const DAVA::Any&);

    //IRulerListener
    void OnMousePress(DAVA::float32 position) override;
    void OnMouseMove(DAVA::float32 position) override;
    void OnMouseRelease(DAVA::float32 position) override;
    void OnMouseLeave() override;
    QList<QAction*> GetActions(DAVA::float32 position, QObject* parent) override;

    //cursor and preview guide states
    //NO_DISPLAY: normal cursor and no preview guide
    //DISPLAY_PREVIEW: normal cursor and preview guide is visible
    //DISPLAY_DRAG: drag cursor and no preview guide
    //DISPLAY_REMOVE: cursor changed to RemoveCursor and no preview guide
    enum eDisplayState
    {
        NO_DISPLAY,
        DISPLAY_PREVIEW,
        DISPLAY_DRAG,
        DISPLAY_REMOVE
    };
    eDisplayState displayState = NO_DISPLAY;
    void SetDisplayState(eDisplayState state);

    //controller state. Used only to wrap valuePtr pointer
    //NO_DRAG: do nothing
    //DRAG: store copy of current guides and pointer to a modified guide
    enum eDragState
    {
        NO_DRAG,
        DRAG
    };
    eDragState dragState = NO_DRAG;
    void EnableDrag(DAVA::float32 position);
    void DisableDrag();

    void OnRootControlsChanged(const DAVA::Any& rootControls);

    void SyncGuidesWithValues();

    //returns closest value to given position
    //store current values in cachedValues variable
    //returns cachedValues.end() if closest value is too far or if no values available
    PackageNode::AxisGuides::iterator GetNearestValuePtr(DAVA::float32 position);

    bool IsEnabled() const;
    PackageNode::AxisGuides GetValues() const;
    void SetValues(const PackageNode::AxisGuides& values);

    void CreatePreviewGuide();
    void SetupPreviewGuide(DAVA::float32 position);
    void DragGuide(DAVA::float32 position);

    void RemoveGuide(DAVA::float32 value);
    void RemoveAllGuides();

    bool IsGuidesEnabled() const;
    void SetGuidesEnabled(bool enabled);

    void OnGuidesColorChanged(const DAVA::Color& color);
    void OnPreviewGuideColorChanged(const DAVA::Color& color);

    Guide CreateGuide(const DAVA::Color& color) const;
    void SetGuideColor(QWidget* guide, const DAVA::Color& color) const;

    void RemoveLastGuideWidget();

    void ResizeGuide(Guide& guide) const;
    void MoveGuide(DAVA::float32 value, Guide& guide) const;

    CentralWidgetData* GetCentralWidgetData() const;
    DocumentData* GetDocumentData() const;

    DAVA::float32 PositionToValue(DAVA::float32 position) const;
    DAVA::float32 ValueToPosition(DAVA::float32 value) const;

    DAVA::Vector2::eAxis orientation = DAVA::Vector2::AXIS_X;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::DataWrapper documentDataWrapper;
    DAVA::TArc::DataWrapper preferencesDataWrapper;

    //use it only for drag-n-drop
    PackageNode::AxisGuides cachedValues;

    //parent widget of guides
    QWidget* container = nullptr;

    //pointer to currentGuide to modify it value on drag
    PackageNode::AxisGuides::iterator valuePtr;

    //semi-transparent preview guide
    Guide previewGuide;
    QList<Guide> guides;

    GuidesControllerPreferences preferences;

    CanvasDataAdapter canvasDataAdapter;
    DAVA::TArc::DataWrapper canvasDataAdapterWrapper;
};
