#ifndef FONTMANAGERDIALOG_H
#define FONTMANAGERDIALOG_H

#include <QDialog>
#include <DAVAEngine.h>

class QStandardItemModel;
class QStringList;
class QItemSelectionModel;
class QStandardItem;

using namespace DAVA;

namespace Ui {
class FontManagerDialog;
}

class FontManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontManagerDialog(bool okButtonEnable = false, const QString& graphicsFontPath = QString(), QDialog *parent = 0);
    ~FontManagerDialog();
    //Return created font
    Font * ResultFont();
private:
    Ui::FontManagerDialog *ui;
    QStandardItemModel *tableModel;
    Font *dialogResultFont;
	QString currentFontPath;
    
    void ConnectToSignals();
    void InitializeTableView();
    void UpdateTableViewContents();
	void UpdateDialogInformation();
	Font* GetSelectedFont(QItemSelectionModel *selectionModel);
	//void SetDefaultItemFont(QStandardItem *item, QString defaultFontName, QString fontName);
	QStandardItem* CreateFontItem(QString itemText, QString fontName, QString defaultFontName);

private slots:
    void OkButtonClicked();
    void SetDefaultButtonClicked();

};

#endif // FONTMANAGERDIALOG_H
