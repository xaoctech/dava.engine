/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "QtPropertyModel.h"
#include "QtPropertyData.h"

QtPropertyModel::QtPropertyModel(QWidget *optionalWidgetViewport, QObject* parent /* = 0 */)
	: QAbstractItemModel(parent)
	, trackEdit(false)
	, OWviewport(optionalWidgetViewport)
{ 
	root = new QtPropertyData();
	root->model = this;
	root->SetOWViewport(OWviewport);
}

QtPropertyModel::~QtPropertyModel()
{ 
	delete root;
}

QModelIndex QtPropertyModel::index(int row, int column, const QModelIndex & parent /* = QModelIndex() */) const
{
	QModelIndex ret;
	QtPropertyData *parentData = root;

	if(parent.isValid())
	{
		parentData = (QtPropertyData *) parent.internalPointer();
	}

	if(NULL != parentData && row < parentData->ChildCount() && column < 2)
	{
		ret = createIndex(row, column, parentData->ChildGet(row));
	}

	return ret;
}

QModelIndex QtPropertyModel::parent(const QModelIndex & index) const
{
	QModelIndex ret;

	QtPropertyData *data = itemFromIndex(index);
	if(NULL != data)
	{
		ret = indexFromItem(data->Parent());
	}

	return ret;
}

int QtPropertyModel::rowCount(const QModelIndex & parent /* = QModelIndex() */) const
{
	int count = 0;

	QtPropertyData *data = itemFromIndex(parent);
	if(NULL != data)
	{
		count = data->ChildCount();
	}

	return count;
}

int QtPropertyModel::columnCount(const QModelIndex & parent /* = QModelIndex() */) const
{
	return 2;
}

QVariant QtPropertyModel::data(const QModelIndex & index, int role /* = Qt::DisplayRole */) const
{
	QVariant ret;
	QtPropertyData *data = itemFromIndex(index);
	if(NULL != data)
	{
		if(index.column() == 0)
		{
			switch(role)
			{
			case Qt::DisplayRole:
				ret = data->GetName();
				break;
			case Qt::FontRole:
			case Qt::BackgroundRole:
			case Qt::ForegroundRole:
				ret = data->data(role);
				break;
			default:
				break;
			}
		}
		else
		{
			ret = data->data(role);
		}
	}

	return ret;
}

QVariant QtPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant ret;

	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
		case 0:
			ret = "Property";
			break;
		case 1:
			ret = "Value";
			break;
		default:
			break;
		}
	}

	return ret;
}

bool QtPropertyModel::setData(const QModelIndex & index, const QVariant & value, int role /* = Qt::EditRole */)
{
	bool ret = false;

	QtPropertyData *data = itemFromIndex(index);
	if(NULL != data && index.column() == 1)
	{
		ret = data->setData(value, role);
	}

	return ret;
}

Qt::ItemFlags QtPropertyModel::flags(const QModelIndex & index) const
{
	Qt::ItemFlags ret = 0;

	if(index.column() == 1)
	{
		QtPropertyData *data = itemFromIndex(index);
		if(NULL != data)
		{
			ret = Qt::ItemIsSelectable | data->GetFlags();
		}
	}
	else
	{
		ret = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}

	return ret;
}

QtPropertyData* QtPropertyModel::itemFromIndex(const QModelIndex & index) const
{
	QtPropertyData *ret = NULL;

	if(!index.isValid())
	{
		return root;
	}

	if(index.isValid() && index.model() == this && index.column() < 2)
	{
		ret = (QtPropertyData *)index.internalPointer();
	}

	return ret;
}

QModelIndex QtPropertyModel::indexFromItem(QtPropertyData *data) const
{
	QModelIndex ret;

	if(NULL != data && data->GetModel() == this)
	{
		QtPropertyData *parentData = data->Parent();
		if(NULL != parentData)
		{
			int row = parentData->ChildIndex(data);
			if(row >= 0)
			{
				ret = createIndex(row, 1, data);
			}
		}
	}

	return ret;
}

