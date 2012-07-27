#ifndef __GRAPH_MODEL_H__
#define __GRAPH_MODEL_H__

#include <QAbstractItemModel>

#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>


class GraphItem;

class GraphModel : public QAbstractItemModel
{
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

protected:

    virtual void SetupModelData() = 0;

protected:

    GraphItem *rootItem;
};

#endif // __GRAPH_MODEL_H__
