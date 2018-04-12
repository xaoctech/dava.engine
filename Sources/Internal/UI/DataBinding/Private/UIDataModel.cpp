#include "UI/DataBinding/Private/UIDataModel.h"

#include "UI/DataBinding/Private/UIDataList.h"
#include "UI/DataBinding/Private/UIDataChildFactory.h"

#include "FileSystem/File.h"

#include "UI/DataBinding/Private/UIDataBindingDefaultFunctions.h"

#include "UI/Formula/FormulaContext.h"
#include "UI/Formula/Private/FormulaException.h"
#include "UI/Formula/Private/FormulaParser.h"
#include "UI/Formula/Private/FormulaExecutor.h"
#include "UI/UIControl.h"

#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
////////////////////////////////////////////////////////////////////////////////
// UIDataModel
////////////////////////////////////////////////////////////////////////////////

UIDataModel::UIDataModel(UIComponent* component, Priority priority_, bool editorMode)
    : UIDataNode(editorMode)
    , priority(priority_)
{
    int32 depth = 0;
    if (component != nullptr)
    {
        UIControl* c = component->GetControl();
        while (c)
        {
            c = c->GetParent();
            depth++;
        }
    }

    order = depth * PRIORITY_COUNT + priority;
}

UIDataModel::~UIDataModel()
{
}

std::unique_ptr<UIDataModel> UIDataModel::Create(UIDataSourceComponent* component, bool editorMode)
{
    return std::make_unique<UIDataSourceModel>(component, editorMode);
}

std::unique_ptr<UIDataModel> UIDataModel::Create(UIDataScopeComponent* component, bool editorMode)
{
    return std::make_unique<UIDataScopeModel>(component, editorMode);
}

std::unique_ptr<UIDataModel> UIDataModel::Create(UIDataViewModelComponent* component, bool editorMode)
{
    return std::make_unique<UIDataViewModel>(component, editorMode);
}

std::unique_ptr<UIDataModel> UIDataModel::Create(UIDataListComponent* component, bool editorMode)
{
    return std::make_unique<UIDataList>(component, editorMode);
}

std::unique_ptr<UIDataModel> UIDataModel::Create(UIDataChildFactoryComponent* component, bool editorMode)
{
    return std::make_unique<UIDataChildFactory>(component, editorMode);
}

int32 UIDataModel::GetOrder() const
{
    return order;
}

int32 UIDataModel::GetPriority() const
{
    return priority;
}

bool UIDataModel::IsDirty() const
{
    return dirty;
}

void UIDataModel::SetDirty()
{
    dirty = true;
}

void UIDataModel::ResetDirty()
{
    dirty = false;
}

const std::shared_ptr<FormulaContext>& UIDataModel::GetFormulaContext() const
{
    return context;
}

const std::shared_ptr<FormulaContext>& UIDataModel::GetRootContext() const
{
    DVASSERT(GetParent() != nullptr);
    return GetParent()->GetRootContext();
}

void UIDataModel::MarkAsUnprocessed()
{
    processed = false;
}

bool UIDataModel::Process(UIDataBindingDependenciesManager* dependenciesManager)
{
    if (processed)
    {
        return false;
    }

    processed = true;
    return true;
}

