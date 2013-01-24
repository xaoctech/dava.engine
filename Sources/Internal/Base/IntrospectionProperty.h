#ifndef __DAVAENGINE_INTROSPECTION_PROPERTY_H__
#define __DAVAENGINE_INTROSPECTION_PROPERTY_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	template<typename T, typename V>
	class IntrospectionProperty : public IntrospectionMember
	{
	public:
		typedef V	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionProperty(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	template<typename T, typename V>
	class IntrospectionPropertyReturnRef : public IntrospectionMember
	{
	public:
		typedef V&	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionPropertyReturnRef(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V& realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}
};

#endif // __DAVAENGINE_INTROSPECTION_PROPERTY_H__
