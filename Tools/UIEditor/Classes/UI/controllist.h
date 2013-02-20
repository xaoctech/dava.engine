#ifndef CONTROLLIST_H
#define CONTROLLIST_H

#include <QTreeWidget>
#include <QMimeData>

class ControlMimeData: public QMimeData
{
	Q_OBJECT
public:
	ControlMimeData(const QString& controlName);
	~ControlMimeData();

	QString GetControlName() const {return controlName;};
	
private:
	QString controlName;
};

class ControlList : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ControlList(QWidget *parent = 0);
    
protected:
	virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QList<QTreeWidgetItem*> items) const;
    virtual bool dropMimeData(QTreeWidgetItem *parent, int index,
                              const QMimeData *data, Qt::DropAction action);
    virtual Qt::DropActions supportedDropActions() const;

signals:
    
public slots:
    
};

#endif // CONTROLLIST_H
