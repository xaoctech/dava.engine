#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionMember *_member)
	: object(_object)
	, member(_member)
{
	SubPropertiesCreate();
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	QVariant v;

	if(member->Type() == DAVA::MetaInfo::Instance<DAVA::int32>())
	{
		v = member->Value(object).AsInt32();
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
	{
		v = member->Value(object).AsBool();
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::String>())
	{
		v = QString(member->Value(object).AsString().c_str());
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::float32>())
	{
		v = (double) member->Value(object).AsFloat();
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix2>())
	{
		DAVA::Matrix2 mat2 = member->Value(object).AsMatrix2();
		v = QString().sprintf("([%g, %g], [%g, %g])",
			mat2._00, mat2._01,
			mat2._10, mat2._11);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix3>())
	{
		DAVA::Matrix3 mat3 = member->Value(object).AsMatrix3();
		v = QString().sprintf("([%g, %g, %g], [%g, %g, %g], [%g, %g, %g])",
			mat3._00, mat3._01, mat3._02,
			mat3._10, mat3._11, mat3._12,
			mat3._20, mat3._21, mat3._22);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix4>())
	{
		DAVA::Matrix4 mat4 = member->Value(object).AsMatrix4();
		v = QString().sprintf("([%g, %g, %g, %g], [%g, %g, %g, %g], [%g, %g, %g, %g], [%g, %g, %g, %g])",
			mat4._00, mat4._01, mat4._02, mat4._03,
			mat4._10, mat4._11, mat4._12, mat4._13,
			mat4._20, mat4._21, mat4._22, mat4._23,
			mat4._30, mat4._31, mat4._32, mat4._33);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
	{
		DAVA::Vector2 vec = member->Value(object).AsVector2();
		v = QString().sprintf("[%g, %g]", vec.x, vec.y);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
	{
		DAVA::Vector3 vec = member->Value(object).AsVector3();
		v = QString().sprintf("[%g, %g, %g]", vec.x, vec.y, vec.z);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
	{
		DAVA::Vector4 vec = member->Value(object).AsVector4();
		v = QString().sprintf("[%g, %g, %g, %g]", vec.x, vec.y, vec.z, vec.w);
	}

	return v;
}

void QtPropertyDataIntrospection::SetValueInternal(const QVariant &value)
{
	DAVA::VariantType v;
	bool ok = false;

	if(member->Type() == DAVA::MetaInfo::Instance<DAVA::int32>())
	{
		v.SetInt32(value.toInt(&ok));
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
	{
		v.SetBool(value.toBool());
		ok = true;
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::String>())
	{
		v.SetString(value.toString().toStdString());
		ok = true;
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::float32>())
	{
		v.SetFloat(value.toFloat(&ok));
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix2>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix3>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix4>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
	{
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
	{
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
	{
	}

	if(ok)
	{
		member->SetValue(object, v);
	}
}

void QtPropertyDataIntrospection::ChildChanged(const QString &key, QtPropertyData *data)
{
	if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix2>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix3>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix4>())
	{
		DVASSERT("Templory unsupported type");
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
	{
		QVariant v = data->GetValue();
		DAVA::Vector2 vec = member->Value(object).AsVector2();
		DAVA::VariantType va;

		if(key == "X") vec.x = v.toFloat();
		if(key == "Y") vec.y = v.toFloat();

		va.SetVector2(vec);
		member->SetValue(object, va);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
	{
		QVariant v = data->GetValue();
		DAVA::Vector3 vec = member->Value(object).AsVector3();
		DAVA::VariantType va;

		if(key == "X") vec.x = v.toFloat();
		if(key == "Y") vec.y = v.toFloat();
		if(key == "Z") vec.z = v.toFloat();

		va.SetVector3(vec);
		member->SetValue(object, va);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
	{
		QVariant v = data->GetValue();
		DAVA::Vector4 vec = member->Value(object).AsVector4();
		DAVA::VariantType va;

		if(key == "X") vec.x = v.toFloat();
		if(key == "Y") vec.y = v.toFloat();
		if(key == "Z") vec.z = v.toFloat();
		if(key == "W") vec.w = v.toFloat();

		va.SetVector4(vec);
		member->SetValue(object, va);
	}
}

void QtPropertyDataIntrospection::ChildUpdate()
{
	SubPropertiesUpdate();
}

void QtPropertyDataIntrospection::SubPropertiesCreate()
{
	if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
	{
		DAVA::Vector2 vec = member->Value(object).AsVector2();
		ChildAdd("X", vec.x);
		ChildAdd("Y", vec.y);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
	{
		DAVA::Vector3 vec = member->Value(object).AsVector3();
		ChildAdd("X", vec.x);
		ChildAdd("Y", vec.y);
		ChildAdd("Z", vec.z);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
	{
		DAVA::Vector4 vec = member->Value(object).AsVector4();
		ChildAdd("X", vec.x);
		ChildAdd("Y", vec.y);
		ChildAdd("Z", vec.z);
		ChildAdd("W", vec.w);
	}
}

void QtPropertyDataIntrospection::SubPropertiesUpdate()
{
	if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
	{
		DAVA::Vector2 vec = member->Value(object).AsVector2();
		ChildGet("X")->SetValue(vec.x);
		ChildGet("Y")->SetValue(vec.y);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
	{
		DAVA::Vector3 vec = member->Value(object).AsVector3();
		ChildGet("X")->SetValue(vec.x);
		ChildGet("Y")->SetValue(vec.y);
		ChildGet("Z")->SetValue(vec.z);
	}
	else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
	{
		DAVA::Vector4 vec = member->Value(object).AsVector4();
		ChildGet("X")->SetValue(vec.x);
		ChildGet("Y")->SetValue(vec.y);
		ChildGet("Z")->SetValue(vec.z);
		ChildGet("W")->SetValue(vec.w);
	}
}
