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
		printf("Type: %s\n", info->Name());
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember* member = info->Member(i);
			printf(" %s, type %s\n", member->Name(), member->Type()->GetTypeName());
		}

		info = info->BaseInfo();
	}

	printf("\nSet/Get test:\n");

	t1.setp1(1111);

	const DAVA::IntrospectionMember *p1 = t1.Info()->Member("p1");

	DAVA::VariantType v = p1->Value(&t1);

	printf("p1 = %d, pp1 = %d\n", v.AsInt32(), t1.pp1);

	v.SetInt32(200);

	printf("trying to set new p1 value\n");
	p1->SetValue(&t1, v);

	v = p1->Value(&t1);
	printf("p1 = %d, pp1 = %d\n", v.AsInt32(), t1.pp1);

	printf("\n\n");

	info = t2.Info();
	while(NULL != info)
	{
		printf("Type: %s\n", info->Name());
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
