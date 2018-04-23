#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/TimerUpdater.h"
#include "TArc/Utils/QtDelayedExecutor.h"

namespace DAVA
{
class SimpleModifyExtension : public ModifyExtension
{
public:
    SimpleModifyExtension(std::weak_ptr<PropertiesView::Updater> updater);
    void BeginBatch(const String& text, uint32 commandCount) override;
    void ProduceCommand(const std::shared_ptr<PropertyNode>& node, const Any& newValue) override;
    void ProduceCommand(const Reflection::Field& object, const Any& newValue) override;
    void Exec(std::unique_ptr<Command>&& command) override;
    void EndBatch() override;

private:
    void UpdateView();
    std::weak_ptr<PropertiesView::Updater> updater;
    QtDelayedExecutor executor;
};
}
