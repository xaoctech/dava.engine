#ifndef FONTMANAGERDIALOG_H
#define FONTMANAGERDIALOG_H

#include <QDialog>
#include <DAVAEngine.h>

class QStandardItemModel;
class QStringList;

using namespace DAVA;

namespace Ui {
class FontManagerDialog;
}

class FontManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontManagerDialog(bool okButtonEnable = false, QDialog *parent = 0);
    ~FontManagerDialog();
    //Return created font
    Font * ResultFont();
private:
    Ui::FontManagerDialog *ui;
    QStandardItemModel *tableModel;
    Font *dialogResultFont;
    
    void ConnectToSignals();
    void InitializeTableView();
    void UpdateTableViewContents();
	void UpdateDialogInformation();

private slots:
    void OkButtonClicked();
};

#endif // FONTMANAGERDIALOG_H
