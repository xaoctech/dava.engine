#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Main/QtUtils.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaKeyedArchive.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include <QPushButton>

QtPropertyDataDavaKeyedArcive::QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *archive)
	: curArchive(archive)
{
	if(NULL != curArchive)
	{
		curArchive->Retain();
	}

	SetFlags(FLAG_IS_DISABLED);
	ChildsCreate();
}

QtPropertyDataDavaKeyedArcive::~QtPropertyDataDavaKeyedArcive()
{
	if(NULL != curArchive)
	{
		curArchive->Release();
	}
}

QVariant QtPropertyDataDavaKeyedArcive::GetValueInternal()
{
	QVariant v;

	if(NULL != curArchive)
	{
		v = QString("KeyedArchive");
	}
	else
	{
		v = QString("KeyedArchive[NULL]");
	}

	return v;
}

void QtPropertyDataDavaKeyedArcive::SetValueInternal(const QVariant &value)
{ }

void QtPropertyDataDavaKeyedArcive::ChildChanged(const QString &key, QtPropertyData *data)
{
	if(NULL != curArchive)
	{
		QtPropertyDataDavaVariant *variantData = dynamic_cast<QtPropertyDataDavaVariant *>(data);
		if(NULL != variantData)
		{
			curArchive->SetVariant(key.toStdString(), variantData->GetVariantValue());
		}
	}
}

void QtPropertyDataDavaKeyedArcive::ChildsCreate()
{
	if(NULL != curArchive)
	{
		DAVA::Map<DAVA::String, DAVA::VariantType*> data = curArchive->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = data.begin();

		for(; i != data.end(); ++i)
		{
			QtPropertyData *childValueData;

			if(i->second->type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
			{
				childValueData = new QtPropertyDataDavaKeyedArcive(i->second->AsKeyedArchive());
			}
			else
			{
				childValueData = new QtPropertyDataDavaVariant(*(i->second));
			}

			ChildAdd(i->first.c_str(), childValueData);

			// add optional widget (button) to remove this key
			QPushButton *remButton = new QPushButton(QIcon(":/QtIcons/keyminus.png"), "");
			remButton->setIconSize(QSize(12, 12));
			childValueData->AddOW(QtPropertyOW(remButton));
		}

		// add optional widget (button) to add new key
		QPushButton *addButton = new QPushButton(QIcon(":/QtIcons/keyplus.png"), "");
		addButton->setIconSize(QSize(12, 12));
		AddOW(QtPropertyOW(addButton));
	}
}

void QtPropertyDataDavaKeyedArcive::AddKeyedArchiveField()
{
	if(NULL != curArchive)
	{
		curArchive->SetString("test", "string");
	}
}

void QtPropertyDataDavaKeyedArcive::RemKeyedArchiveField()
{
	QPushButton* btn = dynamic_cast<QPushButton*>(QObject::sender());
	if(NULL != btn && NULL != curArchive)
	{
		// search for child data with such button

	}
}