QModelIndex QtPropertyModel::AppendProperty(const QString &name, QtPropertyData* data, const QModelIndex &parent /* = QModelIndex() */)
{
	if(NULL != data)
	{
		QtPropertyData *parentData = itemFromIndex(parent);
		if(NULL == parentData)
		{
			parentData = root;
		}

		parentData->ChildAdd(name, data);
	}

	return indexFromItem(data);
}

void QtPropertyModel::RemoveProperty(const QModelIndex &index)
{
	QtPropertyData *data = itemFromIndex(index);
	if(NULL != data)
	{
		QtPropertyData *parentData = data->Parent();
		if(NULL != parentData)
		{
			parentData->ChildRemove(data);
		}
	}
}

void QtPropertyModel::RemovePropertyAll()
{
	root->ChildRemoveAll();
}

void QtPropertyModel::UpdateStructure(const QModelIndex &parent /* = QModelIndex */)
{
	UpdateStructureInternal(parent);
}

void QtPropertyModel::UpdateStructureInternal(const QModelIndex &i)
{
	QtPropertyData *data = itemFromIndex(i);
	if(NULL != data)
	{
		if(data->UpdateValue())
		{
			emit dataChanged(i, i);
		}

		for(int row = 0; row < rowCount(i); ++row)
		{
			UpdateStructureInternal(index(row, 0, i));
		}
	}
}

void QtPropertyModel::DataChanged(QtPropertyData *data, int reason)
{
	if(trackEdit)
	{
		QModelIndex index = indexFromItem(data);
		if(index.isValid())
		{
			if(reason != QtPropertyData::VALUE_EDITED)
			{
				emit dataChanged(index, index);
			}

			emit PropertyChanged(index);

			if(reason == QtPropertyData::VALUE_EDITED)
			{
				emit PropertyEdited(index);
			}
		}
	}
}

void QtPropertyModel::DataAboutToBeAdded(QtPropertyData *parent, int first, int last)
{
	if(NULL != parent)
	{
		QModelIndex index = indexFromItem(parent);
		if(index.isValid())
		{
			// same index, but column will be 0
			index = createIndex(index.row(), 0, index.internalPointer());
		}

		beginInsertRows(index, first, last);
	}
}

void QtPropertyModel::DataAdded()
{
	endInsertRows();
}

void QtPropertyModel::DataAboutToBeRemoved(QtPropertyData *parent, int first, int last)
{
	if(NULL != parent)
	{
		QModelIndex index = indexFromItem(parent);
		if(index.isValid())
		{
			// same index, but column will be 0
			index = createIndex(index.row(), 0, index.internalPointer());
		}

		beginRemoveRows(index, first, last);
	}
}

void QtPropertyModel::DataRemoved()
{
	endRemoveRows();
}

void QtPropertyModel::SetEditTracking(bool enabled)
{
	trackEdit = enabled;
}

bool QtPropertyModel::GetEditTracking()
{
	return trackEdit;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Filtering model
//
//////////////////////////////////////////////////////////////////////////////////////////////////

QtPropertyFilteringModel::QtPropertyFilteringModel(QtPropertyModel *_propModel, QObject *parent /* = NULL */)
: QSortFilterProxyModel(parent)
, propModel(_propModel)
{
	setSourceModel(propModel);
}

bool QtPropertyFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(NULL != propModel)
	{
		// check self accept
		if(selfAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}

		//accept if any of the parents is accepted
		QModelIndex parent = sourceParent;
		while(parent.isValid())
		{
			if(selfAcceptRow(parent.row(), parent.parent()))
			{
				return true;
			}

			parent = parent.parent();
		}

		// accept if any child is accepted
		if(childrenAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}
	}

	return false;
}

QtPropertyData* QtPropertyFilteringModel::itemFromIndex(const QModelIndex &index) const
{
	return propModel->itemFromIndex(mapToSource(index));
}

bool QtPropertyFilteringModel::selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool QtPropertyFilteringModel::childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool ret = false;

	QModelIndex index = propModel->index(sourceRow, 0, sourceParent);
	if(propModel->rowCount(index) > 0)
	{
		for(int i = 0; i < propModel->rowCount(index); i++)
		{
			if(selfAcceptRow(i, index) || childrenAcceptRow(i, index))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}
