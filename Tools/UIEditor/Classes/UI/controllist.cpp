#include "Classes/UI/controllist.h"
#include "HierarchyTreeController.h"

ControlMimeData::ControlMimeData(const QString& controlName)
{
	this->controlName = controlName;
}

ControlMimeData::~ControlMimeData()
{

}

ControlList::ControlList(QWidget *parent) :
    QTreeWidget(parent)
{
    // Recover the original font size - it is lost by some reason.
    QFont currentFont = this->font();
    currentFont.setPointSize(13);
    this->setFont(currentFont);
}

QStringList ControlList::mimeTypes() const
{
	QStringList list = QTreeWidget::mimeTypes();
	return list;
}

QMimeData* ControlList::mimeData(const QList<QTreeWidgetItem*> items) const
{
	Q_ASSERT(items.size() == 1);
	if (items.size() != 1)
		return NULL;
	
	QTreeWidgetItem* item = items[0];
	ControlMimeData* data = new ControlMimeData(item->text(0));
	
	HierarchyTreeController::Instance()->ResetSelectedControl();
	
	return data;
}

bool ControlList::dropMimeData(QTreeWidgetItem *parent, int index,
						  const QMimeData *data, Qt::DropAction action)
{
	bool bRes = QTreeWidget::dropMimeData(parent, index, data, action);
	return bRes;
}

Qt::DropActions ControlList::supportedDropActions() const
{
	Qt::DropActions	actions = QTreeWidget::supportedDropActions();
	return actions;
}
