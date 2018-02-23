#pragma once

#include <nodes/FlowScene>
#include <memory>

namespace QtNodes
{
class DataModelRegistry;
} //QtNodes

namespace DAVA
{
class ContextAccessor;
class UI;
class VisualScript;
class VisualScriptFlowScene : public QtNodes::FlowScene
{
public:
    VisualScriptFlowScene(ContextAccessor* accessor, UI* ui, VisualScript* script, std::shared_ptr<QtNodes::DataModelRegistry> registry);
    ~VisualScriptFlowScene() override;

    void SaveRuntime() const override;
    void LoadRuntime() override;

private:
    void NodeWillBeRemoved(QtNodes::NodeDataModel* model) override;

    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;
    VisualScript* script = nullptr;
};

} // DAVA