std::shared_ptr<FormulaDataMap> UIDataModel::LoadDataMap(const FilePath& path)
{
    RefPtr<File> file(File::Create(path, File::OPEN | File::READ));
    if (file.Valid())
    {
        String str;
        uint32 fileSize = static_cast<int32>(file->GetSize());
        str.resize(fileSize);
        uint32 readSize = file->Read(&str.front(), static_cast<int32>(fileSize));
        if (readSize == fileSize)
        {
            FormulaParser parser(str);
            return parser.ParseMap();
        }
        else
        {
            DAVA_THROW(FormulaException, Format("Can't read file: %s", path.GetAbsolutePathname().c_str()), 0, 0);
        }
    }
    DAVA_THROW(FormulaException, Format("Can't open file: %s", path.GetAbsolutePathname().c_str()), 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// UIDataRootModel
////////////////////////////////////////////////////////////////////////////////
UIDataRootModel::UIDataRootModel(bool editorMode)
    : UIDataModel(nullptr, PRIORITY_ROOT, editorMode)
{
    std::shared_ptr<FormulaFunctionContext> funcContext = std::make_shared<FormulaFunctionContext>(std::shared_ptr<FormulaContext>());
    funcContext->RegisterFunction("min", MakeFunction(&UIDataBindingDefaultFunctions::IntMin));
    funcContext->RegisterFunction("min", MakeFunction(&UIDataBindingDefaultFunctions::FloatMin));
    funcContext->RegisterFunction("max", MakeFunction(&UIDataBindingDefaultFunctions::IntMax));
    funcContext->RegisterFunction("max", MakeFunction(&UIDataBindingDefaultFunctions::FloatMax));
    funcContext->RegisterFunction("clamp", MakeFunction(&UIDataBindingDefaultFunctions::IntClamp));
    funcContext->RegisterFunction("clamp", MakeFunction(&UIDataBindingDefaultFunctions::FloatClamp));
    funcContext->RegisterFunction("abs", MakeFunction(&UIDataBindingDefaultFunctions::IntAbs));
    funcContext->RegisterFunction("abs", MakeFunction(&UIDataBindingDefaultFunctions::FloatAbs));
    funcContext->RegisterFunction("toDeg", MakeFunction(&UIDataBindingDefaultFunctions::RadToDeg));
    funcContext->RegisterFunction("toRad", MakeFunction(&UIDataBindingDefaultFunctions::DegToRad));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::IntToStr));
    funcContext->RegisterFunction("str1000Separated", MakeFunction(&UIDataBindingDefaultFunctions::IntToStr1000Separated));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::FloatToStr));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::FloatToStrWithPrecision));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::Float64ToStr));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::Float64ToStrWithPrecision));
    funcContext->RegisterFunction("str", MakeFunction(&UIDataBindingDefaultFunctions::Vector2ToStr));
    funcContext->RegisterFunction("localize", MakeFunction(&UIDataBindingDefaultFunctions::Localize));
    context = funcContext;
}

UIDataRootModel::~UIDataRootModel()
{
}

UIComponent* UIDataRootModel::GetComponent() const
{
    return nullptr;
}

const std::shared_ptr<FormulaContext>& UIDataRootModel::GetRootContext() const
{
    return context;
}

bool UIDataRootModel::Process(UIDataBindingDependenciesManager* dependenciesManager)
{
    return UIDataModel::Process(dependenciesManager);
}

////////////////////////////////////////////////////////////////////////////////
// UIDataSourceModel
////////////////////////////////////////////////////////////////////////////////

UIDataSourceModel::UIDataSourceModel(UIDataSourceComponent* component_, bool editorMode)
    : UIDataModel(component_, PRIORITY_SOURCE, editorMode)
    , component(component_)
{
}

UIDataSourceModel::~UIDataSourceModel()
{
}

UIComponent* UIDataSourceModel::GetComponent() const
{
    return component;
}

