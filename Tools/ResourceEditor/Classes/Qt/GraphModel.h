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
    Q_OBJECT

public:
    GraphModel(QObject *parent = 0);
    ~GraphModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    void setupModelData(GraphItem *parent);

    GraphItem *rootItem;
};

#endif // __GRAPH_MODEL_H__
