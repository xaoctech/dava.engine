#ifndef __TEXTURE_CONVERTOR_WORK_H__
#define __TEXTURE_CONVERTOR_WORK_H__

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"

struct JobItem
{
	enum JobItemType
	{
		JobOriginal,
		JobPVR,
		JobDXT
	};

	JobItem()
		: forceConvert(false)
		, type(JobOriginal)
		, descriptor(NULL)
	{ }

	bool forceConvert;
	JobItemType type;
	const DAVA::TextureDescriptor *descriptor;

	// grant thread safe access to descriptor
	DAVA::TextureDescriptor descriptorCopy;
};

class JobStack
{
public:
	JobStack();
	~JobStack();

	void push(const JobItem &item);
	JobItem* pop();
	int size();

private:
	struct JobItemWrapper : public JobItem
	{
		JobItemWrapper(const JobItem &item);

		JobItemWrapper *next;
		JobItemWrapper *prev;
	};

	JobItemWrapper *head;

	int itemsCount;
};

#endif // __TEXTURE_CONVERTOR_WORK_H__
