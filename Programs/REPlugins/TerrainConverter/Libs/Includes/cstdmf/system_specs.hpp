#ifndef CSTDMF_SYSTEM_SPECS_HPP
#define CSTDMF_SYSTEM_SPECS_HPP

#include "cstdmf/stdmf.hpp"

#include <stdint.h>

BW_BEGIN_NAMESPACE
	
class SystemSpecs
{
public:
	enum class CPUVendor : uint8_t
	{
		Intel	= 1,
		AMD		= 2,
		Other	= 3,

		Unknown = 4
	};

	enum class GPUVendor : uint8_t
	{
		Intel	= 1,
		NVidia	= 2,
		AMD		= 3,
		Other	= 4,

		Unknown = 5
	};

	enum class FormFactor
	{
		Desktop = 1,
		Laptop  = 2,
		Other   = 3,

		Unknown = 4
	};

	struct OSVersion
	{
		OSVersion( uint8_t mj, uint8_t mi, uint8_t sp_mj, uint8_t sp_mi )
		{
			version[0] = mj;
			version[1] = mi;
			version[2] = sp_mj;
			version[3] = sp_mi;
		}

		int major()	  const { return (int)version[0]; }
		int minor()	  const { return (int)version[1]; }
		int spMajor() const { return (int)version[2]; }
		int spMinor() const { return (int)version[3]; }

		bool isValid() const { return ( version[0] | version[1] | version[2] | version[3] ) != 0; }

		uint8_t version[4]; // major.minor.sp_major.sp_minor
	};

	static bool			Initialize();

	static bool			IsValid();

	// CPU
	static CPUVendor	getCPUVendor( bool force = false );
	static uint32_t		getCPUCoresNum( bool force = false );
	static uint32_t		getCPUFrequency( bool force = false );
	
	// Memory
	static uint32_t		getPhysicalMemoryAmount( bool force = false );
	static uint32_t		getPhysicalMemoryFrequency( bool force = false );
	static uint32_t		getTotalVirtualMemoryAmount( bool force = false );
	static uint32_t		getAvailableVirtualMemoryAmount( bool force = true );

	// GPU
	static GPUVendor	getGPUVendor( bool force = false );
	static uint32_t		getGPUMemory( bool force = false );

	// misc
	static bool			isNotebook( bool force = false );
	static FormFactor	getFormFactor( bool force = false );
	static OSVersion	getOSVersion( bool force = false );

private:
	
	static CPUVendor	_s_CPUVendor;
	static uint32_t		_s_CPUCoresNum;
	static uint32_t		_s_CPUFrequency;

	static uint32_t		_s_PhysicalMemoryAmount;
	static uint32_t		_s_PhysicalMemoryFrequency;
	static uint32_t		_s_TotalVirtualMemoryAmount;
	static uint32_t		_s_AvailableVirtualMemoryAmount;

	static GPUVendor	_s_GPUVendor;
	static uint32_t		_s_GPUMemory;

	static FormFactor	_s_FormFactor;
	static OSVersion	_s_OSVersion;
};

BW_END_NAMESPACE

#endif // CSTDMF_SYSTEM_SPECS_HPP