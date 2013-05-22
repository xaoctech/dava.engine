/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DockSceneTree/SceneTreeItem.h"


SceneTreeItem::SceneTreeItem(DAVA::Entity *_entity)
	: entity(_entity)
{
	UpdateEntity();
}

SceneTreeItem::~SceneTreeItem()
{ }

int SceneTreeItem::type() const
{
	return QStandardItem::UserType + 1;
}

QVariant SceneTreeItem::data(int role) const
{
	QVariant v;

	switch(role)
	{
	//case Qt::DecorationRole:
	//	v = itemData->GetIcon();
	//	break;
	case Qt::DisplayRole:
		v = entity->GetName().c_str();
		break;
	default:
		break;
	}

	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}

void SceneTreeItem::setData(const QVariant & value, int role)
{
	switch(role)
	{
	case Qt::EditRole:
		/*
		if(NULL != itemData)
		{
			itemData->SetValue(value);
		}
		break;
		*/
	default:
		QStandardItem::setData(value, role);
		break;
	}
}

DAVA::Entity* SceneTreeItem::GetEntity() const
{
	return entity;
}

SceneTreeItem* SceneTreeItem::SearchEntity(DAVA::Entity *entityToSearch)
{
	SceneTreeItem * ret = NULL;

	if(entityToSearch == entity)
	{
		ret = this;
	}
	else
	{
		for (int i = 0; i < rowCount(); i++)
		{
			SceneTreeItem *childItem = (SceneTreeItem *) child(i);

			ret = childItem->SearchEntity(entityToSearch);
			if(NULL != ret)
			{
				break;
			}
		}
	}

	return ret;
}

void SceneTreeItem::UpdateEntity()
{
	if(NULL != entity)
	{
		// remove all item children
		removeRows(0, rowCount());

		// check if entity has children
		if(entity->GetChildrenCount() > 0)
		{
			// if entity is solid - just mark it
			if(!entity->GetSolid())
			{
				// add child items
				for (int i = 0; i < entity->GetChildrenCount(); ++i)
				{
					appendRow(new SceneTreeItem(entity->GetChild(i)));
				}
			}
		}
	}
}
