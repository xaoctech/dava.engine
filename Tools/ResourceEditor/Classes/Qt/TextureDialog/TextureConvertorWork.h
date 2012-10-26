#ifndef __TEXTURE_CONVERTOR_WORK_H__
#define __TEXTURE_CONVERTOR_WORK_H__

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"

struct WorkItem
{
	enum WorkItemType
	{
		WorkPVR,
		WorkDXT
	};

	WorkItemType type;
	const DAVA::Texture *texture;
	DAVA::TextureDescriptor descriptor;
};

class WorkStack
{
public:
	WorkStack();
	~WorkStack();

	void push(const WorkItem::WorkItemType &type, const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor);
	WorkItem* pop();
	int size();

private:
	struct WorkItemWrapper : public WorkItem
	{
		WorkItemWrapper(const WorkItem::WorkItemType &type, const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor);

		WorkItemWrapper *next;
		WorkItemWrapper *prev;
	};

	WorkItemWrapper *head;

	int itemsCount;
};

#endif // __TEXTURE_CONVERTOR_WORK_H__
