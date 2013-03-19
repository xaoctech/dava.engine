#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class LibraryWidget;
}

class LibraryWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit LibraryWidget(QWidget *parent = 0);
    ~LibraryWidget();
	
//	QTreeWidgetItem* AddControl(const QString& name);
	QTreeWidgetItem* AddControl(const QString& name, const QString& iconPath);
	void RemoveControl(QTreeWidgetItem* item);
	void UpdateControl(QTreeWidgetItem* item, const QString& name);
	void SetItemVisible(QTreeWidgetItem* item, bool visible);
	
private:
    Ui::LibraryWidget *ui;
};

#endif // LIBRARYWIDGET_H
