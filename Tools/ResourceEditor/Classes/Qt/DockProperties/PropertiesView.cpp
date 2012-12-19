#include "DockProperties/PropertiesView.h"

PropertiesView::PropertiesView(QWidget *parent /* = 0 */)
	: QtGroupBoxPropertyBrowser(parent)
	, curNode(NULL)
{
	test1 t1;
	test2 t2;

	const DAVA::IntrospectionInfo *info = t1.Info();

	while(NULL != info)
	{
		printf("Type: %s\n", info->name);
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember* member = info->Member(i);
			printf(" %s, type %s\n", member->Name(), member->Type()->GetTypeName());
		}

		info = info->BaseInfo();
	}

	printf("\n");

	info = t2.Info();
	while(NULL != info)
	{
		printf("Type: %s\n", info->name);
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember* member = info->Member(i);
			printf(" %s, type %s\n", member->Name(), member->Type()->GetTypeName());
		}

		info = info->BaseInfo();
	}
}

PropertiesView::~PropertiesView()
{
	SafeRelease(curNode);
}

void PropertiesView::SetNode(DAVA::SceneNode *node)
{
	SafeRelease(curNode);
	curNode = SafeRetain(node);
}


