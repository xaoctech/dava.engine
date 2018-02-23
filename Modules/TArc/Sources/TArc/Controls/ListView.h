#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <QListView>
#include <QAbstractItemModel>

namespace DAVA
{
class ListView : public ControlProxyImpl<QListView>
{
    using TBase = ControlProxyImpl<QListView>;

public:
    enum Fields : uint32
    {
        CurrentValue,
        ValueList,
        ValueFieldName,
        ItemEditingEnabled,
        Enabled,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ListView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ListView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

    virtual QAbstractItemDelegate* CreateDelegate();

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& fields) override;

    void OnDataChanged(const Any& key, const String& value);

    void OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);
    void SetCurrentItem(const Any& key);

    QAbstractItemModel* listModel = nullptr;
    QtConnections connections;
    bool updateGuard = false;
    bool delegateCreated = false;
};
} // namespace DAVA
