/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_MATH2DMATRIX3_H__
#define __DAVAENGINE_MATH2DMATRIX3_H__

#include <math.h>

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Math/MathDefines.h"

namespace DAVA
{
/**	
	\ingroup math
	\brief Class to work with 3 x 3 matrices.
 */
struct Matrix3
{
	union
	{
		float32 data[9];
		float32	_data[3][3];
		struct {
			float32 _00, _01, _02;
            float32 _10, _11, _12;
            float32 _20, _21, _22;
		};
	};
	
	inline Matrix3() {};

	inline Matrix3(	float32 __00, float32 __01, float32 __02,
					float32 __10, float32 __11, float32 __12,
					float32 __20, float32 __21, float32 __22);

	inline Matrix3(const Matrix3 & m);

	inline float32 Det() const;

	
	// Helpers
	inline void Identity();
	inline void	CreateRotation(const Vector3 & r, float32 angleInRadians);
	inline void	BuildRotation(float32 Angle);
	inline void	BuildTranslation(float32 x, float32 y);
	inline void	BuildTranslation(const Vector2 & vec);
	inline void	BuildScale(const Vector2 & vec);
	inline bool GetInverse(Matrix3 & out, float32 fTolerance = 1e-06) const;

	inline Matrix3& operator *= (const Matrix3 & arg);
	inline Matrix3 operator *	(const Matrix3 & arg) const;

	inline Matrix3& operator -= (const Matrix3 & arg);
	inline Matrix3 operator -	(const Matrix3 & arg) const;

	inline Matrix3& operator += (const Matrix3 & arg);
	inline Matrix3 operator +	(const Matrix3 & arg) const;
    
