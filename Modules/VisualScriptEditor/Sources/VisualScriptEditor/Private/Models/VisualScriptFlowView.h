#pragma once

#include <nodes/FlowView>

#include <TArc/Utils/QtDelayedExecutor.h>

namespace QtNodes
{
class FlowScene;
}

namespace DAVA
{
class VisualScriptFlowView : public QtNodes::FlowView
{
public:
    VisualScriptFlowView(QtNodes::FlowScene* scene);

    void ClearSelectedNodes();

protected:
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dropEvent(QDropEvent*) override;

private:
    QtDelayedExecutor menuCaller;
};

} // DAVA
