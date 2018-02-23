#pragma once

#include <nodes/DataModelRegistry>

#include <TArc/Qt/QtString.h>

#include <Base/Any.h>
#include <Functional/Function.h>
#include <Reflection/Reflection.h>

#include <memory>
#include <map>
#include <vector>

namespace DAVA
{
class ContextAccessor;
class UI;
class VisualScriptNode;
class VisualScript;

struct RegistryItemDescriptorData
{
    Function<VisualScriptNode*()> creator;
};

class VisualScriptRegistryModel : public QtNodes::DataModelRegistry
{
public:
    VisualScriptRegistryModel(ContextAccessor* accessor, UI* ui, VisualScript* script);
    ~VisualScriptRegistryModel() override;

    void SetDataContainers(const Vector<Reflection>& dataContainer);

    void BuildStaticModel();

    const std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& GetStaticModels() const override;
    const std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& GetDynamicModels() override;
    std::unique_ptr<QtNodes::NodeDataModel> Create(const QtNodes::RegistryItemDescriptor* descriptor) override;

private:
    void ReleaseReflectedItems(std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& reflectedItems);

    RegistryItemDescriptorData* CreateStaticItem(const QString& category, const QString& name);
    RegistryItemDescriptorData* CreateDynamicItem(const QString& category, const QString& name);
    RegistryItemDescriptorData* CreateItemInternal(const QString& category, const QString& name, bool staticItem);

    void RegisterEvents();
    void RegisterControllers();
    void RegisterReflectedMethods();
    void RegisterReflectedData();
    void RegisterSubscripts();
    void RegisterSubscriptsAction();

    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;
    VisualScript* script = nullptr;

    Vector<Reflection> dataContainer;
    std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>> staticReflectedItems;
    std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>> dynamicReflectedItems;
};

} // DAVA