    inline bool operator == (const Matrix3 & _m) const;
	inline bool operator != (const Matrix3 & _m) const;
};

inline Vector2 operator * (const Vector2 & _v, const Matrix3 & _m);
	
	
// Implementation

inline Matrix3::Matrix3(float32 __00, float32 __01, float32 __02,
						float32 __10, float32 __11, float32 __12,
						float32 __20, float32 __21, float32 __22)
{
	_00 = __00; _01 = __01; _02 = __02;
	_10 = __10; _11 = __11; _12 = __12;
	_20 = __20; _21 = __21; _22 = __22;
};

inline Matrix3::Matrix3(const Matrix3 & m)
{
	*this = m;
}

inline float32 Matrix3::Det() const
{
	return _00 * _11 * _22 + _01 * _12 * _20 + _02 * _10 * _21 
		- _02 * _11 * _20 - _01 * _10 * _22 - _00 * _12 * _21;
}

inline void Matrix3::Identity()
{
	_00 = 1.0f; _01 = 0.0f; _02 = 0.0f;
	_10 = 0.0f; _11 = 1.0f; _12 = 0.0f;
	_20 = 0.0f; _21 = 0.0f; _22 = 1.0f;
}

inline void	Matrix3::CreateRotation(const Vector3 & r, float32 angleInRadians)
{
	float32 cosA = cosf(angleInRadians);
	float32 sinA = sinf(angleInRadians);
	Identity();
	_data[0][0] = cosA + (1 - cosA) * r.x * r.x;
	_data[0][1] = (1 - cosA) * r.x * r.y -  r.z * sinA;
	_data[0][2] = (1 - cosA) * r.x * r.z +  r.y * sinA;


	_data[1][0] = (1 - cosA) * r.x * r.y +  r.z * sinA;
	_data[1][1] = cosA + (1 - cosA) * r.y * r.y;
	_data[1][2] = (1 - cosA) * r.y * r.z -  r.x * sinA;


	_data[2][0] = (1 - cosA) * r.x * r.z -  r.y * sinA;
	_data[2][1] = (1 - cosA) * r.y * r.z +  r.x * sinA;
	_data[2][2] = cosA + (1 - cosA) * r.z * r.z;
}

inline void Matrix3::BuildRotation(float32 angle)
{
	float32	cosA = cosf(angle);
	float32	sinA = sinf(angle);

	_00 = cosA;		_01 = sinA;		_02 = 0;
	_10 = -sinA;	_11 = cosA;		_12 = 0;
	_20 = 0;		_21 = 0;		_22 = 1;
}

inline void	Matrix3::BuildTranslation(float32 _xT, float32 _yT)
{
	Identity();
	_20 = _xT;
	_21 = _yT;
}
	
inline void	Matrix3::BuildTranslation(const Vector2 & vec)
{
	Identity();
	_20 = vec.x;
	_21 = vec.y;
}
	
inline void	Matrix3::BuildScale(const Vector2 & vec)
{
	Identity();
	_00 = vec.x;
	_11 = vec.y;
}

inline Matrix3 Matrix3::operator *(const Matrix3 & m) const
{
	return Matrix3( _00 * m._00 + _01 * m._10 + _02 * m._20,		
					_00 * m._01 + _01 * m._11 + _02 * m._21,		
					_00 * m._02 + _01 * m._12 + _02 * m._22, 

					_10 * m._00 + _11 * m._10 + _12 * m._20,		
					_10 * m._01 + _11 * m._11 + _12 * m._21,		
					_10 * m._02 + _11 * m._12 + _12 * m._22, 

					_20 * m._00 + _21 * m._10 + _22 * m._20,		
					_20 * m._01 + _21 * m._11 + _22 * m._21,		
					_20 * m._02 + _21 * m._12 + _22 * m._22);
}

inline Matrix3& Matrix3::operator *= (const Matrix3 & m)
{
	return (*this = *this * m);
}

inline Matrix3 Matrix3::operator +(const Matrix3 & m) const
{
	return Matrix3( _00 + m._00, _01 + m._01, _02 + m._02, 
					_10 + m._10, _11 + m._11, _12 + m._12,
					_20 + m._20, _21 + m._21, _22 + m._22);
}

inline Matrix3& Matrix3::operator += (const Matrix3 & m)
{
	return (*this = *this + m);
}

inline Matrix3 Matrix3::operator -(const Matrix3 & m) const
{
	return Matrix3( _00 - m._00, _01 - m._01, _02 - m._02, 
					_10 - m._10, _11 - m._11, _12 - m._12,
					_20 - m._20, _21 - m._21, _22 - m._22);
}

inline Matrix3& Matrix3::operator -= (const Matrix3 & m)
{
	return (*this = *this - m);
}
	
inline Vector2 operator * (const Vector2 & _v, const Matrix3 & _m)
{
	Vector2 res;
	
	res.x = _v.x * _m._00 + _v.y * _m._10 + _m._20;
	res.y = _v.x * _m._01 + _v.y * _m._11 + _m._21;
	
	return res;
}
    
inline Vector3 operator * (const Vector3 & _v, const Matrix3 & _m)
{
    Vector3 res;
    
    res.x = _v.x * _m._00 + _v.y * _m._10 + _v.z * _m._20;
    res.y = _v.x * _m._01 + _v.y * _m._11 + _v.z * _m._21;
    res.z = _v.x * _m._02 + _v.y * _m._12 + _v.z * _m._22;
    
    return res;
}
	
	
	/*
	 rkInverse[0][0] = m[1][1]*m[2][2] -
	 m[1][2]*m[2][1];
	 rkInverse[0][1] = m[0][2]*m[2][1] -
	 m[0][1]*m[2][2];
	 rkInverse[0][2] = m[0][1]*m[1][2] -
	 m[0][2]*m[1][1];
	 rkInverse[1][0] = m[1][2]*m[2][0] -
	 m[1][0]*m[2][2];
	 rkInverse[1][1] = m[0][0]*m[2][2] -
	 m[0][2]*m[2][0];
	 rkInverse[1][2] = m[0][2]*m[1][0] -
	 m[0][0]*m[1][2];
	 rkInverse[2][0] = m[1][0]*m[2][1] -
	 m[1][1]*m[2][0];
	 rkInverse[2][1] = m[0][1]*m[2][0] -
	 m[0][0]*m[2][1];
	 rkInverse[2][2] = m[0][0]*m[1][1] -
	 m[0][1]*m[1][0];
	 
	 Real fDet =
	 m[0][0]*rkInverse[0][0] +
	 m[0][1]*rkInverse[1][0]+
	 m[0][2]*rkInverse[2][0];
	 
	 if ( Math::Abs(fDet) <= fTolerance )
	 return false;
	 
	 Real fInvDet = 1.0f/fDet;
	 for (size_t iRow = 0; iRow < 3; iRow++)
	 {
	 for (size_t iCol = 0; iCol < 3; iCol++)
	 rkInverse[iRow][iCol] *= fInvDet;
	 }

        return true;

	 */

inline bool Matrix3::GetInverse(Matrix3 & out, float32 fTolerance) const
{
	const Matrix3 &m = *this;
	out._00 = m._data[1][1]*m._data[2][2] - m._data[1][2]*m._data[2][1];
	out._01 = m._data[0][2]*m._data[2][1] - m._data[0][1]*m._data[2][2];
	out._02 = m._data[0][1]*m._data[1][2] - m._data[0][2]*m._data[1][1];
	out._10 = m._data[1][2]*m._data[2][0] - m._data[1][0]*m._data[2][2];
	out._11 = m._data[0][0]*m._data[2][2] - m._data[0][2]*m._data[2][0];
	out._12 = m._data[0][2]*m._data[1][0] - m._data[0][0]*m._data[1][2];
	out._20 = m._data[1][0]*m._data[2][1] - m._data[1][1]*m._data[2][0];
	out._21 = m._data[0][1]*m._data[2][0] - m._data[0][0]*m._data[2][1];
	out._22 = m._data[0][0]*m._data[1][1] - m._data[0][1]*m._data[1][0];
	
	float32 fDet = m._data[0][0]*out._data[0][0] + m._data[0][1]*out._data[1][0]+ m._data[0][2]*out._data[2][0];
	
	if (Abs(fDet) <= fTolerance )
		return false;
	
	float32 fInvDet = 1.0f / fDet;
	for (int32 iRow = 0; iRow < 9; iRow++)
	{
		out.data[iRow] *= fInvDet;
	}
	
	return true;
}
	
inline bool Matrix3::operator == (const Matrix3 & _m) const
{
    for (uint8 k = 0; k < COUNT_OF(data); ++k)
        if (!FLOAT_EQUAL(data[k], _m.data[k]))return false;
    return true;
}

inline bool Matrix3::operator != (const Matrix3 & _m) const
{
    return ! Matrix3::operator==(_m);
}

};	// end of namespace DAVA

#endif // __DAVAENGINE_MATH2DMATRIX3_H__