bool UIDataSourceModel::Process(UIDataBindingDependenciesManager* dependenciesManager)
{
    if (!UIDataModel::Process(dependenciesManager))
    {
        return false;
    }

    std::shared_ptr<FormulaContext> parentContext = GetParent()->GetFormulaContext();
    if (component->IsDirty() || GetParent()->IsDirty() ||
        (dependencyId != UIDataBindingDependenciesManager::UNKNOWN_DEPENDENCY && dependenciesManager->IsDirty(dependencyId)))
    {
        component->SetDirty(false);
        dirty = true;

        bool hasToResetError = true;
        if (component->GetData().IsValid())
        {
            sourceData = std::shared_ptr<FormulaDataMap>();
            context = std::make_shared<FormulaReflectionContext>(component->GetData(), parentContext);
            Vector<void*> dependencies;
            dependencies.push_back(component->GetData().GetValueObject().GetVoidPtr());
            dependencyId = dependenciesManager->MakeDependency(dependencyId, dependencies);
        }
        else if (!component->GetDataFile().IsEmpty())
        {
            if (dependencyId != UIDataBindingDependenciesManager::UNKNOWN_DEPENDENCY)
            {
                dependenciesManager->ReleaseDepencency(dependencyId);
            }

            try
            {
                sourceData = LoadDataMap(component->GetDataFile());
                context = std::make_shared<FormulaReflectionContext>(Reflection::Create(ReflectedObject(sourceData.get())), parentContext);
            }
            catch (const FormulaException& error)
            {
                hasToResetError = false;
                NotifyError(error.GetFormattedMessage(), "UIDataSourceComponent/dataFile");
                context = parentContext;
            }
        }
        else
        {
            if (dependencyId != UIDataBindingDependenciesManager::UNKNOWN_DEPENDENCY)
            {
                dependenciesManager->ReleaseDepencency(dependencyId);
            }

            sourceData = std::shared_ptr<FormulaDataMap>();
            context = parentContext;
        }

        if (hasToResetError)
        {
            ResetError();
        }
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// UIDataViewModel
////////////////////////////////////////////////////////////////////////////////

UIDataViewModel::UIDataViewModel(UIDataViewModelComponent* component_, bool editorMode)
    : UIDataModel(component_, PRIORITY_VIEW_MODEL, editorMode)
    , component(component_)
{
}

UIDataViewModel::~UIDataViewModel()
{
}

UIComponent* UIDataViewModel::GetComponent() const
{
    return component;
}

bool UIDataViewModel::Process(UIDataBindingDependenciesManager* dependenciesManager)
{
    if (!UIDataModel::Process(dependenciesManager))
    {
        return false;
    }

    std::shared_ptr<FormulaContext> parentContext = GetParent()->GetFormulaContext();

    if (component->IsDirty() || GetParent()->IsDirty())
    {
        component->SetDirty(false);
        dirty = true;

        bool hasToResetError = true;

        if (!component->GetViewModelFile().IsEmpty())
        {
            try
            {
                viewData = LoadDataMap(component->GetViewModelFile());
                context = std::make_shared<FormulaReflectionContext>(Reflection::Create(ReflectedObject(viewData.get())), parentContext);
            }
            catch (const FormulaException& error)
            {
                hasToResetError = false;
                NotifyError(error.GetFormattedMessage(), "UIDataSourceComponent/testData");
                context = parentContext;
            }
        }
        else
        {
            context = parentContext;
        }

        if (hasToResetError)
        {
            ResetError();
        }
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// UIDataScopeModel
////////////////////////////////////////////////////////////////////////////////

UIDataScopeModel::UIDataScopeModel(UIDataScopeComponent* component_, bool editorMode)
    : UIDataModel(component_, PRIORITY_SCOPE, editorMode)
    , component(component_)
{
}

UIDataScopeModel::~UIDataScopeModel()
{
}

UIComponent* UIDataScopeModel::GetComponent() const
{
    return component;
}

bool UIDataScopeModel::Process(UIDataBindingDependenciesManager* dependenciesManager)
{
    if (!UIDataModel::Process(dependenciesManager))
    {
        return false;
    }

    std::shared_ptr<FormulaContext> parentContext = GetParent()->GetFormulaContext();

    bool hasToResetError = false;
    bool expChanged = false;
    bool processed = false;

    if (component->IsDirty() || GetParent()->IsDirty())
    {
        component->SetDirty(false);
        dirty = true;

        expChanged = true;
        hasToResetError = true;
        FormulaParser parser(component->GetExpression());
        try
        {
            expression = parser.ParseExpression();
        }
        catch (const FormulaException& error)
        {
            expression.reset();
            hasToResetError = false;
            NotifyError(error.GetFormattedMessage(), "UIDataScopeComponent/expression");
        }
        processed = true;
    }

    if (expression.get())
    {
        if (expChanged || dependenciesManager->IsDirty(dependencyId))
        {
            dirty = true;
            try
            {
                FormulaExecutor executor(parentContext.get());

                Reflection ref = executor.GetDataReference(expression.get());
                if (ref.IsValid())
                {
                    context = std::make_shared<FormulaReflectionContext>(ref, parentContext);
                    const Vector<void*>& dependencies = executor.GetDependencies();
                    if (!dependencies.empty())
                    {
                        dependencyId = dependenciesManager->MakeDependency(dependencyId, dependencies);
                    }
                    else if (dependencyId != UIDataBindingDependenciesManager::UNKNOWN_DEPENDENCY)
                    {
                        dependenciesManager->ReleaseDepencency(dependencyId);
                        dependencyId = UIDataBindingDependenciesManager::UNKNOWN_DEPENDENCY;
                    }
                }
                else
                {
                    DAVA_THROW(FormulaException, Format("Can't get data %s", component->GetExpression().c_str()), expression->GetLineNumber(), expression->GetPositionInLine());
                }
            }
            catch (const FormulaException& error)
            {
                context = parentContext;
                hasToResetError = false;
                NotifyError(error.GetErrorMessage(), "UIDataScopeComponent/expression");
            }
            processed = true;
        }
    }
    else
    {
        context = parentContext;
    }

    if (hasToResetError)
    {
        ResetError();
    }

    return processed;
}
}
