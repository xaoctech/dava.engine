#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Reflection/Reflection.h>
#include <Functional/Function.h>

#include <QWidget>

class QTreeView;

namespace DAVA
{
class ReflectedPropertyModel;
class ExtensionChain;
class ContextAccessor;
class OperationInvoker;
class UI;

class PropertiesView : public QWidget
{
    Q_OBJECT
public:
    enum eViewMode
    {
        VIEW_MODE_REGULAR_TREE = 0x1,
        VIEW_MODE_FAVORITES = 0x2
    };

    enum UpdatePolicy
    {
        FullUpdate,
        FastUpdate
    };

    class Updater
    {
    public:
        virtual ~Updater() = default;

        Signal<UpdatePolicy> update;
    };
    /**
        Create PropertiesView widget with ReflectedModel. As data source for ReflectedMode use value of "objectsField"
        Value of "objectsField" could be casted to Vector<Reflection>
    */
    struct Params
    {
        Params(const WindowKey& key)
            : wndKey(key)
        {
        }

        WindowKey wndKey;
        ContextAccessor* accessor = nullptr;
        OperationInvoker* invoker = nullptr;
        UI* ui = nullptr;
        FieldDescriptor objectsField;
        FieldDescriptor developerModeField;
        String settingsNodeName;
        std::weak_ptr<Updater> updater;
        bool isInDevMode = false;
        bool showToolBar = true;
    };

    PropertiesView(const Params& params);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);
    void Update(UpdatePolicy policy);

    void UpdateExpanded();
    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);
    void OnFavoritesEditChanged(bool isChecked);

    bool IsRegularTreeShown() const;
    void SetRegularTreeShown(bool isShown);

    bool IsFavoritesShown() const;
    void SetFavoritesShown(bool isShown);

    bool IsInDeveloperMode() const;
    void SetDeveloperMode(bool isDevMode);

    void UpdateViewRootIndex();
    void OnCurrentChanged(const QModelIndex& index, const QModelIndex& prev);

private:
    FieldBinder binder;
    Params params;
    class PropertiesTreeView;
    PropertiesTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    QtConnections connections;
    bool isExpandUpdate = false;

    Vector<FastName> currentIndexPath;
    bool isModelUpdate = false;

    int32 viewMode = VIEW_MODE_REGULAR_TREE;

    DAVA_REFLECTION(PropertiesView);
};
}