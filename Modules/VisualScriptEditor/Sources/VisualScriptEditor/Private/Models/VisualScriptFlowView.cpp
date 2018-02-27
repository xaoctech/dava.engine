#include "VisualScriptEditor/Private/Models/VisualScriptFlowView.h"

#include <nodes/FlowScene>

#include <TArc/Controls/PropertyPanel/PropertyPanelMimeData.h>
#include <TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h> // WARNING: dont use Private API !!!

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace DAVA
{
VisualScriptFlowView::VisualScriptFlowView(QtNodes::FlowScene* scene)
    : QtNodes::FlowView(scene)
{
    setAcceptDrops(true);
}

void VisualScriptFlowView::ClearSelectedNodes()
{
    scene()->clearSelection();
}

void VisualScriptFlowView::dragEnterEvent(QDragEnterEvent* dragEvent)
{
    using namespace DAVA;

    QtNodes::FlowView::dragEnterEvent(dragEvent);

    const PropertyPanelMimeData* propertyData = dynamic_cast<const PropertyPanelMimeData*>(dragEvent->mimeData());
    if (propertyData != nullptr)
    {
        dragEvent->setAccepted(true);
    }
}

void VisualScriptFlowView::dragMoveEvent(QDragMoveEvent* dragEvent)
{
    using namespace DAVA;

    QtNodes::FlowView::dragMoveEvent(dragEvent);

    const PropertyPanelMimeData* propertyData = dynamic_cast<const PropertyPanelMimeData*>(dragEvent->mimeData());
    if (propertyData != nullptr)
    {
        dragEvent->setAccepted(true);
    }
}

void VisualScriptFlowView::dragLeaveEvent(QDragLeaveEvent* dragEvent)
{
    QtNodes::FlowView::dragLeaveEvent(dragEvent);
}

void VisualScriptFlowView::dropEvent(QDropEvent* dropEvent)
{
    using namespace DAVA;

    QtNodes::FlowView::dropEvent(dropEvent);

    const PropertyPanelMimeData* propertyData = dynamic_cast<const PropertyPanelMimeData*>(dropEvent->mimeData());
    if (propertyData != nullptr)
    {
        dropEvent->acceptProposedAction();

        const Vector<Reflection::Field>& items = propertyData->GetPropertyItem();
        if (!items.empty())
        {
            const Reflection::Field& field = items[0];

            QPointF scenePos = mapToScene(dropEvent->pos());
            QPoint globalPos = mapToGlobal(dropEvent->pos());

            String keyStr = field.key.Cast<String>();
            QString filter = QString::fromStdString(keyStr);

            menuCaller.DelayedExecute([this, scenePos, globalPos, filter]() {
                showContextMenu(scenePos, globalPos, filter);
            });

            //break; //TODO now we process only first item
        }
    }
}

} //DAVA
