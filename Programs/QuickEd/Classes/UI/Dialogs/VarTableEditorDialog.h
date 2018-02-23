#ifndef __VAR_TABLE_EDITOR_DIALOG_H__
#define __VAR_TABLE_EDITOR_DIALOG_H__

#include "ui_VarTableEditorDialog.h"

#include <UI/Properties/VarTable.h>

class QStandardItemModel;

Q_DECLARE_METATYPE(DAVA::VarTable);

class VarTableEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VarTableEditorDialog(const DAVA::VarTable& values, QWidget* parent = nullptr);
    ~VarTableEditorDialog() = default;

    const DAVA::VarTable& GetValues() const;

private slots:
    void OnOk();
    void OnCancel();
    void OnAddRow();
    void OnRemoveRow();

private:
    Ui::VarTableEditorDialog ui;
    QList<QString> header;
    DAVA::VarTable values;
    QStandardItemModel* model = nullptr;
};
#endif // __VAR_TABLE_EDITOR_DIALOG_H__
