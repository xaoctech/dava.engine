#pragma once

#include "TArc/WindowSubSystem/UI.h"

#include <QtTools/Utils/QtDelayedExecutor.h>
#include <QFrame>

namespace DAVA
{
namespace TArc
{
class OverlayWidget : public QFrame
{
public:
    OverlayWidget(const OverCentralPanelInfo& info, QWidget* content, QWidget* parent);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void UpdateGeometry();

private:
    std::shared_ptr<IGeometryProcessor> geometryProccessor;
    QtDelayedExecutor executor;
    QWidget* content = nullptr;
};
} // namespace TArc
} // namespace DAVA