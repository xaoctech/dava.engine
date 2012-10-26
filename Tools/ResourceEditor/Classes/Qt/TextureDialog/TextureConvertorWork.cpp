#include "TextureDialog/TextureConvertorWork.h"

WorkStack::WorkStack()
	: head(NULL)
	, itemsCount(0)
{ }

WorkStack::~WorkStack()
{
	WorkItemWrapper *item;

	while(NULL != head)
	{
		item = head;
		head = head->next;

		delete item;
	}
}

void WorkStack::push(const WorkItem::WorkItemType &type, const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor)
{
	WorkItemWrapper *item = head;

	// search for the same works in list and remove it
	while(NULL != item)
	{
		if(item->type == type && item->texture == texture)
		{
			if(NULL != item->prev)
			{
				item->prev->next = item->next;
			}

			if(NULL != item->next)
			{
				item->next->prev = item->prev;
			}

			if(item == head)
			{
				head = item->next;
			}

			delete item;
			itemsCount--;

			break;
		}

		item = item->next;
	}

	// add new work
	item = new WorkItemWrapper(type, texture, descriptor);
	if(NULL != head)
	{
		head->prev = item;
		item->next = head;
	}

	head = item;
	itemsCount++;
}

WorkItem* WorkStack::pop()
{
	WorkItemWrapper *item = head;

	if(NULL != head)
	{
		head = head->next;
		if(NULL != head)
		{
			head->prev = NULL;
		}

		itemsCount--;
	}

	return item;
}

int WorkStack::size()
{
	return itemsCount;
}

WorkStack::WorkItemWrapper::WorkItemWrapper(const WorkItem::WorkItemType &type, const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor)
	: prev(NULL)
	, next(NULL)
{
	this->type = type;
	this->texture = texture;
	this->descriptor = descriptor;
}
