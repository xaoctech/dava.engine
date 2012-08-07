#ifndef __GRAPH_MODEL_H__
#define __GRAPH_MODEL_H__

#include <QAbstractItemModel>

#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QItemSelectionModel>


class GraphItem;
class QTreeView;
class GraphModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    GraphModel(QObject *parent = 0);
    virtual ~GraphModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent);

    
    void * ItemData(const QModelIndex &index) const;
    
    virtual void Rebuild() = 0;
    
    
    virtual void MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndext) = 0;
    
    
    QItemSelectionModel *GetSelectionModel();
    
    void Activate(QTreeView *view);
    void Deactivate();
    
protected slots:
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {};

    
protected:
    
    GraphItem * ParentItem(const QModelIndex &parent = QModelIndex()) const;
    
    GraphItem * ItemForData(void * usedData);
    GraphItem * ItemForData(GraphItem * item, void * usedData);
    
protected:

    GraphItem *rootItem;
    
    QItemSelectionModel *itemSelectionModel;
    
    QTreeView *attachedTreeView;
};

#endif // __GRAPH_MODEL_H__
