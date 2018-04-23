#include "TArc/Controls/PropertyPanel/SimpleModifyExtension.h"

namespace DAVA
{
SimpleModifyExtension::SimpleModifyExtension(std::weak_ptr<PropertiesView::Updater> updater)
    : updater(updater)
{
}

void SimpleModifyExtension::BeginBatch(const String& text, uint32 commandCount)
{
}

void SimpleModifyExtension::ProduceCommand(const std::shared_ptr<PropertyNode>& node, const Any& newValue)
{
    ProduceCommand(node->field, newValue);
}

void SimpleModifyExtension::ProduceCommand(const Reflection::Field& object, const Any& newValue)
{
    object.ref.SetValueWithCast(newValue);
    UpdateView();
}

void SimpleModifyExtension::Exec(std::unique_ptr<Command>&& command)
{
    command->Redo();
    UpdateView();
}

void SimpleModifyExtension::EndBatch()
{
}

void SimpleModifyExtension::UpdateView()
{
    executor.DelayedExecute([this]() {
        std::shared_ptr<PropertiesView::Updater> u = updater.lock();
        if (u != nullptr)
        {
            u->update.Emit(PropertiesView::FullUpdate);
        }
    });
}
}
