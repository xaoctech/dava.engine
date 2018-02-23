#include "TArc/Controls/ListView.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <Functional/Signal.h>

#include <QAbstractListModel>
#include <QItemSelectionModel>
#include <QVariant>
#include <QStyledItemDelegate>

namespace DAVA
{
namespace ListViewDetails
{
class ListModel : public QAbstractListModel
{
public:
    int rowCount(const QModelIndex& parent) const override
    {
        return static_cast<int>(values.size());
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            int row = index.row();
            DVASSERT(row < static_cast<int>(values.size()));
            return values[row].second.Cast<QString>();
        }

        return QVariant();
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) override
    {
        if (role != Qt::EditRole)
        {
            return false;
        }

        if (value.canConvert<QString>() == false)
        {
            return false;
        }

        QString v = value.value<QString>();

        int row = index.row();
        DVASSERT(row < static_cast<int>(values.size()));
        dataEdited.Emit(values[row].first, v.toStdString());
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled);
        if (editingEnabled == true)
        {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }

    void BeginReset()
    {
        beginResetModel();
    }

    void EndReset()
    {
        endResetModel();
    }

    void SetEditingEnabled(bool isEnabled)
    {
        editingEnabled = isEnabled;
    }

    Vector<std::pair<Any, Any>> values;
    Signal<const Any&, const String&> dataEdited;

private:
    bool editingEnabled = false;
};
}

ListView::ListView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ListView::ListView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

QAbstractItemDelegate* ListView::CreateDelegate()
{
    return new QStyledItemDelegate(this);
}

void ListView::SetupControl()
{
    setUniformItemSizes(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    listModel = new ListViewDetails::ListModel();
    setModel(listModel);
    static_cast<ListViewDetails::ListModel*>(listModel)->dataEdited.Connect(this, &ListView::OnDataChanged);

    QItemSelectionModel* selectModel = new QItemSelectionModel(listModel, this);
    connections.AddConnection(selectModel, &QItemSelectionModel::selectionChanged, MakeFunction(this, &ListView::OnSelectionChanged));
    setSelectionModel(selectModel);
}

void ListView::UpdateControl(const ControlDescriptor& fields)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());
    if (delegateCreated == false)
    {
        setItemDelegate(CreateDelegate());
        delegateCreated = true;
    }

    ListViewDetails::ListModel* m = static_cast<ListViewDetails::ListModel*>(listModel);
    if (fields.IsChanged(Fields::ValueList))
    {
        FastName valueFieldName = GetFieldValue(Fields::ValueFieldName, FastName());
        auto getValueFn = [&valueFieldName](const Reflection& r) {
            if (valueFieldName.IsValid() == true)
            {
                Reflection subField = r.GetField(valueFieldName);
                DVASSERT(subField.IsValid() == true);
                return subField.GetValue();
            }

            return r.GetValue();
        };
        m->BeginReset();
        m->values.clear();

        Reflection r = model.GetField(fields.GetName(Fields::ValueList));
        DVASSERT(r.IsValid());
        Vector<Reflection::Field> fields = r.GetFields();
        m->values.reserve(fields.size());
        for (const Reflection::Field& f : fields)
        {
            m->values.emplace_back(f.key, getValueFn(f.ref));
        }

        m->EndReset();
    }

    if (fields.IsChanged(Fields::CurrentValue) || fields.IsChanged(Fields::ValueList))
    {
        Any currentValue = GetFieldValue(Fields::CurrentValue, Any());
        bool currentValueSet = false;
        for (size_t i = 0; i < m->values.size(); ++i)
        {
            if (m->values[i].first == currentValue)
            {
                QModelIndex index = m->index(static_cast<int>(i), 0, QModelIndex());
                QItemSelectionModel* selectModel = selectionModel();
                selectModel->clearCurrentIndex();
                selectModel->select(index, QItemSelectionModel::ClearAndSelect);
                currentValueSet = true;
                break;
            }
        }

        if (currentValueSet == false)
        {
            SetCurrentItem(Any());
        }
    }

    if (fields.IsChanged(Fields::ItemEditingEnabled) == true)
    {
        m->SetEditingEnabled(GetFieldValue<bool>(Fields::ItemEditingEnabled, false));
    }

    setEnabled(GetFieldValue(Fields::Enabled, false));
}

void ListView::OnDataChanged(const Any& key, const String& value)
{
    DVASSERT(GetFieldValue(Fields::ItemEditingEnabled, false) == true);

    Reflection r = model.GetField(GetFieldName(Fields::ValueList));
    DVASSERT(r.IsValid());
    Vector<Reflection::Field> fields = r.GetFields();
    for (const Reflection::Field& f : fields)
    {
        if (f.key == key)
        {
            FastName valueFieldName = GetFieldValue(Fields::ValueFieldName, FastName());
            if (valueFieldName.IsValid() == true)
            {
                Reflection subField = f.ref.GetField(valueFieldName);
                DVASSERT(subField.IsValid());
                subField.SetValue(value);
            }
            else
            {
                f.ref.SetValue(value);
            }
        }
    }
}

void ListView::OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());

    QModelIndexList indexList = newSelection.indexes();
    DVASSERT(indexList.size() < 2);
    if (indexList.size() == 1)
    {
        const QModelIndex& index = indexList.front();
        ListViewDetails::ListModel* m = static_cast<ListViewDetails::ListModel*>(listModel);
        DVASSERT(index.row() < m->values.size());
        SetCurrentItem(m->values[index.row()].first);
    }
    else
    {
        SetCurrentItem(Any());
    }
}

void ListView::SetCurrentItem(const Any& key)
{
    FastName currentItemFieldName = GetFieldName(Fields::CurrentValue);
    if (currentItemFieldName.IsValid() == false)
    {
        return;
    }

    wrapper.SetFieldValue(currentItemFieldName, key);
}

} // namespace DAVA
