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



#include "LibraryFilteringModel.h"

LibraryFilteringModel::LibraryFilteringModel(QObject *parent /* = NULL */)
    : QSortFilterProxyModel(parent)
    , model(NULL)
{
}

bool LibraryFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(model == NULL) return false;

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

	return false;
}

void LibraryFilteringModel::SetModel(QAbstractItemModel *newModel)
{
    model = newModel;
    setSourceModel(model);
}

bool LibraryFilteringModel::selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool LibraryFilteringModel::childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(model == NULL) return false;
    
	bool ret = false;
    
	QModelIndex index = model->index(sourceRow, 0, sourceParent);
	if(model->rowCount(index) > 0)
	{
		for(int i = 0; i < model->rowCount(index); i++)
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

