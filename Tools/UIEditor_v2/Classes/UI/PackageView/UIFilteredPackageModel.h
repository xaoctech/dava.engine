//
//  UIFilteredPackageModel.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/16/14.
//
//

#ifndef __UIEditor__UIFilteredPackageModel__
#define __UIEditor__UIFilteredPackageModel__

#include "UIFilteredPackageModel.h"
#include <QSortFilterProxyModel>

class UIFilteredPackageModel: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    UIFilteredPackageModel(QObject *parent = NULL);
    ~UIFilteredPackageModel();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    
    bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
    //QModelIndexList expandedRows;
};

#endif /* defined(__UIEditor__UIFilteredPackageModel__) */
