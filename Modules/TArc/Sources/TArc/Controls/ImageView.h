#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QLabel>
#include <QPixmap>
#include <QPointer>

class QMouseEvent;
namespace DAVA
{
class ImageView final : public ControlProxyImpl<QLabel>
{
public:
    enum class Fields : uint32
    {
        Image, // Image* - pointer to image.
        Size, // int32 - size of preview, default 128
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ImageView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ImageView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& changedFields) override;

    QPointer<QLabel> popupView;
    QPixmap pixmap;
    int32 imageViewSize = 128;
};
} // namespace DAVA
