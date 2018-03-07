#include "UI/Dialogs/VarTableEditorDialog.h"
#include "FileSystem/LocalizationSystem.h"

#include <Utils/QtDavaConvertion.h>
#include <UI/UIPackage.h>
#include <UI/UIControl.h>
#include <UI/Properties/VarTable.h>
#include <Utils/StringFormat.h>
#include <Reflection/ReflectedTypeDB.h>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QComboBox>
#include <QStringListModel>

using namespace DAVA;

class ComboBoxDelegate : public QItemDelegate
{
public:
    ComboBoxDelegate(QObject* parent)
        :
        QItemDelegate(parent)
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const
    {
        QComboBox* editor = new QComboBox(parent);
        editor->setEditable(false);

        QStringList stringlist;
        for (const Type* type : VarTable::SUPPORTED_TYPES)
        {
            const ReflectedType* reflType = ReflectedTypeDB::GetByType(type);
            stringlist.append(StringToQString(reflType->GetPermanentName()));
        }
        editor->setModel(new QStringListModel(stringlist));
        return editor;
    }

    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(value);

        int i = comboBox->findData(value);
        if (i != -1)
        { // -1 for not found
            comboBox->setCurrentIndex(i);
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const
    {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option, const QModelIndex& /* index */) const
    {
        editor->setGeometry(option.rect);
    }
};

VarTableEditorDialog::VarTableEditorDialog(const DAVA::VarTable& values_, QWidget* parent)
    : QDialog(parent)
    , header(QList<QString>({ "Name", "Type", "Value" }))
    , values(values_)
{
    ui.setupUi(this);

    ui.addButton->setIcon(QIcon(":/Icons/add.png"));
    ui.removeButton->setIcon(QIcon(":/Icons/editclear.png"));

    connect(ui.addButton, &QPushButton::clicked, this, &VarTableEditorDialog::OnAddRow);
    connect(ui.removeButton, &QPushButton::clicked, this, &VarTableEditorDialog::OnRemoveRow);
    connect(ui.pushButton_ok, &QPushButton::clicked, this, &VarTableEditorDialog::OnOk);
    connect(ui.pushButton_cancel, &QPushButton::clicked, this, &VarTableEditorDialog::OnCancel);

    model = new QStandardItemModel(0, header.size(), this);
    model->setHorizontalHeaderLabels(header);

    values.ForEachProperty([&](const FastName& varName, const Any& value) {
        const Type* type = value.GetType();
        const ReflectedType* reflType = ReflectedTypeDB::GetByType(type);

        QList<QStandardItem*> items;

        auto column = [&](const String& s) {
            QStandardItem* item = new QStandardItem();
            item->setData(StringToQString(s), Qt::DisplayRole);
            items.push_back(item);
        };

        column(String(varName.c_str()));
        column(reflType->GetPermanentName());
        column(VarTable::AnyToString(value));

        model->appendRow(items);
    });

    ui.tableView->setModel(model);
    ui.tableView->resizeColumnsToContents();

    ui.tableView->setItemDelegateForColumn(1, new ComboBoxDelegate(this));
}

const VarTable& VarTableEditorDialog::GetValues() const
{
    return values;
}

void VarTableEditorDialog::OnOk()
{
    values.ClearProperties();

    for (int row = 0; row < model->rowCount(); row++)
    {
        String nameStr = QStringToString(model->data(model->index(row, 0), Qt::DisplayRole).toString());
        String typeStr = QStringToString(model->data(model->index(row, 1), Qt::DisplayRole).toString());
        String valueStr = QStringToString(model->data(model->index(row, 2), Qt::DisplayRole).toString());
        const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(typeStr);
        const Type* type = reflectedType ? reflectedType->GetType() : Type::Instance<String>();
        DVASSERT(type);
        values.SetPropertyValue(FastName(nameStr), VarTable::ParseString(type, valueStr));
    }
    accept();
}

void VarTableEditorDialog::OnCancel()
{
    reject();
}

void VarTableEditorDialog::OnAddRow()
{
    QList<QStandardItem*> items;
    auto column = [&](const String& s) {
        QStandardItem* item = new QStandardItem();
        item->setData(StringToQString(s), Qt::DisplayRole);
        items.push_back(item);
    };
    String name = "variable";
    for (int32 i = 1; i < 1000; i++)
    {
        name = Format("variable%d", i);
        int32 row = 0;
        for (; row < model->rowCount(); row++)
        {
            QString nameStr = model->data(model->index(row, 0), Qt::DisplayRole).toString();
            if (nameStr == name.c_str())
            {
                break;
            }
        }
        if (row == model->rowCount())
        {
            //not found
            break;
        }
    }

    column(name);
    column("String");
    column("");
    model->appendRow(items);
}

void VarTableEditorDialog::OnRemoveRow()
{
    QModelIndex current = ui.tableView->selectionModel()->currentIndex();
    if (current.isValid())
    {
        model->removeRow(current.row());
    }
}
