#pragma once

#include "UI/DataBinding/Private/UIDataNode.h"

#include "UI/DataBinding/UIDataSourceComponent.h"
#include "UI/DataBinding/UIDataViewModelComponent.h"
#include "UI/DataBinding/UIDataScopeComponent.h"
#include "UI/DataBinding/UIDataListComponent.h"
#include "UI/DataBinding/UIDataChildFactoryComponent.h"

namespace DAVA
{
class FormulaDataMap;
class FormulaContext;
class UIDataBindingIssueDelegate;
class UIDataBindingDependenciesManager;

class UIDataModel : public UIDataNode
{
public:
    enum Priority
    {
        PRIORITY_ROOT = 0,
        PRIORITY_SOURCE = 1,
        PRIORITY_SCOPE = 2,
        PRIORITY_VIEW_MODEL = 3,
        PRIORITY_FACTORY = 4,
        PRIORITY_LIST = 5,
        PRIORITY_DATA_BINDING = 6,
        PRIORITY_COUNT = 7
    };

    virtual ~UIDataModel();

    static std::unique_ptr<UIDataModel> Create(UIDataSourceComponent* component, bool editorMode);
    static std::unique_ptr<UIDataModel> Create(UIDataScopeComponent* component, bool editorMode);
    static std::unique_ptr<UIDataModel> Create(UIDataViewModelComponent* component, bool editorMode);
    static std::unique_ptr<UIDataModel> Create(UIDataListComponent* component, bool editorMode);
    static std::unique_ptr<UIDataModel> Create(UIDataChildFactoryComponent* component, bool editorMode);

    int32 GetOrder() const;
    int32 GetPriority() const;

    bool IsDirty() const;
    void SetDirty();
    void ResetDirty();

    const std::shared_ptr<FormulaContext>& GetFormulaContext() const;
    virtual const std::shared_ptr<FormulaContext>& GetRootContext() const;

    void MarkAsUnprocessed();
    virtual bool Process(UIDataBindingDependenciesManager* dependenciesManager);

protected:
    bool processed = false;
    UIDataModel(UIComponent* component, Priority priority, bool editorMode);

    std::shared_ptr<FormulaDataMap> LoadDataMap(const FilePath& path);

protected:
    int32 order = 0;
    Priority priority;
    bool dirty = false;
    std::shared_ptr<FormulaContext> context;
};

class UIDataRootModel : public UIDataModel
{
public:
    UIDataRootModel(bool editorMode);
    ~UIDataRootModel() override;

    UIComponent* GetComponent() const override;
    const std::shared_ptr<FormulaContext>& GetRootContext() const override;

    bool Process(UIDataBindingDependenciesManager* dependenciesManager) override;
};

class UIDataSourceModel : public UIDataModel
{
public:
    UIDataSourceModel(UIDataSourceComponent* component, bool editorMode);
    ~UIDataSourceModel() override;
    UIComponent* GetComponent() const override;

    bool Process(UIDataBindingDependenciesManager* dependenciesManager) override;

private:
    UIDataSourceComponent* component = nullptr;
    std::shared_ptr<FormulaDataMap> sourceData;
};

class UIDataViewModel : public UIDataModel
{
public:
    UIDataViewModel(UIDataViewModelComponent* component, bool editorMode);
    ~UIDataViewModel() override;

    UIComponent* GetComponent() const override;
    bool Process(UIDataBindingDependenciesManager* dependenciesManager) override;

private:
    UIDataViewModelComponent* component = nullptr;
    std::shared_ptr<FormulaDataMap> viewData;
};

class UIDataScopeModel : public UIDataModel
{
public:
    UIDataScopeModel(UIDataScopeComponent* component, bool editorMode);
    ~UIDataScopeModel() override;

    UIComponent* GetComponent() const override;
    bool Process(UIDataBindingDependenciesManager* dependenciesManager) override;

private:
    UIDataScopeComponent* component = nullptr;
    std::shared_ptr<FormulaExpression> expression;
};
}
