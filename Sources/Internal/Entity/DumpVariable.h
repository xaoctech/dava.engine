#ifndef __DAVAENGINE_DUMP_VARIABLE_H__
#define __DAVAENGINE_DUMP_VARIABLE_H__

#include "Math/AABBox3.h"

namespace DAVA
{

//template<class T> 
//void DumpVariable(const T & var)
//{
//	Logger::Info("    DumpVariable is not defined for type %s", typeid(var).name());
//}

//template<>
inline void DumpVariable(const uint32 & var)
{
	Logger::Info("    %d", var);
}

//template<>
inline void DumpVariable(const AABBox3 & var)
{
	Logger::Info("    (%.3f %.3f %.3f) (%.3f %.3f %.3f)", var.min.x, var.min.y, var.min.z, var.max.x, var.max.y, var.max.z);
}

//template<>
inline void DumpVariable(const Sphere & var)
{
	Logger::Info("    (%.3f %.3f %.3f) (%.3f)", var.center.x, var.center.y, var.center.z, var.radius);
}

//template<>
inline void DumpVariable(const Matrix4 & var)
{
	Logger::Info("    ---");
	Logger::Info("    [%.3f %.3f %.3f %.3f]", var._data[0][0], var._data[1][0], var._data[2][0], var._data[3][0]);
	Logger::Info("    [%.3f %.3f %.3f %.3f]", var._data[0][1], var._data[1][1], var._data[2][1], var._data[3][1]);
	Logger::Info("    [%.3f %.3f %.3f %.3f]", var._data[0][2], var._data[1][2], var._data[2][2], var._data[3][2]);
	Logger::Info("    [%.3f %.3f %.3f %.3f]", var._data[0][3], var._data[1][3], var._data[2][3], var._data[3][3]);
	Logger::Info("    ---");
}

inline void DumpVariable(void * var)
{
	Logger::Info("    %p", var);
}

};


#endif //__DAVAENGINE_DUMP_VARIABLE_H__
